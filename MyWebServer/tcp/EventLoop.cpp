#include "EventLoop.h"
#include "Channel.h"
#include "Epoller.h"
#include <vector>

EventLoop::EventLoop(){
    poller_ = std::make_unique<Epoller>();
}

EventLoop::~EventLoop(){}

void EventLoop::Loop() const{
    while (true){
        std::vector<Channel*> active_channels;
        active_channels = poller_->Poll();
        for(auto it = active_channels.begin(); it!=active_channels.end();++it){
            (*it)->HandleEvent();
        }
    }
}

void EventLoop::UpdateChannel(Channel *ch) const{ poller_->UpdateChannel(ch);}
void EventLoop::DeleteChannel(Channel *ch) const{ poller_->DeleteChannel(ch);}