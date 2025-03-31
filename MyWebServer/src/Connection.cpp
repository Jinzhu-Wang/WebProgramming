#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include "util.h"
#include "Buffer.h"
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#define READ_BUFFER 1024

Connection::Connection(EventLoop* loop, Socket* sock):loop_(loop), sock_(sock){
    if(loop_!=nullptr){
        channel_ = new Channel(loop_,sock_->getfd()); 
        channel_->EnableRead();
        channel_->use_ET();
    }
    read_buffer_ = new Buffer();
    send_buffer_ = new Buffer();
    state_ = State::Connected;
}

Connection::~Connection(){
    if(loop_!=nullptr){
        delete channel_;
    }
    delete sock_;
    delete read_buffer_;
    delete send_buffer_;
}

void Connection::Read(){
    assert(state_ == State::Connected); //验证是否为connect状态
    read_buffer_->clear();
    ReadNonBlocking();
}

void Connection::Write(){
    assert(state_ == State::Connected);
    WriteNonBlocking();
    send_buffer_->clear();
}

void Connection::ReadNonBlocking(){
    int sockfd = sock_->getfd();
    char buf[READ_BUFFER];
    while(1){ // 使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        int str_len = read(sockfd,buf,READ_BUFFER-1);
        if(str_len > 0){
            buf[str_len] = '\0';
            read_buffer_->Append(buf,str_len);
        } else if(str_len==-1 && errno==EINTR){ //客户端正常中断，继续读取
            printf("continue reading");
            continue;
        } else if(str_len==-1 && ((errno==EAGAIN) || (errno == EWOULDBLOCK))){ //非阻塞IO，这个条件表示数据全部读取完毕
            printf("message from client fd %d: %s\n", sockfd, read_buffer_->c_str());
            break;
        } else if(str_len==0){ //EOF，客户端断开连接 str_len==0
            printf("EOF, client fd %d disconnected\n", sockfd);
            state_ = State::Closed;
            break;
        } else{
            printf("Other error on client fd %d\n", sockfd);
            break;
        }
    }
}

void Connection::WriteNonBlocking(){
    int sockfd = sock_->getfd();
    const char* buf = send_buffer_->c_str();
    int data_size = send_buffer_->size();
    int data_left = data_size;

    while(data_left >0){
        ssize_t bytes_write = write(sockfd, buf+data_size-data_left, data_left);
        if(bytes_write == -1 && errno == EINTR){
            printf("continue writing\n");
            continue;;
        }
        if(bytes_write == -1 &&errno ==EAGAIN){
            break;
        }
        if(bytes_write == -1){
            printf("Other error on client fd %d\n", sockfd);
            state_ = State::Closed;
            break;
        }
        data_left -=bytes_write;
    }
}

void Connection::Close(){
    delete_connection_callback_(sock_);
}

State Connection::GetState(){ return state_;}

void Connection::SetSendBuffer(const char* str){ send_buffer_->set_buf(str);}

Buffer* Connection::GetReadBuffer(){ return read_buffer_;}
const char* Connection::ReadBuffer(){ return read_buffer_->c_str();}

Buffer *Connection::GetSendBuffer() { return send_buffer_; }
const char *Connection::SendBuffer() { return send_buffer_->c_str(); }

void Connection::SetDeleteConnectionCallback(std::function<void(Socket*)> const &callback){
    delete_connection_callback_ = callback;
}

void Connection::SetOnConnectCallback(std::function<void(Connection*)> const &callback){
    on_connect_callback_ = callback;
    channel_->set_read_callback([this](){on_connect_callback_(this);});
}

void Connection::GetlineSendBuffer(){send_buffer_ ->getline();}

Socket* Connection::GetSocket(){return sock_;}