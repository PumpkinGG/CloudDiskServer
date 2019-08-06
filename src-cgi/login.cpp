#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <stdexcept>

#include <fcgi_stdio.h>
#include <mysqlx/xdevapi.h>
#include <glog/logging.h>
#include <json/json.h>

#include "login.h"
#include "redis_api.h"
#include "redis_keys.h"
#include "utils_cfg.h"
#include "utils_ssl.h"

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


void doWork()
{
	// Read config info
	MysqlInfo mysql;
	RedisInfo redis;
	getMysqlInfo(mysql);
	getRedisInfo(redis);

	// working loop
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
			LoginMessage loginMsg;
			ReplyStatus replyStat;
			string replyJsonData;
			string nickname, token;
			try {
				parseLoginData(data, contentLen, loginMsg);
				replyStat = checkInMySQL(mysql, loginMsg, nickname);
				token = genAndSaveToken(redis, loginMsg);
			}
			catch (const mysqlx::Error& err) {
				LOG(ERROR) << "mysqlx error " << err.what();
				replyStat = ReplyStatus::kFailed;
			}
			catch (const exception& ex) {
				LOG(ERROR) << ex.what();
				replyStat = ReplyStatus::kFailed;
			}
			replyJsonData = setReplyData(replyStat, nickname, token);

			// release here
			::free(data);
			FCGI_printf("Content-type: %s\r\n\r\n", kJsonContentType.c_str());
			FCGI_printf("%s", replyJsonData.c_str());
		}
		else {
			FCGI_printf("Content-type: %s\r\n\r\n", kJsonContentType.c_str());
			FCGI_printf("%s", setReplyData(ReplyStatus::kFailed, "", "").c_str());
		}
	}
}

void parseLoginData(const char* data, const size_t len, LoginMessage& out)
{
	Json::Reader reader;
	Json::Value root;
	string dataStr(data, len);

	if (!reader.parse(dataStr, root)) {
		LOG(ERROR) << "Json parse error";
		throw runtime_error("Json parse error");
	}

	out.user = root["user"].asString();
	out.password = root["password"].asString();
}

std::string setReplyData(const ReplyStatus stat, const std::string& nickname, const std::string& token)
{
	Json::Value root;
	root["code"] = getReplyCode(stat);
	root["nickname"] = nickname;
	root["token"] = token;
	return root.toStyledString();
}

string genAndSaveToken(const RedisInfo& redisInfo, const LoginMessage& loginMsg)
{
	// 连接Redis
	Redis redis;
	if (!redis.connect(redisInfo.ip, redisInfo.port)) {
		throw runtime_error("Redis server connect failed");
	}

	// Generate key
	string key, token;
	key += loginMsg.user;
	int randNum;
	::srand((unsigned int)::time(NULL));
	for (int i = 0; i < 4; i++) {
		randNum = ::rand() % 1000;
		key += to_string(randNum);
	}

	// Generate token
	token = Utils::aesEncToBase64(key);
	token = Utils::getStrMd5(token);

	// Save token to redis
	/*
		@TODO...
	*/
	string redisKey = kTokenPrefix + loginMsg.user;
	if (!redis.setTimeout(redisKey, token, kTokenExpiration)) {
		throw runtime_error("Save token to Redis failed");
	}
	return token;
}

ReplyStatus checkInMySQL(const MysqlInfo& mysql, const LoginMessage& loginMsg, std::string& nickname)
{
	mysqlx::Session sess(mysql.host, mysql.port, mysql.user,
		mysql.passwd, mysql.database);
	mysqlx::Table userTable = sess.getDefaultSchema().getTable("user", true);

	mysqlx::RowResult rows = userTable.select("password", "nickname")
		.where("name = :name")
		.bind("name", loginMsg.user)
		.execute();

	if (rows.count() == 0) {
		return ReplyStatus::kFailed;
	}

	mysqlx::Row row = rows.fetchOne();
	string password = (string)row[0];
	if (password != loginMsg.password) {
		return ReplyStatus::kFailed;
	}

	nickname = (string)row[1];
	// 去掉中文字符串结尾的\x00
	if ((unsigned char)nickname.back() == 0) {
		nickname.pop_back();
	}
	return ReplyStatus::kSuccess;
}
