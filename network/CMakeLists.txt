cmake_minimum_required(VERSION 2.8)
project(epoll-http-server CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Stage 1: 单线程阻塞式 Socket 服务器（基准线）
add_executable(stage1_blocking main_stage1.cpp)
