#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Acceptor.h"
#include <functional>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define READ_BUFFER 1024

Server::Server(EventLoop* loop,char* port): loop_(loop),acceptor_(nullptr){
    acceptor_ = new Acceptor(loop_,port);
    std::function<void(Socket*)> cb = std::bind(&Server::new_connection,this,std::placeholders::_1) ;
    acceptor_->set_new_connection_callback(cb);
}

void Server::handle_read_event(int sockfd){
    char buf[READ_BUFFER];
    while(1){
        int str_len = read(sockfd,buf,READ_BUFFER-1);
        if(str_len > 0){
            buf[str_len] = '\0';
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd,buf,str_len);
        } else if(str_len==-1 && errno==EINTR){ //客户端正常中断，继续读取
            printf("continue reading");
            continue;
        } else if(str_len==-1 && ((errno==EAGAIN) || (errno == EWOULDBLOCK))){ //非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        } else{ //EOF，客户端断开连接 str_len==0
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd); //关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}

void Server::new_connection(Socket* serv_sock){
    InetAddress *clnt_addr = new InetAddress();
    Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr)); 
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getfd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
    clnt_sock->set_nonblocking();
    Channel *clntChannel = new Channel(loop_, clnt_sock->getfd());
    clntChannel->enable_reading();
    std::function<void()> cb = std::bind(&Server::handle_read_event,this, clnt_sock->getfd()) ;
    clntChannel->set_callback(cb);           
}


Server::~Server()
{
    delete acceptor_;
    
}