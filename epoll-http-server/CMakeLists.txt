cmake_minimum_required(VERSION 3.10)
project(epoll-http-server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_library(network_core
    src/Socket.cpp
    src/Epoll.cpp
    src/Channel.cpp
    src/EventLoop.cpp
    src/Acceptor.cpp
    src/EventLoopThread.cpp
    src/EventLoopThreadPool.cpp
    src/HttpRequest.cpp
    src/HttpResponse.cpp
    src/HttpServer.cpp
)
target_include_directories(network_core PUBLIC src)
target_link_libraries(network_core PUBLIC Threads::Threads)

function(add_stage name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE network_core)
endfunction()

# Stage 1: 单线程阻塞式 Socket 服务器（基准线）
add_executable(stage1_blocking examples/main_stage1.cpp)

# Stage 2: epoll 单线程非阻塞 I/O 多路复用
add_stage(stage2_epoll examples/main_stage2.cpp)

# Stage 3: 主从 Reactor 模型（One Loop Per Thread）
add_stage(stage3_reactor examples/main_stage3.cpp)

# Stage 4: HTTP 协议解析 + Reactor HTTP 服务器
add_stage(stage4_http examples/main_stage4.cpp)
