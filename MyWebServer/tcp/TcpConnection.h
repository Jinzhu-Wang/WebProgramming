#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "common.h"
#include <functional>
#include <memory>
#include <string>


class Buffer;

enum class ConnectionState {
    Invalid = 1,
    Connected,
    Disconnected
};

class TcpConnection{
private:
    //该连接绑定的socket
    int connfd_;
    int connid_;
    //连接状态
    ConnectionState state_;

    EventLoop* loop_;

    std::unique_ptr<Channel> channel_;
    std::unique_ptr<Buffer> read_buf_;
    std::unique_ptr<Buffer> send_buf_;

    std::function<void(int)> on_close_;
    std::function<void(TcpConnection *)> on_message_;

    void ReadNonBlocking();
    void WriteNonBlocking();

public:
    DISALLOW_COPY_AND_MOVE(TcpConnection);
    TcpConnection(EventLoop* loop, int connfd, int connid);
    ~TcpConnection();

    void set_close_callback(std::function<void(int)> const &fn); //修饰callback，表示函数不能改变callback本身的值。
    void set_message_callback(std::function<void(TcpConnection*)> const &fn); //修饰callback，表示函数不能改变callback本身的值。

    void set_send_buf(const char* str);
    Buffer* read_buf();
    Buffer* send_buf();

    void Read();
    void Write();
    void Send(const std::string &msg); //输出信息
    void Send(const char* msg, int len);
    void Send(const char* msg);

    void HandleMessage();
    void HandleClose();

    ConnectionState state() const;
    EventLoop* loop() const;
    int fd() const;
    int id() const;    
};


#endif