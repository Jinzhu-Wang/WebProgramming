#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "common.h"

#include <memory>
#include <functional>

class EventLoop;
class Channel;

class Acceptor
{
private:
    EventLoop* loop_;
    int listenfd_;
    std::unique_ptr<Channel>channel_;
    std::function<void(int)> new_connection_callback_ ;
        
public:
    DISALLOW_COPY_AND_MOVE(Acceptor);
    Acceptor(EventLoop*,const char* ip, const int port);
    ~Acceptor();

    void set_new_connection_callback(std::function<void(int)> const &callback); 

    //创建socket
    void Create();

    //绑定IP地址
    void Bind(const char* ip,const int port);

    //监听socket
    void Listen();
    
    //接收链接
    void AcceptConnection();

    //设置TCP连接Keepalive
    void SetTcpKeepAlive(int sockfd);

};



#endif