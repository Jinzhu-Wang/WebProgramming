#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include "util.h"
#include "Buffer.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#define READ_BUFFER 1024

Connection::Connection(EventLoop* loop, Socket* sock):loop_(loop), sock_(sock),channel_(nullptr), in_buffer_(new std::string()), read_buffer_(nullptr){
    channel_ = new Channel(loop_,sock_->getfd());
    channel_->enable_reading();

    std::function<void()> cb = std::bind(&Connection::echo,this,sock_->getfd());
    channel_->set_callback(cb);
    read_buffer_ = new Buffer();
}

Connection::~Connection(){
    delete channel_;
    delete sock_;
}

void Connection::echo(int sockfd){
    char buf[READ_BUFFER];
    while(1){
        int str_len = read(sockfd,buf,READ_BUFFER-1);
        if(str_len > 0){
            buf[str_len] = '\0';
            read_buffer_->append(buf,str_len);
        } else if(str_len==-1 && errno==EINTR){ //客户端正常中断，继续读取
            printf("continue reading");
            continue;
        } else if(str_len==-1 && ((errno==EAGAIN) || (errno == EWOULDBLOCK))){ //非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            printf("message from client fd %d: %s\n", sockfd, read_buffer_->c_str());
            errif(write(sockfd, read_buffer_->c_str(), read_buffer_->size()) == -1, "socket write error");
            read_buffer_->clear();
            break;
        } else{ //EOF，客户端断开连接 str_len==0
            printf("EOF, client fd %d disconnected\n", sockfd);
            //close(sockfd); //关闭socket会自动将文件描述符从epoll树上移除
            delete_connection_callback(sock_);
            break;
        }
    }

}

void Connection::set_delete_connection_callback(std::function<void(Socket*)> cb){
    delete_connection_callback = cb;
}