#include "Epoll.h"
#include "util.h"
#include "Channel.h"
#include <unistd.h>

#define MAXEVENTS 1000

Epoll::Epoll():epfd(-1){
    epfd = epoll_create1(0);
    errif(epfd==-1,"epoll create error");
    events = new epoll_event[MAXEVENTS];
}

// void Epoll::addFd(int fd, uint32_t op){
//     struct epoll_event ev;
//     ev.events = op;
//     ev.data.fd=fd;
//     errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add event error");
// }

std::vector<Channel*> Epoll::poll(int timeout){
    std::vector<Channel*> activeChannels;
    int nfds = epoll_wait(epfd, events, MAXEVENTS,timeout);
    errif(nfds == -1, "epoll wait error");
    for(int i = 0; i < nfds; ++i){
        Channel *ch = (Channel*)events[i].data.ptr;
        ch->set_revents(events[i].events);
        activeChannels.push_back(ch);
    }
    return activeChannels;
}

void Epoll::update_channel(Channel *channel){
    int fd = channel->get_fd();
    bool inepoll = channel->get_inepoll();
    struct epoll_event ev;
    ev.data.ptr = channel; //用ptr=channel代替fd=fd;
    ev.events = channel->get_events();
    if(!inepoll){
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->set_inepoll();
    } else{
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
    }
}

Epoll::~Epoll(){
    if(epfd != -1){
        close(epfd);
        epfd = -1;
    }
    delete [] events;
}