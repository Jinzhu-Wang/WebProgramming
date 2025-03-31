#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "common.h"
#include <memory> //智能指针


class Epoller;
class EventLoop{
private:
    std::unique_ptr<Epoller> poller_;

public:
    DISALLOW_COPY_AND_MOVE(EventLoop);
    EventLoop();
    ~EventLoop();

    void Loop() const;
    void UpdateChannel(Channel*) const;
    void DeleteChannel(Channel*) const;

};

#endif