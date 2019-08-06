#pragma once

#include <string>
#include "utils_cfg.h"

// 客户端发来的注册信息结构体
struct LoginMessage {
	std::string user;
	std::string password;
};

/*
 * Login working function
 */
void doWork();

/*
 * Parse client post json data
 */
void parseLoginData(const char* data, const size_t len, LoginMessage& out);

/*
 * Check user in MySQL database
 */
Utils::ReplyStatus checkInMySQL(const Utils::MysqlInfo& mysql,
	const LoginMessage& loginMsg, std::string& nickname);

/*
 * Form reply post json data
 */
std::string setReplyData(const Utils::ReplyStatus stat,
	const std::string& nickname, const std::string& token);

/*
 * Generate token & save it to redis
 * If success, return token, else return ""
 */
std::string genAndSaveToken(const Utils::RedisInfo& redisInfo,
	const LoginMessage& loginMsg);