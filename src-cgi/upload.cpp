#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <fcgi_stdio.h>
#include <mysqlx/xdevapi.h>
#include <glog/logging.h>

#include "upload.h"
#include "fastdfs_api.h"
#include "utils_cfg.h"

using namespace std;
using namespace Utils;

namespace {
	const int kHeaderSize = 256; // Take 256 bytes to parse the head of message.
	const int kTailorSize = 256; // Reserve 256 bytes to parse the end of message.
	const int kBuffSize = 1024;
} // namespace 

int main(int argc, char* argv[])
{
	google::InitGoogleLogging(argv[0]);
	FLAGS_log_dir = kLogPath;

	try {
		doWork();
	}
	catch (const exception& ex) {
		LOG(ERROR) << "escaped exception catched in main function " << ex.what();
	}

	google::ShutdownGoogleLogging();
	return 0;
}


void doWork() {
	// Read config info
	FdfsInfo fdfsInfo;
	MysqlInfo mysqlInfo;
	getFdfsInfo(fdfsInfo);
	getMysqlInfo(mysqlInfo);

	while (FCGI_Accept() >= 0) {
		int contentLen;
		string contentType;
		string content;
		FileInfo fileInfo;

		contentType = ::getenv("CONTENT_TYPE");
		if (::getenv("CONTENT_LENGTH") != NULL) {
			contentLen = ::atoi(::getenv("CONTENT_LENGTH"));
		}

		if (contentLen > 0) {
			string tempFileName;
			ReplyStatus replyStat;

			try {
				tempFileName = recvUploadFile(contentLen, fileInfo);
				string fileId = uploadFileToFastdfs(tempFileName, fdfsInfo);
				replyStat = dealWithMySQL(mysqlInfo, fdfsInfo, fileInfo, fileId);
			}
			catch (const exception& ex) {
				LOG(ERROR) << ex.what();
				replyStat = ReplyStatus::kFailed;
			}

			// Delete tempFile on server
			::unlink(tempFileName.c_str());
			FCGI_printf("Content-type: %s\r\n\r\n", kJsonContentType.c_str());
			FCGI_printf("%s", setReplyData(ReplyStatus::kSuccess).c_str());
		}
		else
		{
			FCGI_printf("Content-type: %s\r\n\r\n", kJsonContentType.c_str());
			FCGI_printf("%s", setReplyData(ReplyStatus::kFailed).c_str());
		}
	}
}

string uploadFileToFastdfs(const string& tempFileName, FdfsInfo& fdfsInfo)
{
	string fileId;
	FastDFS fastDFS;
	try {
		fastDFS.init(fdfsInfo.fdfsConfigFile);
		fileId = fastDFS.uploadFile(tempFileName);
	}
	catch (exception& ex) {
		LOG(ERROR) << ex.what();
		throw;
	}
	return fileId;
}

ReplyStatus dealWithMySQL(const Utils::MysqlInfo& mysqlInfo, Utils::FdfsInfo& fdfsInfo, 
	const FileInfo& fileInfo, const std::string& fileId)
{
	mysqlx::Session sess(mysqlInfo.host, mysqlInfo.port, mysqlInfo.user,
		mysqlInfo.passwd, mysqlInfo.database);
	mysqlx::Table fileInfoTable = sess.getDefaultSchema().getTable("file_info", true);
	mysqlx::Table userFileTable = sess.getDefaultSchema().getTable("user_file_list", true);

	string fileUrl;
	fileUrl.append("http://")
		.append(fdfsInfo.storageServerIp)
		.append(":")
		.append(to_string(fdfsInfo.storageServerPort))
		.append("/")
		.append(fileId);
	
	sess.startTransaction();
	try {
		fileInfoTable.insert("md5", "url", "size", "type", "count")
			.values(fileInfo.md5, fileUrl, fileInfo.size, fileInfo.type, 1)
			.execute();
		userFileTable.insert("user_name", "md5", "file_name")
			.values(fileInfo.user, fileInfo.md5, fileInfo.fileName)
			.execute();
	}
	catch (const mysqlx::Error& err) {
		sess.rollback();
		return ReplyStatus::kFailed;
	}
	sess.commit();

	DLOG(INFO) << fileUrl;
	return ReplyStatus::kSuccess;
}

