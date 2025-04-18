#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "common.h"
#include "TimeStamp.h"
#include <functional>
#include <memory>
#include <string>

class HttpContext;
class Buffer;
class TimeStamp;

enum class ConnectionState {
    Invalid = 1,
    Connected,
    Disconnected
};

class TcpConnection : public std::enable_shared_from_this<TcpConnection>{
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

    std::function<void(const std::shared_ptr<TcpConnection> &)> on_close_;
    std::function<void(const std::shared_ptr<TcpConnection> &)> on_message_;
    std::function<void(const std::shared_ptr<TcpConnection> &)> on_connect_;

    void ReadNonBlocking();
    void WriteNonBlocking();

    std::unique_ptr<HttpContext> context_;

    // 需要频繁赋值，使用普通成员变量。
    TimeStamp timestamp_; //前向声明只允许使用指针或引用这是一个对象实例

public:
    DISALLOW_COPY_AND_MOVE(TcpConnection);
    TcpConnection(EventLoop* loop, int connfd, int connid);
    ~TcpConnection();

    // 初始化TcpConneection
    void ConnectionEstablished();
    // 销毁TcpConnection
    void ConnectionDestructor();
    // 建立连接时调用回调函数
    void set_connection_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn);
    // 关闭时的回调函数
    void set_close_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn); //修饰callback，表示函数不能改变callback本身的值。
    // 接受到信息的回调函数    
    void set_message_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn); //修饰callback，表示函数不能改变callback本身的值。


    Buffer* read_buf();
    Buffer* send_buf();

    void Read();
    void Write();
    void Send(const std::string &msg); //输出信息
    void Send(const char* msg, int len);
    void Send(const char* msg);

    void HandleMessage(); // 当接收到信息时，进行回调
    void HandleWrite();
    void HandleClose(); // 当TcpConnection发起关闭请求时，进行回调，释放相应的socket.

    ConnectionState state() const;
    EventLoop* loop() const;
    int fd() const;
    int id() const;    
    HttpContext *context() const;

    TimeStamp timestamp() const;
    void UpdateTimeStamp(TimeStamp now);
};


#endif