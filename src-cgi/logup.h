#pragma once

#include <string>
#include <stdio.h>
#include "utils_cfg.h"

// 客户端发来的注册信息结构体
struct LogupMessage {
	std::string user;
	std::string password;
	std::string nickname;
	std::string email;
	std::string phone;
};

/*
 * Upload working function
 */
void doWork();

/*
 * Parse client post json data
 */
void parseLogupData(const char* data, const size_t len, LogupMessage& out);

/*
 * Form reply post json data
 */
std::string setReplyData(const Utils::ReplyStatus stat);

/*
 * Insert logup info to mysql
 */
Utils::ReplyStatus dealWithMysql(const Utils::MysqlInfo& mysql, 
	const LogupMessage& logupMsg);