#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, int numThreads)
    : baseLoop_(baseLoop), numThreads_(numThreads), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {
    for (auto& t : threads_) {
        delete t;
    }
}

void EventLoopThreadPool::start() {
    for (int i = 0; i < numThreads_; ++i) {
        EventLoopThread* t = new EventLoopThread();
        EventLoop* loop = t->startLoop();
        threads_.push_back(t);
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
