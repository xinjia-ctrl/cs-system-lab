#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Epoll.h"
#include <vector>

class Channel;

class EventLoop {
public:
    EventLoop();
    ~EventLoop() = default;

    void loop();
    void quit() { quit_ = true; }

    void updateChannel(Channel* channel);

private:
    Epoll epoll_;
    bool quit_;
};

#endif // EVENTLOOP_H
