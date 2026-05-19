#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <vector>
#include <cstddef>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop* baseLoop, int numThreads);
    ~EventLoopThreadPool();

    void start();
    // 仅被主 Reactor 线程调用（Acceptor 回调中），无需同步
    EventLoop* getNextLoop();
    size_t getThreadCount() const { return loops_.size(); }

private:
    EventLoop* baseLoop_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

#endif // EVENTLOOPTHREADPOOL_H
