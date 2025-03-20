#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Server.h"

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
    new_connection_callback_(serv_sock);
}

void Acceptor::set_new_connection_callback(std::function<void(Socket*)> cb){
    new_connection_callback_ = cb;
}