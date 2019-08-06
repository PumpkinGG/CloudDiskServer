#include <stdlib.h>
#include <string.h>

#include <hiredis/hiredis.h>
#include <glog/logging.h>

#include "redis_api.h"

using namespace std;

int main(int argc, char* argv[]) 
{
	google::InitGoogleLogging(argv[0]);
	FLAGS_logtostderr = true;
	
	Redis redis;
	redis.connect("127.0.0.1", 6379);

	string val;
	int ret;

	ret = redis.set("GOOD", "LUCK");
	DLOG(INFO) << "Redis SET " << (ret > 0 ? "OK" : "Fail");

	val = redis.get("GOOD");
	DLOG(INFO) << "Redis GET value: " << val;

	ret = redis.del("GOOD");
	DLOG(INFO) << "Redis DEL " << ret << " values";

	ret = redis.setHash("USER_TOKEN", "Pumpkin", "GG");
	DLOG(INFO) << "Redis HSET " << (ret > 0 ? "OK" : "Fail");

	val = redis.getHash("USER_TOKEN", "Pumpkin");
	DLOG(INFO) << "Redis HGET value: " << val;

	ret = redis.delHash("USER_TOKEN", "Pumpkin");
	DLOG(INFO) << "Redis HDEL " << ret << " values";

	google::ShutdownGoogleLogging();
	return 0;
}
