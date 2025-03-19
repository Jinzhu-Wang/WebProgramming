#ifndef CHANNEL_H
#define CHANNEL_H
#include <functional>
#include <sys/epoll.h>

class EventLoop;
class Channel{
private:
    EventLoop* loop_; 
    int fd_;
    uint32_t events_;
    uint32_t revents_;
    bool inepoll_;
    std::function<void()> callback;

public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

    void enable_reading();
    void handle_event();
    
    int get_fd();
    uint32_t get_events();
    uint32_t get_revents();
    bool get_inepoll();
    void set_inepoll();
    
    // void setEvents(uint32_t);
    void set_revents(uint32_t); //channel要求epoll监听的事情可能不止一件，epoll通过这个函数返回给主程序具体是什么事发生
    void set_callback(std::function<void()>);

};

#endif