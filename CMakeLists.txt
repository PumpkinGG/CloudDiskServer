# CMakeList.txt: FastDFS 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)
# Windows VS2019需要在顶层CMakeLists里添加include目录，否则引用工程内其他目录头文件会标错
# 若不添加，Cmake编译不会报错
include_directories(./common)

add_subdirectory("test")
add_subdirectory("src-cgi")
