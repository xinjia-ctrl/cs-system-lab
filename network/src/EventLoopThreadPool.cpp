#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include <utility>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, int numThreads)
    : baseLoop_(baseLoop), numThreads_(numThreads), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {
    // EventLoopThread joins its own thread in the destructor.
}

void EventLoopThreadPool::start() {
    for (int i = 0; i < numThreads_; ++i) {
        std::unique_ptr<EventLoopThread> t(new EventLoopThread());
        EventLoop* loop = t->startLoop();
        threads_.push_back(std::move(t));
        loops_.push_back(loop);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    if (loops_.empty()) {
        return baseLoop_;
    }
    EventLoop* loop = loops_[next_];
    next_ = (next_ + 1) % loops_.size();
    return loop;
}
