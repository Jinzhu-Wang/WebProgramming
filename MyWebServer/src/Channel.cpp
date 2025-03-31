#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>

Channel::Channel(EventLoop* loop,int fd): loop_(loop), fd_(fd), events_(0), ready_(0), in_epoll_(false){}
Channel::~Channel(){
    if(fd_ != -1){
        close(fd_);
        fd_ = -1;
    }
}

void Channel::HandleEvent(){
    if(ready_ & (EPOLLIN | EPOLLPRI)){
        read_callback_();
    }
    if(ready_ & (EPOLLOUT)){
         write_callback_();
    }
}

void Channel::EnableRead(){
    events_ |= EPOLLIN | EPOLLET;
    loop_->update_channel(this);
}

void Channel::use_ET(){
    events_ |= EPOLLET;
    loop_->update_channel(this);
}

int Channel::fd(){
    return fd_;
}

uint32_t Channel::get_events(){
    return events_;
}

uint32_t Channel::get_ready(){
    return ready_;
}

bool Channel::IsInEpoll(){
    return in_epoll_;
}

void Channel::SetInEpoll(bool inpoll){
    in_epoll_ = inpoll;

}

void Channel::SetReadyEvents(uint32_t ev){
    ready_ = ev;
}

void Channel::set_read_callback(std::function<void()> cb){
    read_callback_ = cb;
}




