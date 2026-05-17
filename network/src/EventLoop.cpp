#include "EventLoop.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <cstdio>

EventLoop::EventLoop()
    : quit_(false),
      wakeupFd_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      wakeupChannel_(this, wakeupFd_),
      callingPendingFunctors_(false),
      threadId_(std::this_thread::get_id()) {

    if (wakeupFd_ < 0) {
        perror("eventfd");
    }

    wakeupChannel_.setReadCallback([this] { handleWakeup(); });
    wakeupChannel_.enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_.disableAll();
    if (wakeupFd_ >= 0) {
        ::close(wakeupFd_);
    }
}

void EventLoop::loop() {
    quit_ = false;
    while (!quit_) {
        auto activeEvents = epoll_.wait();

        for (const auto& ev : activeEvents) {
            Channel* ch = static_cast<Channel*>(ev.data.ptr);
            ch->setRevents(ev.events);
            ch->handleEvent();
        }

        doPendingFunctors();
    }
}

void EventLoop::quit() {
    quit_ = true;
    wakeup();
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

void EventLoop::runInLoop(const Functor& cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }
    wakeup();
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    write(wakeupFd_, &one, sizeof(one));
}

void EventLoop::handleWakeup() {
    uint64_t one;
    read(wakeupFd_, &one, sizeof(one));
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (auto& f : functors) {
        f();
    }

    callingPendingFunctors_ = false;
}