string recvUploadFile(const int contentLen, FileInfo& fileInfo)
{
	int stdBuffLen = contentLen;
	int mainContentLen = 0;
	int readLen = 0;
	string boundaryStr;

	// Parse Header
	readLen = fcgiParseHead(boundaryStr, fileInfo);
	stdBuffLen -= readLen;

	// Create temp file
	string tempFileName = fileInfo.user + ":" + fileInfo.fileName;
	FILE* tempFile = FCGI_fopen(tempFileName.c_str(), "wb");
	if (tempFile == NULL) {
		LOG(ERROR) << "Temp file create error!";
		throw runtime_error("Temp file create error!");
	}

	// Parse main content
	if (stdBuffLen > kTailorSize) {
		readLen = fcgiParseContent(stdBuffLen - kTailorSize, tempFile);
		stdBuffLen -= readLen;
	}

	// Parse tailor
	readLen = fcgiParseTailor(boundaryStr, tempFile);
	stdBuffLen -= readLen;
	assert(stdBuffLen == 0);

	// Close file
	FCGI_fclose(tempFile);
	return tempFileName;
}


int fcgiParseHead(string& boundaryStr, FileInfo& fileInfo) {
	/*
		Header Struct
		1 --boundary_xxx\r\n
		2 Content-Disposition:form-data; name = "file"\r\n
		3 Content-Type: application/octet-stream\r\n
		4 User: xxx\r\n
		5 FileName: xxx.cpp\r\n
		6 FileSize: 8051\r\n
		7 MD5: fffedd8dd1ae798df94afa72ff809935\r\n
		8 \r\n
	*/
	int readLen = 0;
	char buff[kHeaderSize] = { 0 };

	// Boundary
	FCGI_fgets(buff, kHeaderSize, FCGI_stdin);
	readLen += ::strlen(buff);
	boundaryStr.append(buff);
	boundaryStr.pop_back();
	boundaryStr.pop_back();

	// Skip 2 lines
	for (int i = 0; i < 2; i++) {
		FCGI_fgets(buff, kHeaderSize, FCGI_stdin);
		readLen += ::strlen(buff);
	}

	// User
	FCGI_fgets(buff, kHeaderSize, FCGI_stdin);
	readLen += ::strlen(buff);
	fileInfo.user.append(buff + ::strlen("User: "));
	fileInfo.user.pop_back();
	fileInfo.user.pop_back();
	
	// FileName
	FCGI_fgets(buff, kHeaderSize, FCGI_stdin);
	readLen += ::strlen(buff);
	fileInfo.fileName.append(buff + ::strlen("FileName: "));
	fileInfo.fileName.pop_back();
	fileInfo.fileName.pop_back();
	auto typeStart = fileInfo.fileName.find_last_of('.');
	if (typeStart != fileInfo.fileName.npos) {
		fileInfo.type = fileInfo.fileName.substr(typeStart + 1);
	}
	
	// FileSize
	FCGI_fgets(buff, kHeaderSize, FCGI_stdin);
	readLen += ::strlen(buff);
	string sizeStr(buff + ::strlen("FileSize: "));
	sizeStr.pop_back();
	sizeStr.pop_back();
	fileInfo.size = ::atoi(sizeStr.c_str());

	// MD5
	FCGI_fgets(buff, kHeaderSize, FCGI_stdin);
	readLen += ::strlen(buff);
	fileInfo.md5.append(buff + ::strlen("MD5: "));
	fileInfo.md5.pop_back();
	fileInfo.md5.pop_back();
	
	// Skip \r\n
	FCGI_fgets(buff, kHeaderSize, FCGI_stdin);
	readLen += ::strlen(buff);

	/*DLOG(INFO) << "Boundary " << boundaryStr << endl
		<< "User " << fileInfo.user << endl
		<< "Name " << fileInfo.fileName << endl
		<< "Type " << fileInfo.type << endl
		<< "MD5 " << fileInfo.md5 << endl
		<< "Size " << fileInfo.size;*/
	return readLen;
}

int fcgiParseContent(const int mainContentLen, FILE* tempFile) {
	int totalToReadLen = mainContentLen;
	int totalLen = 0;
	char buff[kBuffSize] = { 0 };
	while (totalToReadLen > 0) {
		int toReadLen = min(totalToReadLen, kBuffSize);
		int readLen = FCGI_fread(buff, sizeof(char), toReadLen, FCGI_stdin);
		FCGI_fwrite(buff, sizeof(char), readLen, tempFile);
		totalToReadLen -= readLen;
		totalLen += readLen;
		//::memset(buff, 0, kBuffSize);
	}
	return totalLen;
}

int fcgiParseTailor(const string& boundaryStr, FILE* tempFile) {
	int readLen = 0;
	char buff[kTailorSize] = { 0 };
	readLen += FCGI_fread(buff, sizeof(char), kTailorSize, FCGI_stdin);
	// Find End Boundary, main content need drop "\r\n"
	char* pos = ::strstr(buff, boundaryStr.c_str());
	FCGI_fwrite(buff, sizeof(char), pos - buff - 2, tempFile);
	return readLen;
}

string setReplyData(const ReplyStatus stat) {
	Json::Value root;
	root["code"] = getReplyCode(stat);
	return root.toStyledString();
}
