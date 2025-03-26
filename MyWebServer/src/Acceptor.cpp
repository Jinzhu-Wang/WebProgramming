#include "Acceptor.h"
#include "Socket.h"

#include "Channel.h"
#include <stdio.h>

Acceptor::Acceptor(EventLoop* loop, char* port):loop_(loop){
    serv_sock = new Socket();
    serv_addr = new InetAddress(port);

    serv_sock->bind(serv_addr);
    serv_sock->listen();
    // serv_sock->set_nonblocking();

    serv_channel = new Channel(loop_,serv_sock->getfd());
    std::function<void()> cb = std::bind(&Acceptor::accept_connection,this);
    serv_channel->set_read_callback(cb);
    serv_channel->enable_reading();
    delete serv_addr;
}

Acceptor::~Acceptor(){
    delete serv_sock;
    delete serv_channel;
    
}

void Acceptor::accept_connection(){
    InetAddress *clnt_addr = new InetAddress();
    Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));  //这里是服务端用于处理客户端连接的，不是真的客户端
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getfd(), inet_ntoa(clnt_addr->get_addr().sin_addr), ntohs(clnt_addr->get_addr().sin_port));
    clnt_sock->set_nonblocking();
    new_connection_callback_(clnt_sock);
    delete clnt_addr;
}

void Acceptor::set_new_connection_callback(std::function<void(Socket*)> cb){
    new_connection_callback_ = cb;
}