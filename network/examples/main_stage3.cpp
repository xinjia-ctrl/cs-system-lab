#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "Channel.h"
#include "Socket.h"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <utility>
#include <arpa/inet.h>

const int PORT = 8080;
const char* RESPONSE =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "Connection: close\r\n"
    "\r\n"
    "Hello, World!\n";

class ReactorServer {
public:
    ReactorServer(int numThreads = 4)
        : mainLoop_(),
          acceptor_(&mainLoop_, PORT),
          threadPool_(&mainLoop_, numThreads) {

        acceptor_.setNewConnectionCallback(
            [this](int conn_fd, struct sockaddr_in addr) {
                onNewConnection(conn_fd, addr);
            });
    }

    bool start() {
        if (!acceptor_.isRunning()) {
            fprintf(stderr, "Acceptor failed to start\n");
            return false;
        }
        threadPool_.start();
        printf("Stage 3 — Reactor server (1 main + %zu sub reactors)\n",
               threadPool_.getThreadCount());
        return true;
    }

    void run() {
        mainLoop_.loop();
    }

private:
    void onNewConnection(int conn_fd, struct sockaddr_in addr) {
        printf("New connection fd=%d from %s:%d\n",
               conn_fd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        // Set non-blocking
        Socket sock(conn_fd);
        sock.setNonBlocking();
        sock.release();

        // Dispatch to a sub-reactor
        EventLoop* subLoop = threadPool_.getNextLoop();
        subLoop->runInLoop([conn_fd, subLoop, this] {
            std::unique_ptr<Channel> channel(new Channel(subLoop, conn_fd));
            Channel* ch = channel.get();

            ch->setReadCallback([ch, this] { onRead(ch); });
            ch->setCloseCallback([ch, this] { onClose(ch); });
            ch->enableReading();

            std::lock_guard<std::mutex> lock(mutex_);
            clients_[conn_fd] = std::move(channel);
        });
    }

    void onRead(Channel* ch) {
        int fd = ch->fd();
        char buf[4096];
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("[thread %ld] fd=%d received %d bytes\n",
                   (long)std::hash<std::thread::id>{}(std::this_thread::get_id()) % 1000,
                   fd, n);
            write(fd, RESPONSE, strlen(RESPONSE));
        } else if (n == 0) {
            // Peer closed
            removeConnection(ch);
            return;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        } else {
            removeConnection(ch);
            return;
        }

        // Close after one request-response
        removeConnection(ch);
    }

    void onClose(Channel* ch) {
        int fd = ch->fd();
        printf("fd=%d disconnected\n", fd);
        removeConnection(ch);
    }

    void removeConnection(Channel* ch) {
        int fd = ch->fd();
        ch->disableAll();  // Remove from epoll first
        {
            std::lock_guard<std::mutex> lock(mutex_);
            clients_.erase(fd);
        }
        ::close(fd);
        printf("fd=%d closed\n", fd);
    }

    EventLoop mainLoop_;
    Acceptor acceptor_;
    EventLoopThreadPool threadPool_;

    std::mutex mutex_;
    std::unordered_map<int, std::unique_ptr<Channel>> clients_;
};

int main() {
    ReactorServer server(4);
    if (!server.start()) {
        return 1;
    }
    server.run();
    return 0;
}
