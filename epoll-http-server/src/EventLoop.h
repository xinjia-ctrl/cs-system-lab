#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Epoll.h"
#include "Channel.h"

#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>

class EventLoop {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void updateChannel(Channel* channel);

    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);

    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

private:
    void wakeup();
    void handleWakeup();
    void doPendingFunctors();

    Epoll epoll_;
    std::atomic<bool> quit_;

    // Cross-thread wakeup
    int wakeupFd_;
    Channel wakeupChannel_;
    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;

    std::thread::id threadId_;
};

#endif // EVENTLOOP_H
