#pragma once

#include <string>
#include <hiredis/hiredis.h>

class Redis {
public:
	Redis();
	~Redis();

public:
	bool connect(std::string ip, int port);

	// String
	std::string get(std::string key);
	int setTimeout(std::string key, std::string value, int seconds);
	int set(std::string key, std::string value);
	int del(std::string key);

	// Hash
	std::string getHash(std::string key, std::string field);
	int setHash(std::string key, std::string field, std::string value);
	int delHash(std::string key, std::string field);

private:
	redisContext* conn_;
	redisReply* reply_;

};