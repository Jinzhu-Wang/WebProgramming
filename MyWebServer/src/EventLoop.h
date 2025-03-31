#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
class Epoller;
class Channel;
class ThreadPool;
class EventLoop{
private:
    Epoller* ep_;
    bool quit_;

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void update_channel(Channel*);

};

#endif