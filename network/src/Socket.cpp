#include "Socket.h"
#include <fcntl.h>
#include <cerrno>

Socket::Socket() : fd_(-1), owned_(true) {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) {
        perror("socket");
    }
}

Socket::Socket(int fd) : fd_(fd), owned_(true) {}

Socket::~Socket() {
    if (owned_ && fd_ >= 0) {
        ::close(fd_);
    }
}

Socket::Socket(Socket&& other) noexcept
    : fd_(other.fd_), owned_(other.owned_) {
    other.fd_ = -1;
    other.owned_ = false;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        if (owned_ && fd_ >= 0) ::close(fd_);
        fd_ = other.fd_;
        owned_ = other.owned_;
        other.fd_ = -1;
        other.owned_ = false;
    }
    return *this;
}

bool Socket::bind(const char* ip, int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (ip) {
        inet_pton(AF_INET, ip, &addr.sin_addr);
    } else {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    if (::bind(fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return false;
    }
    return true;
}

bool Socket::listen(int backlog) {
    if (::listen(fd_, backlog) < 0) {
        perror("listen");
        return false;
    }
    return true;
}

int Socket::accept(struct sockaddr_in* client_addr) {
    socklen_t len = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    if (!client_addr) client_addr = &addr;
    int conn_fd = ::accept(fd_, (struct sockaddr*)client_addr, &len);
    if (conn_fd < 0) {
        // EAGAIN/EWOULDBLOCK is not an error in non-blocking mode
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("accept");
        }
    }
    return conn_fd;
}

void Socket::setNonBlocking() {
    int flags = fcntl(fd_, F_GETFL, 0);
    fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
}

void Socket::setReuseAddr() {
    int opt = 1;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

int Socket::release() {
    owned_ = false;
    return fd_;
}

void Socket::close() {
    if (owned_ && fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}
