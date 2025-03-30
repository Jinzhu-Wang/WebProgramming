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
    bool inepoll_;
    std::function<void()> read_callback_;
    std::function<void()> write_callback_;

public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

    void enable_reading();
    void handle_event();
    
    int get_fd();
    uint32_t get_events();
    uint32_t get_ready();
    bool get_inepoll();
    void set_inepoll(bool );
    void use_ET();
    
    void set_ready(uint32_t);
    // void set_revents(uint32_t); //channel要求epoll监听的事情可能不止一件，epoll通过这个函数返回给主程序具体是什么事发生
    void set_read_callback(std::function<void()>);
    // void set_use_thread_pool(bool use = true);

};

#endif