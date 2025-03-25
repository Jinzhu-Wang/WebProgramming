#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>

Channel::Channel(EventLoop* loop,int fd): loop_(loop), fd_(fd), events_(0), ready_(0), inepoll_(false), use_thread_pool_(true){}
Channel::~Channel(){
    if(fd_ != -1){
        close(fd_);
        fd_ = -1;
    }
}

void Channel::handle_event(){
    if(ready_ & (EPOLLIN | EPOLLPRI)){
        if(use_thread_pool_){
            loop_->add_thread(read_callback_);
        } else{
            read_callback_();
        }
    }
    if(ready_ & (EPOLLOUT)){
        if(use_thread_pool_){
            loop_->add_thread(write_callback_);
        } else{
            write_callback_();
        }
    }
}

void Channel::enable_reading(){
    events_ |= EPOLLIN | EPOLLET;
    loop_->update_channel(this);
}

void Channel::use_ET(){
    events_ |= EPOLLET;
    loop_->update_channel(this);
}

int Channel::get_fd(){
    return fd_;
}

uint32_t Channel::get_events(){
    return events_;
}

uint32_t Channel::get_ready(){
    return ready_;
}

bool Channel::get_inepoll(){
    return inepoll_;
}

void Channel::set_inepoll(bool inpoll){
    inepoll_ = inpoll;

}

void Channel::set_ready(uint32_t ev){
    ready_ = ev;
}

void Channel::set_read_callback(std::function<void()> cb){
    read_callback_ = cb;
}

void Channel::set_use_thread_pool(bool use){
    use_thread_pool_ = use;
}


