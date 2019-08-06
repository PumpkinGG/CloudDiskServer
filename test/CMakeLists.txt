cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(/usr/include/mysql-cppconn-8
	/usr/include/jsoncpp
	${CMAKE_SOURCE_DIR}/common)
link_directories(/usr/local/lib64)

set(CommonDir ${CMAKE_SOURCE_DIR}/common)

add_executable (fastdfs.test "test_fastdfs.cpp" ${CommonDir}/fastdfs_api.cpp
	${CommonDir}/fastdfs_api.h)
target_link_libraries(fastdfs.test -lfdfsclient -lfastcommon -lglog)

add_executable (redis.test "test_redis.cpp"
	${CommonDir}/redis_api.cpp ${CommonDir}/redis_api.cpp)
target_link_libraries(redis.test -lhiredis -lglog)

add_executable (mysql.test "test_mysql.cpp")
target_link_libraries(mysql.test -lmysqlcppconn8 -lglog)

add_executable (ssl.test "test_ssl.cpp"
	${CommonDir}/utils_ssl.cpp ${CommonDir}/utils_ssl.h)
target_link_libraries(ssl.test -lcrypto -lglog)
