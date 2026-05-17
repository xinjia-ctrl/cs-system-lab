#include "Acceptor.h"
#include "EventLoop.h"
#include <cstdio>

Acceptor::Acceptor(EventLoop* loop, int port)
    : loop_(loop),
      listenSocket_(),
      acceptChannel_(loop_, -1),
      started_(false) {

    if (listenSocket_.fd() < 0) return;

    listenSocket_.setReuseAddr();
    if (!listenSocket_.bind(nullptr, port)) return;
    if (!listenSocket_.listen()) return;
    listenSocket_.setNonBlocking();

    printf("Acceptor listening on port %d\n", port);

    acceptChannel_ = Channel(loop_, listenSocket_.fd());
    acceptChannel_.setReadCallback([this] { onAccept(); });
    acceptChannel_.enableReading();
    started_ = true;
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
}

void Acceptor::onAccept() {
    while (true) {
        struct sockaddr_in client_addr;
        int conn_fd = listenSocket_.accept(&client_addr);
        if (conn_fd < 0) break;

        if (newConnCallback_) {
            newConnCallback_(conn_fd, client_addr);
        } else {
            ::close(conn_fd);
        }
    }
}
