#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), addedToEpoll_(false) {}

void Channel::handleEvent() {
    // readCallback 可能通过 onRead→removeConnection 删除本 Channel
    // 因此每次回调后立即返回，不再访问 this 成员
    if ((revents_ & EPOLLIN) && readCallback_) {
        readCallback_();
        return;
    }
    if (revents_ & (EPOLLHUP | EPOLLRDHUP)) {
        if (closeCallback_) closeCallback_();
        return;
    }
    if ((revents_ & EPOLLERR) && errorCallback_) {
        errorCallback_();
    }
    if ((revents_ & EPOLLOUT) && writeCallback_) {
        writeCallback_();
    }
}

void Channel::update() {
    loop_->updateChannel(this);
}
