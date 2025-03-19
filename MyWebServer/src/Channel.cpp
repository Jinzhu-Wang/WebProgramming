#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop* loop,int fd): loop_(loop), fd_(fd), events_(0), revents_(0), inepoll_(false){}
Channel::~Channel(){}

void Channel::enable_reading(){
    events_ = EPOLLIN | EPOLLET;
    loop_->update_channel(this);
}

int Channel::get_fd(){
    return fd_;
}

uint32_t Channel::get_events(){
    return events_;
}

uint32_t Channel::get_revents(){
    return revents_;
}

bool Channel::get_inepoll(){
    return inepoll_;
}

void Channel::set_inepoll(){
    inepoll_ = true;

}

void Channel::set_revents(uint32_t ev){
    revents_ = ev;
}

void Channel::handle_event(){
    callback();
}

void Channel::set_callback(std::function<void()> cb){
    callback = cb;
}

