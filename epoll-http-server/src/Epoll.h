#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <vector>

class Epoll {
public:
    Epoll();
    ~Epoll();

    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;

    void addFd(int fd, uint32_t events, void* ptr);
    void modFd(int fd, uint32_t events, void* ptr);
    void delFd(int fd);

    std::vector<struct epoll_event> wait(int timeout_ms = -1);
    int fd() const { return epfd_; }

private:
    int epfd_;
    std::vector<struct epoll_event> events_;
};

#endif // EPOLL_H
