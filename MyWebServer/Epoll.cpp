#include "Epoll.h"
#include "util.h"
#include <unistd.h>

#define MAXEVENTS 1000

Epoll::Epoll():epfd(-1){
    epfd = epoll_create1(0);
    errif(epfd==-1,"epoll create error");
    events = new epoll_event[MAXEVENTS];
}

void Epoll::addFd(int fd, uint32_t op){
    struct epoll_event ev;
    ev.events = op;
    ev.data.fd=fd;
    errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add event error");
}

std::vector<epoll_event> Epoll::poll(int timeout){
    int nfds = epoll_wait(epfd, events, MAXEVENTS,timeout);
    errif(nfds == -1, "epoll wait error");
    return std::vector<epoll_event> (events,events+nfds);
}

Epoll::~Epoll(){
    if(epfd != -1){
        close(epfd);
        epfd = -1;
    }
    delete [] events;
}