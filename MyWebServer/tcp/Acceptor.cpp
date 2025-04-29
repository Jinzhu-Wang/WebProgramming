#include "Acceptor.h"
#include "Channel.h"
#include "EventLoop.h"

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <assert.h>
#include <cstring>
#include <fcntl.h>
#include <assert.h>
#include <iostream>
#include <netinet/tcp.h>


Acceptor::Acceptor(EventLoop* loop,const char* ip, const int port):loop_(loop),listenfd_(-1){
    Create();
    Bind(ip,port);
    Listen();
    channel_ = std::make_unique<Channel>(listenfd_, loop);
    std::function<void()> cb = std::bind(&Acceptor::AcceptConnection, this);
    channel_->set_read_callback(cb);
    channel_->EnableRead();
}

Acceptor::~Acceptor(){
    loop_->DeleteChannel(channel_.get());
    ::close(listenfd_);
}

void Acceptor::Create(){
    assert(listenfd_ == -1);
    listenfd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(listenfd_ == -1){
        std::cout << "Failed to create socket" <<std::endl;
    }
    // 设置 SO_REUSEADDR 选项
    int opt = 1; // 用于 setsockopt 的值，非零表示启用
    if (setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
        close(listenfd_);
    }
}

void Acceptor::Bind(const char* ip, const int port){
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if(::bind(listenfd_,(const struct sockaddr*)& addr, sizeof(addr))==-1){
        perror("Bind failed");
        std::cout << "Failed to Bind " << ip << ":" << port <<std::endl;
    }
}

void Acceptor::Listen(){
    assert(listenfd_!=-1);
    if(::listen(listenfd_,SOMAXCONN) ==-1){
        std::cout << "Failed to Listen" << std::endl;
    }
}

// 设置TCP连接Keepalive
void Acceptor::SetTcpKeepAlive(int sockfd) {
    int yes = 1;
    // 启用 TCP Keepalive
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) == -1) {
        perror("setsockopt(SO_KEEPALIVE) failed");
    }
    // 设置 TCP Keepalive 空闲时间为 1分钟 (60秒)
    int keep_idle = 60;
    if (setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(keep_idle)) == -1) {
        perror("setsockopt(TCP_KEEPIDLE) failed");
    }
    // 设置 Keepalive 探测间隔为 10秒
    int keep_interval = 10;
    if (setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, &keep_interval, sizeof(keep_interval)) == -1) {
        perror("setsockopt(TCP_KEEPINTVL) failed");
    }
    // 设置 Keepalive 探测的最大尝试次数为 5次
    int keep_count = 5;
    if (setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &keep_count, sizeof(keep_count)) == -1) {
        perror("setsockopt(TCP_KEEPCNT) failed");
    }
}

void Acceptor::AcceptConnection(){
    struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(client_addr);
    assert(listenfd_ != -1);

    int clnt_fd = ::accept4(listenfd_, (struct sockaddr*)& client_addr, &client_addr_length, SOCK_NONBLOCK | SOCK_CLOEXEC);
    
    if (clnt_fd == -1){
        std::cout << "Failed to Accept" << std::endl;
    }
    
    SetTcpKeepAlive(clnt_fd); //设置TCP连接Keepalive
    if(new_connection_callback_){
        new_connection_callback_(clnt_fd);
    }
}

void Acceptor::set_new_connection_callback(std::function<void(int)> const &callback){
    new_connection_callback_ = std::move(callback);
}