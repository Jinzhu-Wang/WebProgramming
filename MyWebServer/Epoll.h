#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>
#include <vector>

class Epoll{
private:
    int epfd;
    struct epoll_event *events; //用来记录每次发生的事件
public:
    Epoll();
    ~Epoll();

    void addFd(int fd, uint32_t op);
    std::vector<epoll_event> poll(int timeout = -1);
};


#endif