#pragma once

#include <string>
#include <vector>
#include "utils_cfg.h"

struct UserFileMessage {
	std::string user;
	std::string token;
};

struct FileInfo {
	std::string md5;               // 文件md5码
	std::string filename;          // 文件名字
	std::string user;              // 用户
	std::string timestamp;         // 上传时间
	std::string url;               // url
	std::string type;              // 文件类型
	int size;                      // 文件大小
	int shareStatus;               // 是否共享, 1共享， 0不共享
	int downloadCount;             // 下载量
};

/*
 * GetUserFile working function
 */
void doWork();

/*
 * Parse Client Json data
 */
void parseClientJson(const char* data, const size_t len, UserFileMessage& out);

/*
 * Check Token in Redis
 * if token is valid return true, else return false
 */
bool checkToken(const Utils::RedisInfo& redisInfo,
	const std::string& user, const std::string& token); 

/*
 * Check user files in mysql
 */
Utils::ReplyStatus checkInMysql(const Utils::MysqlInfo& mysqlInfo,
	const UserFileMessage& userMsg, std::vector<FileInfo>& files);

/*
 * Pack reply post json data
 */
std::string setReplyData(const Utils::ReplyStatus stat,
	const std::vector<FileInfo>& files);