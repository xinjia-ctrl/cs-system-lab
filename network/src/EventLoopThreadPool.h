#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <vector>
#include <cstddef>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop* baseLoop, int numThreads);
    ~EventLoopThreadPool();

    void start();
    EventLoop* getNextLoop();
    size_t getThreadCount() const { return loops_.size(); }

private:
    EventLoop* baseLoop_;
    int numThreads_;
    int next_;
    std::vector<EventLoopThread*> threads_;
    std::vector<EventLoop*> loops_;
};

#endif // EVENTLOOPTHREADPOOL_H
