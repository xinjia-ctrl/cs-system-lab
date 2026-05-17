cmake_minimum_required(VERSION 2.8)
project(epoll-http-server CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Stage 1: 单线程阻塞式 Socket 服务器（基准线）
add_executable(stage1_blocking main_stage1.cpp)

# Stage 2: epoll 单线程非阻塞 I/O 多路复用
add_executable(stage2_epoll
    main_stage2.cpp
    src/Socket.cpp
    src/Epoll.cpp
    src/Channel.cpp
    src/EventLoop.cpp
)
