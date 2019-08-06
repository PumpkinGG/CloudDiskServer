#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdexcept>

#include <fcgi_stdio.h>
#include <mysqlx/xdevapi.h>
#include <glog/logging.h>
#include <json/json.h>

#include "logup.h"
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

void doWork() {
	// Read config info
	MysqlInfo mysql;
	getMysqlInfo(mysql);

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
			LogupMessage logupMsg;
			ReplyStatus replyStat;
			try {
				parseLogupData(data, contentLen, logupMsg);
				replyStat = dealWithMysql(mysql, logupMsg);
			}
			catch (const mysqlx::Error& err) {
				LOG(ERROR) << err.what();
				replyStat = ReplyStatus::kFailed;
			}
			catch (exception& ex) {
				LOG(ERROR) << ex.what();
				replyStat = ReplyStatus::kFailed;
			}

			// release here
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

void parseLogupData(const char* data, const size_t len, LogupMessage& out)
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
	out.nickname = root["nickname"].asString();
	out.email = root["email"].asString();
	out.phone = "";
}


string setReplyData(const ReplyStatus stat) {
	Json::Value root;
	root["code"] = getReplyCode(stat);
	return root.toStyledString();
}


ReplyStatus dealWithMysql(const MysqlInfo& mysql, const LogupMessage& logupMsg) {
	mysqlx::Session sess(mysql.host, mysql.port, mysql.user, 
		mysql.passwd, mysql.database);
	mysqlx::Table userTable = sess.getDefaultSchema().getTable("user", true);

	int rows = userTable.select("name")
		.where("name = :name")
		.bind("name", logupMsg.user)
		.execute().count();
	if (rows != 0) {
		return kUserExisted;
	}

	try {
		userTable.insert("name", "password", "nickname", "email")
			.values(logupMsg.user, getStrMd5(logupMsg.password), logupMsg.nickname, logupMsg.email)
			.execute();
	}
	catch (const mysqlx::Error& err) {
		LOG(ERROR) << err;
		return kFailed;
	}

	DLOG(INFO) << "user: " << logupMsg.user << " created success";
	return kSuccess;
}