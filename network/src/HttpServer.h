#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "Channel.h"
#include "Socket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <string>

class HttpServer {
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(int port, int numThreads = 4);
    ~HttpServer();

    void setCallback(const HttpCallback& cb) { callback_ = cb; }

    bool start();
    void run();

private:
    // 每个连接持有一个 Channel + HTTP 解析上下文 + 响应写缓冲
    struct Connection {
        Channel* channel;
        HttpRequest request;
        std::string writeBuf;   // 待发送的响应数据缓存
        size_t writeSent;       // 已发送的字节数

        Connection(EventLoop* loop, int fd)
            : channel(nullptr), writeSent(0) {
            channel = new Channel(loop, fd);
        }
        ~Connection() { delete channel; }
    };

    void onNewConnection(int conn_fd, struct sockaddr_in addr);
    void onRead(Connection* conn);
    void onWrite(Connection* conn);
    void onClose(Connection* conn);
    void sendResponse(Connection* conn, const HttpResponse& resp);
    void removeConnection(Connection* conn);

    EventLoop mainLoop_;
    Acceptor acceptor_;
    EventLoopThreadPool threadPool_;
    HttpCallback callback_;

    std::mutex connectionsMutex_;
    std::unordered_map<int, Connection*> connections_;
};

#endif // HTTPSERVER_H
