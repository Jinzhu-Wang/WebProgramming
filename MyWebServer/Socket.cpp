#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>



Socket::Socket() :fd(-1){
    fd = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    errif(fd == -1, "socket create error");
}

Socket::Socket(int fd){
    this->fd=fd;
}

void Socket::bind(const InetAddress* serv_addr){//避免命名冲突，使用::调用系统函数
    errif(::bind(fd, (sockaddr*)&(serv_addr->addr),serv_addr->addr_len)==-1,"socket bind error");
}

void Socket::listen(){
    errif(::listen(fd,SOMAXCONN)==-1,"socket listen error");
}

int Socket::accept(const InetAddress* clnt_addr){
    int clnt_sock = ::accept(fd,(sockaddr*)&clnt_addr->addr,(socklen_t*)&clnt_addr->addr_len);
    errif(clnt_sock==-1 , "accept error");
    return clnt_sock;
}

void Socket::set_nonblocking(){
    fcntl(fd,F_SETFL,(fcntl(fd,F_GETFL)) | O_NONBLOCK);
}


int Socket::getfd(){
    return this->fd;
}

Socket::~Socket(){
    if(fd!=-1){
        close(fd);
        fd =-1;
    }
}

