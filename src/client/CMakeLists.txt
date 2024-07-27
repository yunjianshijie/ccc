# 这是一个 CMakeLists.txt 文件
cmake_minimum_required(VERSION 3.5.0) # 版本
project(CALC) # 名称
set(CMAKE_CXX_STANDARD 17) # c17
# include_directories(/usr/include)
# include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(client main.cpp 
 ui/user_long.cpp  
 socket.cpp socket1.cpp 
 socket2.cpp 
 ../include/read.cpp
 group.cpp
 file.cpp)
include(FetchContent)

# 收集 client 源文件
# file(GLOB CLIENT_SOURCES
#     "*.cpp"
#     "ui/*.cpp"
# )

# # 创建 client 可执行文件
# add_executable(client ${CLIENT_SOURCES})

# # 链接所需的库
# target_link_libraries(client pthread)