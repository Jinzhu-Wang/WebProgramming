#ifndef SERVER_H
#define SERVER_H
#include <map>
#include <vector>
#include <functional>

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class ThreadPool;

class TcpServer{
private:
    EventLoop* main_reactor_;   
    Acceptor* acceptor_;
    std::map<int, Connection*> connections_;
    std::vector<EventLoop*> sub_reactors_;
    ThreadPool* thread_pool_;
    std::function<void(Connection*)> on_connect_callback_;

public:
    TcpServer(EventLoop* loop,char*);
    ~TcpServer();
    
    void NewConnection(Socket* serv_sock);
    void DeleteConnection(Socket*);
    void OnConnect(std::function<void(Connection*)> fn);
};


#endif