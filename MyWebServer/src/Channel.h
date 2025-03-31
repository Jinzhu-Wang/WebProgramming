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
    uint32_t ready_;
    bool in_epoll_;
    std::function<void()> read_callback_;
    std::function<void()> write_callback_;

public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

    void EnableRead();
    void HandleEvent();
    
    int fd();
    uint32_t get_events();
    uint32_t get_ready();
    bool IsInEpoll();
    void SetInEpoll(bool );
    void use_ET();
    
    void SetReadyEvents(uint32_t);
    // void set_revents(uint32_t); //channel要求epoll监听的事情可能不止一件，epoll通过这个函数返回给主程序具体是什么事发生
    void set_read_callback(std::function<void()>);
    // void set_use_thread_pool(bool use = true);

};

#endif