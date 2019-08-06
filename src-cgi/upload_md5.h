#pragma once

#include <string>
#include "utils_cfg.h"

// 客户端发来消息结构
struct UploadMessage {
	std::string user;
	std::string token;
	std::string md5;
	std::string fileName;
};

/*
 * Upload MD5 working function
 */
void doWork();

/*
 * Parse Client Json data
 */
void parseClientJson(const char* data, const size_t len, UploadMessage& out);

/*
 * Check Token in Redis 
 * if token is valid return true, else return false
 */
bool checkToken(const Utils::RedisInfo& redisInfo, 
	const std::string& user, const std::string& token);

/*
 * Check file Md5 in mysql
 */
Utils::ReplyStatus checkInMysql(const Utils::MysqlInfo& mysqlInfo, 
	const UploadMessage& uploadMsg);

/*
 * Pack reply post json data
 */
std::string setReplyData(const Utils::ReplyStatus stat);