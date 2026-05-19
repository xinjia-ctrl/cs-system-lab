#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <unistd.h>

class Socket {
public:
    Socket();
    explicit Socket(int fd);
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    bool bind(const char* ip, int port);
    bool listen(int backlog = 10);
    int accept(struct sockaddr_in* client_addr = nullptr);
    void setNonBlocking();
    void setReuseAddr();
    int fd() const { return fd_; }

    // Release ownership of fd without closing it
    int release();

private:
    int fd_;
    bool owned_;
};

#endif // SOCKET_H
