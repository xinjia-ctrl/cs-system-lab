#include "HttpServer.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <utility>
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
    // connections_ owns every Connection; clearing the map destroys them.
    connections_.clear();
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

// 新连接到达：设置非阻塞，分发给一个子 Reactor
void HttpServer::onNewConnection(int conn_fd, struct sockaddr_in addr) {
    Socket sock(conn_fd);
    sock.setNonBlocking();
    sock.release();

    EventLoop* subLoop = threadPool_.getNextLoop();
    // 在子 Reactor 线程中创建连接对象
    subLoop->runInLoop([this, conn_fd, subLoop] {
        std::unique_ptr<Connection> conn(new Connection(subLoop, conn_fd));
        Connection* connPtr = conn.get();

        conn->channel->setReadCallback([this, connPtr] {
            onRead(connPtr);
        });
        conn->channel->setWriteCallback([this, connPtr] {
            onWrite(connPtr);
        });
        conn->channel->setCloseCallback([this, connPtr] {
            onClose(connPtr);
        });
        conn->channel->enableReading();

        {
            std::lock_guard<std::mutex> lock(connectionsMutex_);
            connections_[conn_fd] = std::move(conn);
        }
    });
}

// 可读事件：读取数据 → HTTP 解析 → 生成响应 → 发送
void HttpServer::onRead(Connection* conn) {
    int fd = conn->channel->fd();
    char buf[4096];
    int n = read(fd, buf, sizeof(buf));
    if (n <= 0) {
        if (n == 0) {
            removeConnection(conn);
        }
        return;
    }

    // 增量解析 HTTP 请求
    if (!conn->request.parse(buf, static_cast<size_t>(n))) {
        // 解析失败 → 400 Bad Request
        HttpResponse resp;
        resp.setStatusCode(400);
        resp.setBody("Bad Request\n");
        sendResponse(conn, resp);
        return;
    }

    if (!conn->request.gotAll()) {
        return;  // 半包，等更多数据
    }

    // 完整请求 → 回调生成响应
    HttpResponse resp;
    if (callback_) {
        callback_(conn->request, &resp);
    } else {
        resp.setBody("OK\n");
    }
    resp.setKeepAlive(conn->request.keepAlive());

    sendResponse(conn, resp);
}

// 可写事件：继续发送缓冲中未写完的数据
void HttpServer::onWrite(Connection* conn) {
    int fd = conn->channel->fd();
    size_t remaining = conn->writeBuf.size() - conn->writeSent;

    while (remaining > 0) {
        ssize_t n = write(fd,
                          conn->writeBuf.data() + conn->writeSent,
                          remaining);
        if (n > 0) {
            conn->writeSent += static_cast<size_t>(n);
            remaining -= static_cast<size_t>(n);
        } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            // 内核写缓冲区满，等下次 EPOLLOUT
            return;
        } else {
            // 写错误，关闭连接
            removeConnection(conn);
            return;
        }
    }

    // 全部写完
    conn->writeBuf.clear();
    conn->writeSent = 0;
    conn->channel->disableWriting();

    // keep-alive 则继续读下个请求，否则关闭
    if (conn->request.keepAlive()) {
        conn->request.reset();
        // EPOLLIN 一直开启，下一个请求到来时会进入 onRead
    } else {
        removeConnection(conn);
    }
}

// 对端断开
void HttpServer::onClose(Connection* conn) {
    removeConnection(conn);
}

// 构建并发送 HTTP 响应（可能一次写不完，剩余部分靠 EPOLLOUT 继续）
void HttpServer::sendResponse(Connection* conn, const HttpResponse& resp) {
    std::string msg = resp.toMessage();
    int fd = conn->channel->fd();

    // 尝试直接发送
    ssize_t n = write(fd, msg.data(), msg.size());
    if (n < 0 && errno == EAGAIN) {
        n = 0;  // 一个字节都没发出去
    }

    if (n < 0) {
        removeConnection(conn);
        return;
    }

    size_t sent = static_cast<size_t>(n);
    if (sent < msg.size()) {
        // 没发完 → 缓冲剩余数据，注册 EPOLLOUT
        conn->writeBuf.assign(msg.data() + sent, msg.size() - sent);
        conn->writeSent = 0;
        conn->channel->enableWriting();
    } else {
        // 一次发完
        if (resp.keepAlive()) {
            conn->request.reset();
        } else {
            removeConnection(conn);
        }
    }
}

// 清理连接
void HttpServer::removeConnection(Connection* conn) {
    if (conn->closing) {
        return;
    }
    conn->closing = true;

    int fd = conn->channel->fd();
    conn->channel->disableAll();
    ::close(fd);

    // The current callback may still be running inside Channel::handleEvent().
    // Erase later in the same EventLoop turn so the Channel object stays alive
    // until event dispatch returns.
    conn->loop->queueInLoop([this, fd] {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        connections_.erase(fd);
    });
}
