#ifndef SERVER_H
#define SERVER_H


class EventLoop;
class Acceptor;
class Socket;
class Server{
private:
    EventLoop* loop_;
    Acceptor* acceptor_;

public:
    Server(EventLoop* loop,char*);
    ~Server();
    
    void handle_read_event(int);
    void new_connection(Socket* serv_sock);

};


#endif