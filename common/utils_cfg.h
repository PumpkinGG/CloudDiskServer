#pragma once

#include <string>
#include <json/json.h>

namespace Utils {

// 固定配置信息
const std::string kConfigFile = "config/config.json";
const std::string kLogPath = "logs";
const std::string kFormContentType = "multipart/form-data";
const std::string kJsonContentType = "application/json";
const std::string kHtmlContentType = "text/html";

// 服务器回复状态
enum ReplyStatus {
	kTokenInvalid,
	kSuccess,
	kFailed,
	kUserExisted,
	kMd5CheckFail,
	kUserFileExisted
};

// 服务器回复状态码
const std::string kTokenInvalidCode = "110";
const std::string kSuccessCode = "001";
const std::string kFailedCode = "002";
const std::string kUserExistedCode = "003";
const std::string kMd5CheckFailCode = "004";
const std::string kUserFileExistedCode = "005";

// MySQL配置信息
struct MysqlInfo {
	std::string host;
	std::string user;
	std::string passwd;
	std::string database;
	int port;
};

// FastDFS配置信息
struct FdfsInfo {
	std::string fdfsConfigFile;
	std::string storageServerIp;
	int storageServerPort;
};

// Redis配置信息
struct RedisInfo {
	std::string ip;
	int port;
};

// 读取fdfs配置信息
void getFdfsInfo(FdfsInfo& fdfsInfo);

// 读取mysql配置信息
void getMysqlInfo(MysqlInfo& mysqlInfo);

// 读取redis配置信息
void getRedisInfo(RedisInfo& redisInfo);

// 初始化配置文件格式及参数，方便生成配置文件
void initConfigStruct();

// 从配置文件中读取Json文档
Json::Value getConfigDoc();

// 获取状态码
std::string getReplyCode(ReplyStatus stat);

} // namespace Utils