#include "HttpServer.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>

HttpServer::HttpServer(int port, int numThreads)
    : acceptor_(&mainLoop_, port),
      threadPool_(&mainLoop_, numThreads) {

    acceptor_.setNewConnectionCallback(
        [this](int conn_fd, struct sockaddr_in addr) {
            onNewConnection(conn_fd, addr);
        });
}

HttpServer::~HttpServer() {
    for (auto& kv : connections_) {
        delete kv.second->channel;
        delete kv.second;
    }
}

bool HttpServer::start() {
    if (!acceptor_.isRunning()) {
        fprintf(stderr, "HttpServer: Acceptor failed to start\n");
        return false;
    }
    threadPool_.start();
    printf("HttpServer started (%zu sub reactors)\n",
           threadPool_.getThreadCount());
    return true;
}

void HttpServer::run() {
    mainLoop_.loop();
}

void HttpServer::onNewConnection(int conn_fd, struct sockaddr_in addr) {
    Socket sock(conn_fd);
    sock.setNonBlocking();
    sock.release();

    EventLoop* subLoop = threadPool_.getNextLoop();
    subLoop->runInLoop([this, conn_fd, subLoop] {
        auto conn = new Connection();
        conn->channel = new Channel(subLoop, conn_fd);
        conn->channel->setReadCallback([this, conn] {
            onRead(conn->channel);
        });
        conn->channel->setCloseCallback([this, conn] {
            onClose(conn->channel);
        });
        conn->channel->enableReading();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            connections_[conn->channel->fd()] = conn;
        }
    });
}

void HttpServer::onRead(Channel* ch) {
    int fd = ch->fd();
    char buf[4096];
    int n = read(fd, buf, sizeof(buf));
    if (n <= 0) {
        if (n == 0) {
            // Peer closed
            removeConnection(ch);
        }
        return;
    }

    // Find connection
    Connection* conn = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = connections_.find(fd);
        if (it != connections_.end()) {
            conn = it->second;
        }
    }
    if (!conn) return;

    // Parse HTTP request
    if (!conn->context.parse(buf, (size_t)n)) {
        // Parse error — bad request
        HttpResponse resp;
        resp.setStatusCode(400);
        resp.setBody("Bad Request\n");
        std::string msg = resp.toMessage();
        write(fd, msg.data(), msg.size());
        removeConnection(ch);
        return;
    }

    if (!conn->context.gotAll()) {
        // Need more data, wait for next read
        return;
    }

    // Request complete — build response
    HttpResponse resp;
    if (callback_) {
        callback_(conn->context.request(), &resp);
    } else {
        resp.setBody("OK\n");
    }

    // Set keep-alive
    resp.setKeepAlive(conn->context.request().keepAlive());

    // Send response
    std::string msg = resp.toMessage();
    write(fd, msg.data(), msg.size());

    // Decide to keep or close
    if (conn->context.request().keepAlive()) {
        conn->context.reset();
        // Don't close, wait for next request
    } else {
        removeConnection(ch);
    }
}

void HttpServer::onClose(Channel* ch) {
    removeConnection(ch);
}

void HttpServer::removeConnection(Channel* ch) {
    int fd = ch->fd();
    ch->disableAll();

    Connection* conn = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = connections_.find(fd);
        if (it != connections_.end()) {
            conn = it->second;
            connections_.erase(it);
        }
    }
    if (!conn) return;

    ::close(fd);
    {
        // Make sure channel is deleted after fd is closed
        delete conn->channel;
    }
    delete conn;
    printf("fd=%d closed\n", fd);
}
