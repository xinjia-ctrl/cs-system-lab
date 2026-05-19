#include "src/Socket.h"
#include "src/EventLoop.h"
#include "src/Channel.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <arpa/inet.h>

const int PORT = 8080;
const char* RESPONSE =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "Connection: close\r\n"
    "\r\n"
    "Hello, World!\n";

class EchoServer {
public:
    EchoServer()
        : listenSocket_(),
          acceptChannel_(&loop_, -1) {}

    bool start() {
        if (listenSocket_.fd() < 0) {
            perror("socket");
            return false;
        }

        listenSocket_.setReuseAddr();
        if (!listenSocket_.bind(nullptr, PORT)) return false;
        if (!listenSocket_.listen()) return false;
        listenSocket_.setNonBlocking();

        printf("Stage 2 — epoll server listening on port %d\n", PORT);

        acceptChannel_ = Channel(&loop_, listenSocket_.fd());
        acceptChannel_.setReadCallback([this] { onAccept(); });
        acceptChannel_.enableReading();

        return true;
    }

    void run() {
        loop_.loop();
    }

private:
    void onAccept() {
        while (true) {
            struct sockaddr_in client_addr;
            int conn_fd = listenSocket_.accept(&client_addr);
            if (conn_fd < 0) {
                break; // EAGAIN, no more connections
            }

            printf("Accepted connection from %s:%d (fd=%d)\n",
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port), conn_fd);

            // Wrap in Socket to set non-blocking, then release
            Socket conn_sock(conn_fd);
            conn_sock.setNonBlocking();
            conn_sock.release();

            // Create Channel for this connection
            auto ch = new Channel(&loop_, conn_fd);
            ch->setReadCallback([this, ch] { onRead(ch); });
            ch->setCloseCallback([this, ch] { onClose(ch); });
            ch->enableReading();

            clients_[conn_fd] = std::unique_ptr<Channel>(ch);
        }
    }

    void onRead(Channel* ch) {
        int fd = ch->fd();
        char buf[4096];
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("fd=%d received %d bytes\n", fd, n);
            write(fd, RESPONSE, strlen(RESPONSE));
        } else if (n == 0) {
            printf("fd=%d closed by peer\n", fd);
            ch->disableAll();
            clients_.erase(fd);
            ::close(fd);
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            ch->disableAll();
            clients_.erase(fd);
            ::close(fd);
            return;
        }

        // Close after one request-response
        ch->disableAll();
        clients_.erase(fd);
        ::close(fd);
        printf("fd=%d closed\n", fd);
    }

    void onClose(Channel* ch) {
        int fd = ch->fd();
        printf("fd=%d disconnected\n", fd);
        ch->disableAll();
        clients_.erase(fd);
        ::close(fd);
    }

    EventLoop loop_;
    Socket listenSocket_;
    Channel acceptChannel_;
    std::unordered_map<int, std::unique_ptr<Channel>> clients_;
};

int main() {
    EchoServer server;
    if (!server.start()) {
        return 1;
    }
    server.run();
    return 0;
}
