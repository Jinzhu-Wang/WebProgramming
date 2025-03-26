#ifndef SERVER_H
#define SERVER_H
#include <map>
#include <vector>

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class ThreadPool;

class Server{
private:
    EventLoop* main_reactor_;   
    Acceptor* acceptor_;
    std::map<int, Connection*> connections_;
    std::vector<EventLoop*> sub_reactors_;
    ThreadPool* thpool_;

public:
    Server(EventLoop* loop,char*);
    ~Server();
    
    void handle_read_event(int);
    void new_connection(Socket* serv_sock);
    void delete_connection(int);
};


#endif