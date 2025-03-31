#ifndef EPOLLER_H
#define EPOLLER_H
#include "common.h"

#include <sys/epoll.h>
#include <vector>

class Channel;
class Epoller{
private:
    int epfd;
    struct epoll_event *events_; //用来记录每次发生的事件
public:
    DISALLOW_COPY_AND_MOVE(Epoller);
    Epoller();
    ~Epoller();

    //更新监听的channel
    void UpdateChannel(Channel *) const;
    //删除监听的channel
    void DeleteChannel(Channel *channel) const;

    //返回调用完epoll_wait的channel
    std::vector<Channel*> Poll(long timeout = -1) const;

};


#endif