
# cmake_minimum_required(VERSION 3.5.0) # 版本
project(CALC) # 名称
#set(CMAKE_CXX_STANDARD 17) # c17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# 添加头文件路径
# include_directories(${PROJECT_SOURCE_DIR}/include)
# include_directories(${PROJECT_SOURCE_DIR}/chuli)
# include_directories(${PROJECT_SOURCE_DIR}/socket)
# include_directories(${PROJECT_SOURCE_DIR}/epoll)
# include_directories(${PROJECT_SOURCE_DIR}/chuli)
# CFLAGS = -I/usr/include/hiredis
# LDFLAGS = -L/usr/lib -lhiredis
# # Add the path to hiredis CMake module files
# list(APPEND CMAKE_MODULE_PATH "/usr/include/hiredis")
# # # Find hiredis package
# find_package(hiredis REQUIRED)
# # # Include hiredis directories
#  include_directories(${HIREDIS_INCLUDE_DIRS})
# # Link hiredis libraries to the target after the target sources

add_executable(server main.cpp 
 chuli/redis.cpp
 socket.cpp epoll.cpp
  chuli/chuli.cpp
   ../include/read.cpp)

target_link_libraries(server hiredis)

# include(FetchContent)
# # 收集 server 源文件
# file(GLOB SERVER_SOURCES
#     "*.cpp"
#     "chuli/*.cpp"
# )

# # 创建 server 可执行文件
# add_executable(server ${SERVER_SOURCES})

# # 链接所需的库
# target_link_libraries(server pthread hiredis redis)