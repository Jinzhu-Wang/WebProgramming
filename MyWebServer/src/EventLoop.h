#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
class Epoll;
class Channel;
class ThreadPool;
class EventLoop{
private:
    Epoll* ep_;
    bool quit_;

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void update_channel(Channel*);

};

#endif