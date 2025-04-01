#include "TcpConnection.h"
#include "Channel.h"
#include "util.h"
#include "Buffer.h"
#include "common.h"
#include "EventLoop.h"

#include <memory>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <sys/socket.h>
#include <string.h>

TcpConnection::TcpConnection(EventLoop* loop, int connfd, int connid):loop_(loop), connfd_(connfd), connid_(connid){
    if(loop_!=nullptr){
        channel_ = std::make_unique<Channel>(connfd,loop);
        channel_->EnableET();
        channel_->set_read_callback(std::bind(&TcpConnection::HandleMessage,this));
        channel_->EnableRead();
    }
    read_buf_ = std::make_unique<Buffer>();
    send_buf_ = std::make_unique<Buffer>();
}

TcpConnection::~TcpConnection(){
    ::close(connfd_);
}

void TcpConnection::ConnectionEstablished(){
    state_ = ConnectionState::Connected;
    channel_->Tie(shared_from_this());
    // channel_->EnableRead();
    loop_->UpdateChannel(channel_.get());
    if(on_connect_){
        on_connect_(shared_from_this());
    }
}

void TcpConnection::ConnectionDestructor(){
    //std::cout << CurrentThread::tid() << " TcpConnection::ConnectionDestructor" << std::endl;
    // 将该操作从析构处，移植该处，增加性能，因为在析构前，当前`TcpConnection`已经相当于关闭了。
    // 已经可以将其从loop处离开。
    loop_->DeleteChannel(channel_.get());
}

void TcpConnection::set_connection_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &callback){
    on_connect_ = std::move(callback);
}

void TcpConnection::set_close_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &callback){
    on_close_ = std::move(callback);
}

void TcpConnection::set_message_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &callback){
    on_message_ = std::move(callback);
}

void TcpConnection::HandleMessage(){
    Read();
    if(on_message_){ on_message_(shared_from_this());}// 只有缓冲区有数据时才调用回调
}

void TcpConnection::HandleClose(){
    if(state_!= ConnectionState::Disconnected){
        state_ = ConnectionState::Disconnected;
        if(on_close_){ on_close_(shared_from_this());}
    }
}

EventLoop* TcpConnection::loop() const{return loop_;}
int TcpConnection::id() const { return connid_; }
int TcpConnection::fd() const { return connfd_; }
ConnectionState TcpConnection::state() const{ return state_;}
void TcpConnection::set_send_buf(const char* str){ send_buf_->set_buf(str);}
Buffer* TcpConnection::read_buf(){ return read_buf_.get();}
Buffer *TcpConnection::send_buf() { return send_buf_.get(); }

void TcpConnection::Send(const std::string &msg){
    set_send_buf(msg.c_str());
    Write();
}

void TcpConnection::Send(const  char* msg){
    set_send_buf(msg);
    Write();
}

void TcpConnection::Read(){
    assert(state_ == ConnectionState::Connected); //验证是否为connect状态
    read_buf_->Clear();
    ReadNonBlocking();
}

void TcpConnection::Write(){
    assert(state_ == ConnectionState::Connected);
    WriteNonBlocking();
    send_buf_->Clear();
}

void TcpConnection::ReadNonBlocking(){
    char buf[1024];
    while(1){ // 使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        memset(buf, 0, sizeof(buf));
        int str_len = read(connfd_,buf,sizeof(buf)-1);
        if(str_len > 0){
            read_buf_->Append(buf,str_len);
        } else if(str_len==-1 && errno==EINTR){ //客户端正常中断，继续读取
            printf("continue reading");
            continue;
        } else if(str_len==-1 && ((errno==EAGAIN) || (errno == EWOULDBLOCK))){ //非阻塞IO，这个条件表示数据全部读取完毕
            // printf("message from client fd %d: %s\n", connfd_, read_buf_->c_str());
            break;
        } else if(str_len==0){ //EOF，客户端断开连接 str_len==0
            printf("EOF, client fd %d disconnected\n", connfd_);
            HandleClose();
            break;
        } else{
            printf("Other error on client fd %d\n", connfd_);
            HandleClose();
            break;
        }
    }
}

void TcpConnection::WriteNonBlocking(){
    char buf[send_buf_->Size()];
    memcpy(buf, send_buf_->c_str(), send_buf_->Size());
    int data_size = send_buf_->Size();
    int data_left = data_size;

    while(data_left >0){
        ssize_t bytes_write = write(connfd_, buf+data_size-data_left, data_left);
        if(bytes_write == -1 && errno == EINTR){
            printf("continue writing\n");
            continue;
        }
        if(bytes_write == -1 &&errno ==EAGAIN){
            break;
        }
        if(bytes_write == -1){
            printf("Other error on client fd %d\n", connfd_);
            HandleClose();
            break;
        }
        data_left -=bytes_write;
    }
}


