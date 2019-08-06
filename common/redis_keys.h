#pragma once

#include <string>

/*--------------------------------------------------------
| 用户登录服务器生成的Token, 以Key: Value形式存储到Redis
| 以kTokenPrefix前缀标识key
| 通过 SETEX key seconds value 设置过期时间, 12小时过期
`--------------------------------------------------------*/
const std::string kTokenPrefix = "TOKEN:";
const int kTokenExpiration = 60 * 60 * 12;

/*--------------------------------------------------------
| 共享用户文件有序集合 (ZSET)
| Key:     kFilePublicZset
| value:   md5文件名
| redis 语句
|   ZADD key score member 添加成员
|   ZREM key member 删除成员
|   ZREVRANGE key start stop [WITHSCORES] 降序查看
|   ZINCRBY key increment member 权重累加increment
|   ZCARD key 返回key的有序集元素个数
|   ZSCORE key member 获取某个成员的分数
|   ZREMRANGEBYRANK key start stop 删除指定范围的成员
|   zlexcount zset [member [member 判断某个成员是否存在，存在返回1，不存在返回0
`---------------------------------------------------------*/
const std::string kFilePublicZset = "FILE_PUBLIC_ZSET";

/*-------------------------------------------------------
| 文件标示和文件名对应表 (HASH)
| Key:    kFileNameHash
| field:  file_id(md5文件名)
| value:  file_name
| redis 语句
|    hset key field value
|    hget key field
`--------------------------------------------------------*/
const std::string kFileNameHash = "FILE_NAME_HASH";