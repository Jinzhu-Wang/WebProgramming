#include "Epoller.h"
#include "Logging.h"
#include "Channel.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#define MAXEVENTS 1000

Epoller::Epoller():epfd(-1){
    epfd = epoll_create1(0);
    events_ = new epoll_event[MAXEVENTS];
}

std::vector<Channel*> Epoller::Poll(long timeout) const{
    std::vector<Channel*> active_channels;
    int nfds = epoll_wait(epfd, events_, MAXEVENTS,timeout);
    if(nfds == -1){ perror("epoll wait error"); }
    for(int i = 0; i < nfds; ++i){
        Channel *ch = (Channel*)events_[i].data.ptr;
        ch->SetReadyEvents(events_[i].events);
        active_channels.emplace_back(ch);
    }
    return active_channels;
}

void Epoller::UpdateChannel(Channel *channel) const{
    int fd = channel->fd();
    bool inepoll = channel->IsInEpoll();
    struct epoll_event ev{}; //列表初始化,将 ev 的所有成员初始化为零值
    ev.data.ptr = channel; //用ptr=channel代替fd=fd;
    ev.events = channel->listen_events();
    if(!inepoll){
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1){
            LOG_ERROR << "Epoller::UpdateChannel epoll_ctl_add failed";
        }
        channel->SetInEpoll(true);
    } else{
        if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1){
            LOG_ERROR << "epoll_ctl_mod failed for fd#" << fd << ", errno: " << errno 
            << " (" << strerror(errno) << ")";
        }
    }
}

Epoller::~Epoller(){
    if(epfd != -1){
        ::close(epfd);
        epfd = -1;
    }
    delete [] events_;
}

void Epoller::DeleteChannel(Channel *channel) const{
    int fd = channel->fd();
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1){
        LOG_ERROR << "Epoller::UpdateChannel epoll_ctl_del failed";
    }
    channel->SetInEpoll(false);
}