#ifndef SERVER_H
#define SERVER_H
#include <map>

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class Server{
private:
    EventLoop* loop_;
    Acceptor* acceptor_;
    std::map<int, Connection*> connections;

public:
    Server(EventLoop* loop,char*);
    ~Server();
    
    void handle_read_event(int);
    void new_connection(Socket* serv_sock);
    void delete_connection(Socket *sock);
};


#endif