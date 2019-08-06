#include "utils_cfg.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

#include <glog/logging.h>
#include <json/json.h>

using namespace std;

void Utils::initConfigStruct()
{
	Json::Value root, mysql, redis, fdfs, webServer, storageServer;
	mysql["host"] = "localhost";
	mysql["user"] = "";
	mysql["passwd"] = "";
	mysql["port"] = 33060;
	mysql["database"] = "";

	redis["ip"] = "127.0.0.1";
	redis["port"] = 6379;

	fdfs["client"] = "/etc/fdfs/client.conf";

	webServer["ip"] = "192.168.91.100";
	webServer["port"] = 80;

	storageServer["ip"] = "192.168.91.100";
	storageServer["port"] = 80;

	root["mysql"] = mysql;
	root["redis"] = redis;
	root["fastdfs"] = fdfs;
	root["web_server"] = webServer;
	root["storage_server"] = storageServer;

	ofstream os(kConfigFile);
	if (!os.is_open()) {
		LOG(ERROR) << "configure file open err";
		throw logic_error("configure file open err");
	}
	os << root;
	os.close();
	return;
}

Json::Value Utils::getConfigDoc()
{
	Json::Value root;
	ifstream is(kConfigFile);
	if (!is.is_open()) {
		LOG(ERROR) << "configure file open err";
		throw logic_error("configure file open err");
	}
	is >> root;
	return root;
}

std::string Utils::getReplyCode(ReplyStatus stat)
{
	string code;
	switch (stat) {
	case Utils::kTokenInvalid:
		code = kTokenInvalidCode;
		break;
	case Utils::kSuccess:
		code = kSuccessCode;
		break;
	case Utils::kFailed:
		code = kFailedCode;
		break;
	case Utils::kUserExisted:
		code = kUserExistedCode;
		break;
	case Utils::kMd5CheckFail:
		code = kMd5CheckFailCode;
		break;
	case Utils::kUserFileExisted:
		code = kUserFileExistedCode;
		break;
	default:
		break;
	}
	return code;
}

void Utils::getFdfsInfo(FdfsInfo& fdfsInfo)
{
	Json::Value root = getConfigDoc();
	fdfsInfo.fdfsConfigFile = root["fastdfs"]["client"].asString();
	fdfsInfo.storageServerIp = root["storage_server"]["ip"].asString();
	fdfsInfo.storageServerPort = root["storage_server"]["port"].asInt();
}

void Utils::getMysqlInfo(MysqlInfo& mysqlInfo)
{
	Json::Value root = getConfigDoc();
	Json::Value mysql = root["mysql"];
	mysqlInfo.host = mysql["host"].asString();
	mysqlInfo.user = mysql["user"].asString();
	mysqlInfo.passwd = mysql["passwd"].asString();
	mysqlInfo.port = mysql["port"].asInt();
	mysqlInfo.database = mysql["database"].asString();
}

void Utils::getRedisInfo(RedisInfo& redisInfo)
{
	Json::Value root = getConfigDoc();
	Json::Value redis = root["redis"];
	redisInfo.ip = redis["ip"].asString();
	redisInfo.port = redis["port"].asInt();
}

