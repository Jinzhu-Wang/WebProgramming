#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include <functional>

class EventLoop;
class Socket;
class InetAddress;
class Channel;
class Acceptor
{
private:
    EventLoop* loop_;
    Socket* serv_sock;
    InetAddress* serv_addr;
    Channel* serv_channel;
    std::function<void(Socket*)> new_connection_callback_ ;
        
public:
    Acceptor(EventLoop*, char* port);
    ~Acceptor();

    void accept_connection();
    void set_new_connection_callback(std::function<void(Socket*)>);

};



#endif