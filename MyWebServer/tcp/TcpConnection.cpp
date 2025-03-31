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

TcpConnection::TcpConnection(EventLoop* loop, int connfd, int connid):loop_(loop), connfd_(connfd), connid_(connid){
    if(loop_!=nullptr){
        channel_ = std::make_unique<Channel>(connfd,loop);
        channel_->EnableET();
        channel_->set_read_callback(std::bind(&TcpConnection::HandleMessage,this));
        channel_->EnableRead();
    }
    read_buf_ = std::make_unique<Buffer>();
    send_buf_ = std::make_unique<Buffer>();
    state_ = ConnectionState::Connected;
}

TcpConnection::~TcpConnection(){
    ::close(connfd_);
}

void TcpConnection::set_close_callback(std::function<void(int)> const &callback){
    on_close_ = std::move(callback);
}

void TcpConnection::set_message_callback(std::function<void(TcpConnection*)> const &callback){
    on_message_ = std::move(callback);
}

void TcpConnection::HandleMessage(){
    Read();
    if(on_message_){ on_message_(this);}
}

void TcpConnection::HandleClose(){
    if(state_!= ConnectionState::Disconnected){
        state_ = ConnectionState::Disconnected;
        if(on_close_){ on_close_(connfd_);}
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
    WriteNonBlocking();
    send_buf_->Clear();
}

void TcpConnection::ReadNonBlocking(){
    char buf[1024];
    while(1){ // 使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        int str_len = read(connfd_,buf,sizeof(buf)-1);
        if(str_len > 0){
            buf[str_len] = '\0';
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
    const char* buf = send_buf_->c_str();
    int data_size = send_buf_->Size();
    int data_left = data_size;

    while(data_left >0){
        ssize_t bytes_write = write(connfd_, buf+data_size-data_left, data_left);
        if(bytes_write == -1 && errno == EINTR){
            printf("continue writing\n");
            continue;;
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


