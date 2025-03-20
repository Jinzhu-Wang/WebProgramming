#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include <stdio.h>

Acceptor::Acceptor(EventLoop* loop, char* port):loop_(loop){
    serv_sock = new Socket();
    serv_addr = new InetAddress(port);

    serv_sock->bind(serv_addr);
    serv_sock->listen();
    serv_sock->set_nonblocking();

    serv_channel = new Channel(loop_,serv_sock->getfd());
    serv_channel->enable_reading();
    std::function<void()> cb = std::bind(&Acceptor::accept_connection,this);
    serv_channel->set_callback(cb);
}

Acceptor::~Acceptor(){
    delete serv_sock;
    delete serv_channel;
    delete serv_addr;
}

void Acceptor::accept_connection(){
    InetAddress *clnt_addr = new InetAddress();
    Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr)); 
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getfd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
    clnt_sock->set_nonblocking();
    new_connection_callback_(serv_sock);
    delete clnt_addr;
}

void Acceptor::set_new_connection_callback(std::function<void(Socket*)> cb){
    new_connection_callback_ = cb;
}