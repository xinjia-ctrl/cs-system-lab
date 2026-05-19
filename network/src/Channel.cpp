#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), addedToEpoll_(false) {}

void Channel::handleEvent() {
    // 写回调先执行，避免 EPOLLIN 持续到达时 EPOLLOUT 被饿死
    if ((revents_ & EPOLLOUT) && writeCallback_) {
        writeCallback_();
        // writeCallback 可能删除本 Channel（onWrite→removeConnection）
        return;
    }
    if ((revents_ & EPOLLIN) && readCallback_) {
        readCallback_();
        // readCallback 可能删除本 Channel（onRead→n==0→removeConnection）
        return;
    }
    if ((revents_ & (EPOLLHUP | EPOLLRDHUP)) && closeCallback_) {
        closeCallback_();
        return;
    }
    if ((revents_ & EPOLLERR) && errorCallback_) {
        errorCallback_();
        return;
    }
}

void Channel::update() {
    loop_->updateChannel(this);
}
