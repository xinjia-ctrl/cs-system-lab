#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "Channel.h"
#include "Socket.h"
#include "HttpContext.h"
#include "HttpResponse.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <mutex>

class HttpServer {
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(int port, int numThreads = 4);
    ~HttpServer();

    void setCallback(const HttpCallback& cb) { callback_ = cb; }

    bool start();
    void run();

private:
    struct Connection {
        Channel* channel;
        HttpContext context;
    };

    void onNewConnection(int conn_fd, struct sockaddr_in addr);
    void onRead(Channel* ch);
    void onClose(Channel* ch);
    void removeConnection(Channel* ch);

    EventLoop mainLoop_;
    Acceptor acceptor_;
    EventLoopThreadPool threadPool_;
    HttpCallback callback_;

    std::mutex mutex_;
    std::unordered_map<int, Connection*> connections_;
};

#endif // HTTPSERVER_H
