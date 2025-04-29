#include "TcpConnection.h"
#include "Channel.h"
#include "util.h"
#include "Buffer.h"
#include "common.h"
#include "EventLoop.h"
#include "HttpContext.h"
#include "Logging.h"
#include "CurrentThread.h"

#include <memory>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <sys/sendfile.h>

TcpConnection::TcpConnection(EventLoop* loop, int connfd, int connid):connfd_(connfd), connid_(connid), loop_(loop){
    if(loop_!=nullptr){
        channel_ = std::make_unique<Channel>(connfd,loop);
        channel_->EnableET();
        channel_->set_read_callback(std::bind(&TcpConnection::HandleMessage,this));
        channel_->set_write_callback(std::bind(&TcpConnection::HandleWrite,this));
        //channel_->EnableRead();
    }
    read_buf_ = std::make_unique<Buffer>();
    send_buf_ = std::make_unique<Buffer>();
    context_ = std::make_unique<HttpContext>();
}

TcpConnection::~TcpConnection(){
    LOG_INFO<<"----------------------------------------"<<"connfd_:"<<connfd_<<"remove-----";
    ::close(connfd_);
}

void TcpConnection::ConnectionEstablished(){
    state_ = ConnectionState::Connected;
    channel_->Tie(shared_from_this());
    channel_->EnableRead();
    if(on_connect_){
        on_connect_(shared_from_this());
    }
}

void TcpConnection::ConnectionDestructor(){
    LOG_INFO<< CurrentThread::tid()<< "fd: "<<connfd_ << " TcpConnection::ConnectionDestructor" ;
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

void TcpConnection::HandleWrite(){
    LOG_INFO<<"TcpConnection::HandlWrite";
    WriteNonBlocking();
}

void TcpConnection::HandleClose(){
    if(state_!= ConnectionState::Disconnected){
        state_ = ConnectionState::Disconnected;
        // loop_->DeleteChannel(channel_.get()); // 子线程移除 Channel,避免异步移除的等待
        if(on_close_){ on_close_(shared_from_this());}
    }
}

EventLoop* TcpConnection::loop() const{return loop_;}
int TcpConnection::id() const { return connid_; }
int TcpConnection::fd() const { return connfd_; }
ConnectionState TcpConnection::state() const{ return state_;}
Buffer* TcpConnection::read_buf(){ return read_buf_.get();}
Buffer *TcpConnection::send_buf() { return send_buf_.get(); }

void TcpConnection::Send(const std::string &msg){
    Send(msg.data(),static_cast<int>(msg.size()));
}

void TcpConnection::Send(const  char* msg){
    Send(msg,static_cast<int>(strlen(msg)));
}

void TcpConnection::Send(const char* msg, int len){
    int remaining = len;
    int send_size =  0;

    //如果此时send_buf中没有数据，则可以先尝试发送数据
    if(send_buf_->readablebytes()==0){
        //强制类型转换，方便remaining操作
        send_size = static_cast<int>(write(connfd_,msg,len));
        if(send_size >=0){
            //说明发送了部分数据
            remaining -= send_size;
        } else if((send_size == -1) && ((errno == EAGAIN)||(errno==EWOULDBLOCK))){
            //说明此时TCP缓冲区是满的，没有办法写入，什么都不做
            send_size= 0; //说明实际上没有发送数据
        } else{
            LOG_ERROR <<"TcpConnection::Send - TcpConnection Send ERROR";
            return;
        }
    }
    //将剩余的数据加入到send_buf中，等待后续发送
    assert(remaining <= len);
    if(remaining>0){
        send_buf_->Append(msg+send_size,remaining);
        // 到达这一步时
        // 1. 还没有监听写事件，在此时进行了监听
        // 2. 监听了写事件，并且已经触发了，此时再次监听，强制触发一次，如果强制触发失败，仍然可以等待后续TCP缓冲区可写。
        channel_->EnableWrite();
    }
}

void TcpConnection::Read(){
   if(state_ != ConnectionState::Connected){return;} 
    // assert(state_ == ConnectionState::Connected); //验证是否为connect状态
    read_buf_->RetrieveAll();
    ReadNonBlocking();
}

void TcpConnection::Write(){
    assert(state_ == ConnectionState::Connected);
    WriteNonBlocking();
    // send_buf_->RetrieveAll();
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
    int remaining = send_buf_->readablebytes();
    int send_size = static_cast<int>(write(connfd_, send_buf_->Peek(), remaining));
    if((send_size == -1) && 
                ((errno == EAGAIN) || (errno == EWOULDBLOCK))){
        // 说明此时TCP缓冲区是满的，没有办法写入，什么都不做 
        // 主要是防止，在Send时write后监听EPOLLOUT，但是TCP缓冲区还是满的，
        send_size = 0;
    }
    else if (send_size == -1){
        LOG_ERROR << "TcpConnection::Send - TcpConnection Send ERROR";
    }

    remaining -= send_size;
    send_buf_->Retrieve(send_size);
}

HttpContext *TcpConnection::context() const { return context_.get(); }

TimeStamp TcpConnection::timestamp() const { return timestamp_; }
void TcpConnection::UpdateTimeStamp(TimeStamp now){
    timestamp_ = now;
}

void TcpConnection::SendFile(int filefd, int size){
    ssize_t send_size = 0;
    ssize_t data_size = static_cast<ssize_t>(size);

    while(send_size<data_size){
        ssize_t bytes_write = sendfile(connfd_,filefd,(off_t*)&send_size,data_size-send_size);
        if(bytes_write==-1){
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)){
                continue;
            }else{
                //continue;
                break;
            }
        }
        send_size +=bytes_write;
    }
}

