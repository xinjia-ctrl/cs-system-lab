#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "Socket.h"
#include "Channel.h"
#include <functional>

class EventLoop;

class Acceptor {
public:
    using NewConnectionCallback = std::function<void(int conn_fd, struct sockaddr_in)>;

    Acceptor(EventLoop* loop, int port);
    ~Acceptor();

    bool isRunning() const { return started_; }

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnCallback_ = cb;
    }

private:
    void onAccept();

    EventLoop* loop_;
    Socket listenSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnCallback_;
    bool started_;
};

#endif // ACCEPTOR_H
