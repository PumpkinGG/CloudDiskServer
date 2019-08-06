#include "upload_md5.h"
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
			// Read post data from stdin
			char* data = (char*)::malloc(contentLen + 1);
			memset(data, 0, contentLen + 1);
			FCGI_fread(data, sizeof(char), contentLen, stdin);

			// Deal post data and reply
			UploadMessage uploadMsg;
			ReplyStatus replyStat;
			// Main process
			try {
				parseClientJson(data, contentLen, uploadMsg);
				if (!checkToken(redis, uploadMsg.user, uploadMsg.token)) {
					replyStat = ReplyStatus::kTokenInvalid;
				}
				else {
					replyStat = checkInMysql(mysql, uploadMsg);
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
			FCGI_printf("%s", setReplyData(replyStat).c_str());
		}
		else
		{
			FCGI_printf("Content-type: %s\r\n\r\n", kJsonContentType.c_str());
			FCGI_printf("%s", setReplyData(ReplyStatus::kFailed).c_str());
		}
	}
}

void parseClientJson(const char* data, const size_t len, UploadMessage& out)
{
	Json::Reader reader;
	Json::Value root;
	string dataStr(data, len);

	if (!reader.parse(dataStr, root)) {
		LOG(ERROR) << "Json parse error";
		throw runtime_error("Json parse error");
	}

	out.user = root["user"].asString();
	out.token = root["token"].asString();
	out.md5 = root["md5"].asString();
	out.fileName = root["filename"].asString();

	DLOG(INFO) << out.user << " " << out.md5;
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

ReplyStatus checkInMysql(const Utils::MysqlInfo& mysqlInfo, const UploadMessage& uploadMsg)
{
	mysqlx::Session sess(mysqlInfo.host, mysqlInfo.port, mysqlInfo.user,
		mysqlInfo.passwd, mysqlInfo.database);
	mysqlx::Table fileTable = sess.getDefaultSchema().getTable("file_info", true);
	mysqlx::Table userFileTable = sess.getDefaultSchema().getTable("user_file_list", true);

	mysqlx::RowResult rows = fileTable.select("count")
		.where("md5 = :md5")
		.bind("md5", uploadMsg.md5)
		.execute();
	if (rows.count() == 0) {
		// 服务器没有此文件
		return ReplyStatus::kMd5CheckFail;
	}
	// 取得文件引用计数
	auto row = rows.fetchOne();
	int fileCount = row[0];

	// 检查用户有没有此文件
	int isInUserList = userFileTable.select("md5")
		.where("user_name = :name and md5 = :md5")
		.bind("name", uploadMsg.user)
		.bind("md5", uploadMsg.md5)
		.execute().count();
	if (isInUserList != 0) {
		// 用户有此文件
		return ReplyStatus::kUserFileExisted;
	}

	// 添加此文件信息到用户文件列表下
	// 开启事务
	sess.startTransaction();
	// 添加文件到用户列表下
	try {
		userFileTable.insert("user_name", "md5", "file_name")
			.values(uploadMsg.user, uploadMsg.md5, uploadMsg.fileName)
			.execute();
		// file_info中文件引用计数加1
		fileTable.update()
			.set("count", ++fileCount)
			.where("md5 = :md5")
			.bind("md5", uploadMsg.md5)
			.execute();
	}
	catch (const mysqlx::Error& err) {
		sess.rollback();
		return ReplyStatus::kFailed;
	}
	sess.commit();
	return ReplyStatus::kSuccess;
}

string setReplyData(const ReplyStatus stat) {
	Json::Value root;
	root["code"] = getReplyCode(stat);
	return root.toStyledString();
}