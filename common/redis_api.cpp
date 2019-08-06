#include "redis_api.h"
#include <glog/logging.h>

Redis::Redis() :
	conn_(NULL),
	reply_(NULL)
{
}

Redis::~Redis()
{
	if (conn_ != NULL) {
		redisFree(conn_);
	}
}

bool Redis::connect(std::string ip, int port)
{
	conn_ = redisConnect(ip.c_str(), port);
	if (conn_ != NULL && conn_->err) {
		LOG(ERROR) << "Error: " << conn_->errstr;
		return false;
	}
	return true;
}

std::string Redis::get(std::string key)
{
	reply_ = (redisReply*)redisCommand(conn_, "GET %s", key.c_str());
	std::string val = reply_->str;
	freeReplyObject(reply_);
	return val;
}

int Redis::setTimeout(std::string key, std::string value, int seconds)
{
	reply_ = (redisReply*)redisCommand(conn_, "SETEX %s %d %s", 
		key.c_str(), seconds, value.c_str());
	std::string val = reply_->str;
	freeReplyObject(reply_);
	return (val == "OK" ? 1 : 0);
}

int Redis::set(std::string key, std::string value)
{
	reply_ = (redisReply*)redisCommand(conn_, "SET %s %s", key.c_str(), value.c_str());
	std::string val = reply_->str;
	freeReplyObject(reply_);
	return (val == "OK" ? 1 : 0);
}

int Redis::del(std::string key)
{
	reply_ = (redisReply*)redisCommand(conn_, "DEL %s", key.c_str());
	int val = reply_->integer;
	freeReplyObject(reply_);
	return val;
}

std::string Redis::getHash(std::string key, std::string field)
{
	reply_ = (redisReply*)redisCommand(conn_, "HGET %s %s", key.c_str(), field.c_str());
	std::string val = reply_->str;
	freeReplyObject(reply_);
	return val;
}

int Redis::setHash(std::string key, std::string field, std::string value)
{
	reply_ = (redisReply*)redisCommand(conn_, "HSET %s %s %s", 
		key.c_str(), field.c_str(), value.c_str());
	int val = reply_->integer;
	freeReplyObject(reply_);
	return val;
}

int Redis::delHash(std::string key, std::string field)
{
	reply_ = (redisReply*)redisCommand(conn_, "HDEL %s %s",
		key.c_str(), field.c_str());
	int val = reply_->integer;
	freeReplyObject(reply_);
	return val;
}
