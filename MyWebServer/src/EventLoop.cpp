#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <vector>

EventLoop::EventLoop():ep_(nullptr), quit_(false){
    ep_ = new Epoll();
}

EventLoop::~EventLoop(){
    delete ep_;
}

void EventLoop::loop(){
    while (!quit_){
        std::vector<Channel*> active_channels;
        active_channels = ep_->poll();
        for(auto it = active_channels.begin(); it!=active_channels.end();++it){
            (*it)->handle_event();
        }
    }
}

void EventLoop::update_channel(Channel *ch){
    ep_->update_channel(ch);
}