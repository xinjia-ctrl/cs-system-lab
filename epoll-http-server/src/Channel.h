#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <sys/epoll.h>

class EventLoop;

class Channel {
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);
    ~Channel() = default;

    int fd() const { return fd_; }
    uint32_t events() const { return events_; }
    uint32_t revents() const { return revents_; }
    bool addedToEpoll() const { return addedToEpoll_; }
    void setAddedToEpoll(bool v) { addedToEpoll_ = v; }

    void setReadCallback(const EventCallback& cb) { readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
    void setCloseCallback(const EventCallback& cb) { closeCallback_ = cb; }
    void setErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }

    void enableReading() { events_ |= EPOLLIN; update(); }
    void enableWriting() { events_ |= EPOLLOUT; update(); }
    void disableReading() { events_ &= ~EPOLLIN; update(); }
    void disableWriting() { events_ &= ~EPOLLOUT; update(); }
    void disableAll() { events_ = 0; update(); }

    void setRevents(uint32_t revents) { revents_ = revents; }

    void handleEvent();

private:
    void update();

    EventLoop* loop_;
    int fd_;
    uint32_t events_;
    uint32_t revents_;
    bool addedToEpoll_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

#endif // CHANNEL_H
