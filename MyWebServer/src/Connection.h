#ifndef CONNECTION_H
#define CONNECTION_H
#include <functional>

class EventLoop;
class Socket;
class Channel;
class Connection{
private:
    EventLoop* loop_;
    Socket* sock_;
    Channel* channel_;
    std::function<void(Socket*)> delete_connection_callback;

public:
    Connection(EventLoop* loop, Socket* sock);
    ~Connection();

    void echo(int sockfd);
    void set_delete_connection_callback(std::function<void(Socket*)>);
};


#endif