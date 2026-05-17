#include "EventLoop.h"
#include "Channel.h"

EventLoop::EventLoop() : quit_(false) {}

void EventLoop::loop() {
    while (!quit_) {
        auto activeEvents = epoll_.wait();

        for (const auto& ev : activeEvents) {
            Channel* ch = static_cast<Channel*>(ev.data.ptr);
            ch->setRevents(ev.events);
            ch->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel* channel) {
    int fd = channel->fd();
    uint32_t events = channel->events();

    if (events == 0) {
        if (channel->addedToEpoll()) {
            epoll_.delFd(fd);
            channel->setAddedToEpoll(false);
        }
    } else if (!channel->addedToEpoll()) {
        epoll_.addFd(fd, events, channel);
        channel->setAddedToEpoll(true);
    } else {
        epoll_.modFd(fd, events, channel);
    }
}
