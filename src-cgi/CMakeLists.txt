cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(/usr/include/mysql-cppconn-8
	/usr/include/jsoncpp
	${CMAKE_SOURCE_DIR}/common)

set(CommonDir ${CMAKE_SOURCE_DIR}/common)

# 注册
add_executable(logup.cgi "logup.cpp" "logup.h" 
	${CommonDir}/utils_cfg.cpp ${CommonDir}/utils_cfg.h
	${CommonDir}/utils_ssl.cpp ${CommonDir}/utils_ssl.h)
target_link_libraries(logup.cgi -lfcgi -lmysqlcppconn8 -ljsoncpp -lcrypto -lglog)


# 登录
add_executable(login.cgi "login.cpp" "login.h" 
	${CommonDir}/redis_api.cpp ${CommonDir}/redis_api.h ${CommonDir}/redis_keys.h 
	${CommonDir}/utils_cfg.cpp ${CommonDir}/utils_cfg.h
	${CommonDir}/utils_ssl.cpp ${CommonDir}/utils_ssl.h)
target_link_libraries(login.cgi -lfcgi -lmysqlcppconn8 -lhiredis -ljsoncpp -lcrypto -lglog)


# MD5秒传
add_executable(uploadmd5.cgi "upload_md5.cpp" "upload_md5.h" 
	${CommonDir}/redis_api.cpp ${CommonDir}/redis_api.h ${CommonDir}/redis_keys.h 
	${CommonDir}/utils_cfg.cpp ${CommonDir}/utils_cfg.h)
target_link_libraries(uploadmd5.cgi -lfcgi -lhiredis
	-lmysqlcppconn8 -ljsoncpp -lglog)


# 上传
add_executable(upload.cgi "upload.cpp" "upload.h" 
	${CommonDir}/fastdfs_api.cpp ${CommonDir}/fastdfs_api.h 
	${CommonDir}/utils_cfg.cpp ${CommonDir}/utils_cfg.h)
target_link_libraries(upload.cgi -lfcgi -lfdfsclient -lfastcommon
	-lmysqlcppconn8 -ljsoncpp -lglog)


# 文件列表
add_executable(userfile.cgi "userfile.cpp" "userfile.h" 
	${CommonDir}/redis_api.cpp ${CommonDir}/redis_api.h ${CommonDir}/redis_keys.h 
	${CommonDir}/utils_cfg.cpp ${CommonDir}/utils_cfg.h)
target_link_libraries(userfile.cgi -lfcgi -lhiredis
	-lmysqlcppconn8 -ljsoncpp -lglog)