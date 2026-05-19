#include "Epoll.h"
#include <cstdio>
#include <cerrno>
#include <unistd.h>

static const int kMaxEvents = 4096;

Epoll::Epoll() : epfd_(-1) {
    epfd_ = epoll_create1(0);
    if (epfd_ < 0) {
        perror("epoll_create1");
    }
    events_.resize(kMaxEvents);
}

Epoll::~Epoll() {
    if (epfd_ >= 0) {
        ::close(epfd_);
    }
}

void Epoll::addFd(int fd, uint32_t events, void* ptr) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = ptr;
    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("epoll_ctl ADD");
    }
}

void Epoll::modFd(int fd, uint32_t events, void* ptr) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = ptr;
    if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        perror("epoll_ctl MOD");
    }
}

void Epoll::delFd(int fd) {
    if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        perror("epoll_ctl DEL");
    }
}

std::vector<struct epoll_event> Epoll::wait(int timeout_ms) {
    int n = epoll_wait(epfd_, events_.data(), kMaxEvents, timeout_ms);
    if (n < 0) {
        if (errno != EINTR) {
            perror("epoll_wait");
        }
        return {};
    }
    return std::vector<struct epoll_event>(events_.begin(), events_.begin() + n);
}
