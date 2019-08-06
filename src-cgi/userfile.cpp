#include "userfile.h"
#include "redis_api.h"
#include "redis_keys.h"
#include "utils_cfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdexcept>

#include <fcgi_stdio.h>
#include <mysqlx/xdevapi.h>
#include <glog/logging.h>
#include <json/json.h>

using namespace std;
using namespace Utils;

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
	// Read Configure Info
	MysqlInfo mysql;
	RedisInfo redis;
	getMysqlInfo(mysql);
	getRedisInfo(redis);

	// Working loop
	while (FCGI_Accept() >= 0) {
		int contentLen;
		string contentType;
		string content;

		contentType = ::getenv("CONTENT_TYPE");
		if (::getenv("CONTENT_LENGTH") != NULL) {
			contentLen = ::atoi(::getenv("CONTENT_LENGTH"));
		}

		if (contentLen > 0) {
			char* data = (char*)::malloc(contentLen + 1);
			::memset(data, 0, contentLen + 1);
			FCGI_fread(data, sizeof(char), contentLen, FCGI_stdin);

			// Deal post data and reply
			UserFileMessage userMsg;
			ReplyStatus replyStat;
			vector<FileInfo> files;
			// Main process
			try {
				parseClientJson(data, contentLen, userMsg);
				if (!checkToken(redis, userMsg.user, userMsg.token)) {
					replyStat = ReplyStatus::kTokenInvalid;
				}
				else {
					replyStat = checkInMysql(mysql, userMsg, files);
				}
			}
			catch (const mysqlx::Error& err) {
				LOG(ERROR) << "mysqlx error " << err.what();
				replyStat = ReplyStatus::kFailed;
			}
			catch (const exception& ex) {
				LOG(ERROR) << ex.what();
				replyStat = ReplyStatus::kFailed;
			}
			
			// Release resource
			::free(data);
			FCGI_printf("Content-type: %s\r\n\r\n", kJsonContentType.c_str());
			FCGI_printf("%s", setReplyData(replyStat, files).c_str());
		}
		else
		{
			FCGI_printf("Content-type: %s\r\n\r\n", kJsonContentType.c_str());
			FCGI_printf("%s", setReplyData(ReplyStatus::kFailed, vector<FileInfo>()).c_str());
		}
	}
}


void parseClientJson(const char* data, const size_t len, UserFileMessage& out) {
	Json::Reader reader;
	Json::Value root;
	string dataStr(data, len);

	if (!reader.parse(dataStr, root)) {
		LOG(ERROR) << "Json parse error";
		throw runtime_error("Json parse error");
	}

	out.user = root["user"].asString();
	out.token = root["token"].asString();
	
	DLOG(INFO) << out.user << " " << out.token;
}

bool checkToken(const Utils::RedisInfo& redisInfo, const string& user, const string& token)
{
	Redis redis;
	if (!redis.connect(redisInfo.ip, redisInfo.port)) {
		LOG(ERROR) << "connect redis error";
		return false;
	}

	string redisKey = kTokenPrefix + user;
	string tokenInRedis = redis.get(redisKey);
	if (tokenInRedis != token) {
		return false;
	}
	return true;
}

ReplyStatus checkInMysql(const MysqlInfo& mysqlInfo, const UserFileMessage& userMsg, 
	vector<FileInfo>& files) {
	mysqlx::Session sess(mysqlInfo.host, mysqlInfo.port, mysqlInfo.user,
		mysqlInfo.passwd, mysqlInfo.database);
	// Select from MySQL
	string sqlStat;
	sqlStat.append("SELECT u.user_name, u.file_name, DATE_FORMAT(createtime, \"%Y/%m/%d %T\"), u.shared_status, u.download_count, ")
		.append("f.url, f.type, f.size ")
		.append("FROM file_info f, user_file_list u ")
		.append("WHERE u.user_name = '").append(userMsg.user).append("' ")
		.append("AND f.md5 = u.md5");

	try {
		mysqlx::RowResult rows = sess.sql(sqlStat).execute();
		// Add file info
		for (const auto& row : rows) {
			FileInfo temp;
			temp.user = (string)row[0];
			temp.filename = (string)row[1];
			temp.timestamp = (string)row[2];
			temp.shareStatus = row[3];
			temp.downloadCount = row[4];
			temp.url = (string)row[5];
			temp.type = (string)row[6];
			temp.size = row[7];
			files.push_back(temp);
		}
	}
	catch (const mysqlx::Error& err) {
		LOG(ERROR) << err.what();
		files.clear();
		return ReplyStatus::kFailed;
	}

	return ReplyStatus::kSuccess;
}

string setReplyData(const ReplyStatus stat, const vector<FileInfo>& files) {
	Json::Value root;
	root["code"] = getReplyCode(stat);
	for (const auto& file : files) {
		Json::Value fileInfo;
		fileInfo["user"] = file.user;
		fileInfo["filename"] = file.filename;
		fileInfo["timestamp"] = file.timestamp;
		fileInfo["shareStatus"] = file.shareStatus;
		fileInfo["downloadCount"] = file.downloadCount;
		fileInfo["url"] = file.url;
		fileInfo["type"] = file.type;
		fileInfo["size"] = file.size;
		root["files"].append(fileInfo);
	}
	return root.toStyledString();
}