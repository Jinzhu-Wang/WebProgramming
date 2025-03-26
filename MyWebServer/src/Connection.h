#ifndef CONNECTION_H
#define CONNECTION_H
#include <functional>
#include <string>

class EventLoop;
class Socket;
class Channel;
class Buffer;
class Connection{
private:
    EventLoop* loop_;
    Socket* sock_;
    Channel* channel_;
    std::function<void(int)> delete_connection_callback;
    Buffer* read_buffer_;

public:
    Connection(EventLoop* loop, Socket* sock);
    ~Connection();

    void echo(int sockfd);
    void set_delete_connection_callback(std::function<void(int)>);
    void send(int);
};


#endif