# epoll HTTP Server

从零实现的 C++ 高并发 HTTP 服务器，逐步演进了 4 种 I/O 模型：

> 阻塞式 Socket → epoll 多路复用 → 主从 Reactor (One Loop Per Thread) → HTTP/1.1 状态机解析

底层架构与 Nginx、Netty、muduo 同源。

## 功能特性

- 基于 epoll 的水平触发非阻塞 I/O 多路复用
- 主从 Reactor 模型：1 个主 Reactor 负责 accept + N 个子 Reactor 负责 I/O 读写
- HTTP/1.1 有限状态机解析器（请求行 → 请求头 → 请求体）
- 支持 Connection: keep-alive 长连接
- 非阻塞写缓冲，EPOLLOUT 驱动剩余数据发送
- C++11 实现，零三方依赖

## 四阶段演进

### 阶段 1: 单线程阻塞式 Socket 服务器

最朴素的 accept→read→write→close 循环。一个线程服务所有请求，前一个请求不处理完，后一个只能排队。

**学习要点**：阻塞模型在高并发下的缺陷、C10K 问题的根源

### 阶段 2: epoll 单线程非阻塞 I/O 多路复用

引入 epoll 管理所有连接 fd，单线程同时监听数千个 socket。核心组件：

| 组件 | 职责 |
|---|---|
| `Socket` | RAII 封装 socket fd，自动管理生命周期 |
| `Epoll` | RAII 封装 epoll fd，封装 epoll_ctl/epoll_wait |
| `Channel` | fd 事件分发器，绑定回调函数 |
| `EventLoop` | 事件循环核心，epoll_wait → 回调分发 |

**学习要点**：epoll 与 select/poll 的对比、ET vs LT 模式、非阻塞 I/O

### 阶段 3: 主从 Reactor 模型 (One Loop Per Thread)

引入多线程，主 Reactor 只负责 accept，从 Reactor（多个）负责连接读写。新增组件：

| 组件 | 职责 |
|---|---|
| `Acceptor` | 监听 fd 的 accept 封装 |
| `EventLoopThread` | 每个线程跑一个独立 EventLoop |
| `EventLoopThreadPool` | 子 Reactor 线程池，轮询分发新连接 |
| EventLoop 跨线程唤醒 | eventfd + pending functor 队列 |

```
                         ┌────────────────┐
                         │  Main Reactor   │  线程 1
                         │  Acceptor       │
                         └───────┬────────┘
                                 │ 分发新连接
                ┌────────────────┼────────────────┐
                ▼                ▼                ▼
        ┌──────────────┐ ┌──────────────┐ ┌──────────────┐
        │ Sub Reactor 1│ │ Sub Reactor 2│ │ Sub Reactor 3│ 线程 2~N
        │ epoll + 读写  │ │ epoll + 读写  │ │ epoll + 读写  │
        └──────────────┘ └──────────────┘ └──────────────┘
```

**学习要点**：Reactor vs Proactor、多线程竞争、锁粒度控制、One Loop Per Thread

### 阶段 4: HTTP/1.1 协议解析（有限状态机）

实现 HTTP 请求的状态机解析。核心流程：

```
kExpectRequestLine  →  解析 "GET /path HTTP/1.1\r\n"
       ↓
kExpectHeaders      →  逐行解析 "Host: ...\r\n" 直到空行
       ↓
kExpectBody         →  按 Content-Length 读取 body
       ↓
kGotAll             →  解析完成，交给上层回调生成响应
```

支持半包处理：数据未到齐时不阻塞，等待下次 epoll 事件继续解析。

## 快速开始

### 环境要求

- Linux (epoll)
- g++ 4.8+ (支持 C++11)
- cmake 2.8+
- make

### 编译

```bash
cd network
mkdir build && cd build
cmake ..
make
```

### 运行测试

```bash
# 阶段 4（包含完整 HTTP 解析）
./stage4_http &

# 测试
curl http://localhost:8080/
curl http://localhost:8080/hello
curl http://localhost:8080/notexist  # 404

# 压测
ab -n 10000 -c 100 http://localhost:8080/
```

## 压测对比

在 CentOS 7.6 (VM, 单核 2GB) 上使用 ApacheBench 测试，10000 请求，100 并发：

| 阶段 | 模型 | 吞吐量 (req/s) | 平均延迟 (ms) | 失败数 |
|------|------|----------------|---------------|--------|
| 2 | epoll 单线程 | ~9700 | ~10.3 | 0 |
| 3 | Reactor 多线程 | ~6900 | ~14.4 | 0 |
| 4 | HTTP 状态机 | ~8000 | ~12.5 | 0 |

> 阶段 3 略低于阶段 2 的原因：单核 VM 上多线程上下文切换开销 > 并行收益。
> 多核环境下 Reactor 模型优势会更明显（一核对应一个子 Reactor）。

## 项目结构

```
network/
├── CMakeLists.txt          # 构建配置
├── main_stage1.cpp         # 阶段 1 入口
├── main_stage2.cpp         # 阶段 2 入口
├── main_stage3.cpp         # 阶段 3 入口
├── main_stage4.cpp         # 阶段 4 入口
├── src/
│   ├── Socket.h/.cpp       # RAII socket 封装
│   ├── Epoll.h/.cpp        # RAII epoll 封装
│   ├── Channel.h/.cpp      # fd 事件分发器
│   ├── EventLoop.h/.cpp    # 事件循环
│   ├── Acceptor.h/.cpp     # accept 封装
│   ├── EventLoopThread.h/.cpp      # 单线程 EventLoop
│   ├── EventLoopThreadPool.h/.cpp  # Reactor 线程池
│   ├── HttpRequest.h/.cpp  # HTTP 请求解析（状态机）
│   ├── HttpResponse.h/.cpp # HTTP 响应构建
│   └── HttpServer.h/.cpp   # HTTP + Reactor 整合
├── benchmark/
│   └── run.sh              # 一键压测脚本
└── README.md
```

## 关联知识

- **I/O 多路复用**：select / poll / epoll 的原理与对比
- **Reactor 模式**：事件驱动、非阻塞 I/O、回调机制
- **C10K 问题**：单机处理一万并发连接的瓶颈与解法
- **HTTP 协议**：请求/响应格式、Content-Length、keep-alive
- **有限状态机**：网络协议解析的经典设计模式
- **线程模型**：单线程 vs 多线程、One Loop Per Thread、锁竞争

## 参考资料

- [《Linux 多线程服务端编程》陈硕](https://book.douban.com/subject/20471211/)
- [CS144: Introduction to Computer Networking](https://cs144.stanford.edu/)
- [The C10K problem](http://www.kegel.com/c10k.html)
