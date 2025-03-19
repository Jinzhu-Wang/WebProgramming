#ifndef CHANNEL_H
#define CHANNEL_H

#include <sys/epoll.h>

class Epoll;
class Channel{
private:
    Epoll* ep_; 
    int fd_;
    uint32_t events_;
    uint32_t revents_;
    bool inepoll_;

public:
    Channel(Epoll* ep, int fd);
    ~Channel();

    void enable_reading();
    
    int get_fd();
    uint32_t get_events();
    uint32_t get_revents();
    bool get_inepoll();
    void set_inepoll();


    // void setEvents(uint32_t);
    void set_revents(uint32_t); //channel要求epoll监听的事情可能不止一件，epoll通过这个函数返回给主程序具体是什么事发生

};

#endif