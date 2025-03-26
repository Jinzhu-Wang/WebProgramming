#include "Socket.h"
#include "util.h"
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#include <string.h>
#include <stdlib.h>

Socket::Socket() :fd(-1){
    fd = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    errif(fd == -1, "socket create error");
}

Socket::Socket(int fd){
    this->fd=fd;
}

void Socket::bind(InetAddress* serv_addr){//避免命名冲突，使用::调用系统函数
    struct sockaddr_in addr = serv_addr->get_addr();
    errif(::bind(fd, (sockaddr*)&(addr),serv_addr->addr_len)==-1,"socket bind error");
}

void Socket::listen(){
    errif(::listen(fd,SOMAXCONN)==-1,"socket listen error");
}

int Socket::accept(InetAddress* clnt_addr){
    int clnt_sockfd = -1;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    if(fcntl(fd,F_GETFL) & O_NONBLOCK){//处理非阻塞模式，进入循环继续尝试，直到成功接受连接或发生其他错误。
        while(true){
            clnt_sockfd = ::accept(fd,(sockaddr*)&addr, &addr_len);
            if(clnt_sockfd == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){
                // printf("no connection yet\n");
                continue;
            } else if(clnt_sockfd == -1){
                errif(true, "socket accept error");
            } else{
                break;
            }
        }
    } else{ //阻塞模式，会一直等待直到有新连接。
        clnt_sockfd = ::accept(fd,(sockaddr*)&addr,(socklen_t*)&clnt_addr->addr_len);
        errif(clnt_sockfd==-1 , "accept error");
    }
    clnt_addr->set_inetaddr(addr);
    return clnt_sockfd;
}

void Socket::set_nonblocking(){
    fcntl(fd,F_SETFL,(fcntl(fd,F_GETFL)) | O_NONBLOCK);
}


int Socket::getfd(){
    return this->fd;
}

void Socket::connect(InetAddress * addr_in){
    struct sockaddr_in addr = addr_in->get_addr();
    if(fcntl(fd,F_GETFL) & O_NONBLOCK){
        while(true){
            int ret = ::connect(fd, (sockaddr*)&addr, sizeof(addr));
            if(ret == 0){
                break;
            } else if(ret == -1 && (errno == EINPROGRESS)){ //这里的处理不对，为了简单让他不断循环，相当于阻塞式。
                continue;
            } else if(ret==-1){
                errif(true,"sock connect error");
            }
        }
    }else{
        errif(::connect(fd, (sockaddr*)&addr, sizeof(addr)) == -1, "socket connect error");
    }
}


Socket::~Socket(){
    if(fd!=-1){
        close(fd);
        fd =-1;
    }
}


//初始化列表是“一次性构造”，效率更高。
//函数体内是“先构造再修改”，多一步操作
InetAddress::InetAddress(): addr_len(sizeof(addr)){
    memset(&addr,0,addr_len);
}

InetAddress::InetAddress(const char* port): addr_len(sizeof(addr)){
    memset(&addr,0,addr_len);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    addr.sin_port = htons(atoi(port));
}

InetAddress::InetAddress(const char* ip, const char* port): addr_len(sizeof(addr)){
    memset(&addr,0,addr_len);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip); //将点分十进制的 IPv4 地址字符串（例如 "127.0.0.1"）转换为 32 位网络字节序的整数
    addr.sin_port = htons(atoi(port));
}

void InetAddress::set_inetaddr(sockaddr_in _addr){
    addr = _addr;
}

sockaddr_in InetAddress::get_addr() {
    return addr;
}

char* InetAddress::get_ip(){
    return inet_ntoa(addr.sin_addr);
}
uint16_t InetAddress::get_port(){
    return ntohs(addr.sin_port);
}

InetAddress::~InetAddress(){
    
}