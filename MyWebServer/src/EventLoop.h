#ifndef EVENTLOOP_H
#define EVENTLOOP_H

class Epoll;
class Channel;
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