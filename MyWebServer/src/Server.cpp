#include "Server.h"
#include "Socket.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include "EventLoop.h"
#include <unistd.h>
#include <functional>


Server::Server(EventLoop* loop,char* port): main_reactor_(loop),acceptor_(nullptr){
    acceptor_ = new Acceptor(main_reactor_,port);
    std::function<void(Socket*)> cb = std::bind(&Server::new_connection,this,std::placeholders::_1) ;
    acceptor_->set_new_connection_callback(cb);

    int size = std::thread::hardware_concurrency();
    thpool_ = new ThreadPool(size);
    for(int i =0;i<size;++i){
        sub_reactors_.emplace_back(new EventLoop());
    }

    for(int i = 0; i<size; ++i){
        std::function<void()> sub_loop = std::bind(&EventLoop::loop, sub_reactors_[i]);
        thpool_->add(sub_loop);
    }
}

void Server::new_connection(Socket* clnt_sock){
    if(clnt_sock->getfd()!=-1){
        int random = clnt_sock->getfd() % sub_reactors_.size();
        Connection* conn = new Connection(sub_reactors_[random], clnt_sock);
        std::function<void(int)> cb = std::bind(&Server::delete_connection, this, std::placeholders::_1);
        conn->set_delete_connection_callback(cb);
        connections_[clnt_sock->getfd()] = conn;
    }
}

void Server::delete_connection(int sockfd){
    if(sockfd!=-1){
        auto it =connections_.find(sockfd);
        if(it != connections_.end()){
            Connection *conn = connections_[sockfd];
            connections_.erase(sockfd);
            delete conn;
        }
    }
}


Server::~Server()
{
    delete acceptor_;
    delete thpool_;    
}

