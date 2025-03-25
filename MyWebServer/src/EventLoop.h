#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
class Epoll;
class Channel;
class ThreadPool;
class EventLoop{
private:
    Epoll* ep_;
    ThreadPool* thread_pool_;
    bool quit_;

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void update_channel(Channel*);
    void add_thread(std::function<void()>);

};

#endif