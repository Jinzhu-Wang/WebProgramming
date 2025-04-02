#include "TcpServer.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "ThreadPool.h"
#include "CurrentThread.h"
#include "common.h"
#include <unistd.h>
#include <iostream>
// #include <memory>
#include <assert.h>


TcpServer::TcpServer(EventLoop* loop, const char* ip, const int port): main_reactor_(loop), next_conn_id_(1){
    // main_reactor_ = std::make_unique<EventLoop>();
    acceptor_ = std::make_unique<Acceptor>(main_reactor_.get(),ip,port);
    std::function<void(int)> cb = std::bind(&TcpServer::HandleNewConnection,this,std::placeholders::_1) ;
    acceptor_->set_new_connection_callback(cb);

    int size = std::thread::hardware_concurrency();
    thread_pool_ = std::make_unique<ThreadPool>(size);
    for(int i =0;i<size;++i){
        std::unique_ptr<EventLoop> sub_reactor = std::make_unique<EventLoop>();
        sub_reactors_.emplace_back(std::move(sub_reactor));
    }
}

void TcpServer::Start(){
    for(size_t i =0;i<sub_reactors_.size();++i){
        std::function<void()> sub_loop = std::bind(&EventLoop::Loop,sub_reactors_[i].get()) ;
        thread_pool_->Add(std::move(sub_loop));
    }
    main_reactor_->Loop();
}

inline void TcpServer::HandleNewConnection(int fd){
    if(fd!=-1){
        std::cout<<"New Connection fd:" << fd << std::endl;
        int random = fd % sub_reactors_.size();

        std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(sub_reactors_[random].get(),fd,next_conn_id_);
        std::function<void(const std::shared_ptr<TcpConnection>&)> cb = std::bind(&TcpServer::HandleClose, this, std::placeholders::_1);
        conn->set_connection_callback(on_connect_);

        // 将connection分配给Channel的tie,增加计数
        conn->set_close_callback(cb);
        conn->set_message_callback(on_message_);
        connections_map_[fd] = conn;
       
        //分配id
        ++next_conn_id_;
        if(next_conn_id_==1000){
            next_conn_id_=1;
        }
        //开始监听读事件
        conn->ConnectionEstablished();
    }
}

inline void TcpServer::HandleClose(const std::shared_ptr<TcpConnection> & conn){
    std::cout <<  CurrentThread::tid() << " TcpServer::HandleClose"  << std::endl;
    main_reactor_->RunOneFunc(std::bind(&TcpServer::HandleCloseInLoop, this, conn));
}

inline void TcpServer::HandleCloseInLoop(const std::shared_ptr<TcpConnection> & conn){
    std::cout << CurrentThread::tid()  << " TcpServer::HandleCloseInLoop - Remove connection id: " <<  conn->id() << " and fd: " << conn->fd() << std::endl;
    auto it = connections_map_.find(conn->fd());
    assert(it != connections_map_.end());
    connections_map_.erase(it);
    //在子线程已经移除了channel，那么主线程只需要移除已关闭的连接即可。
    // EventLoop *loop = conn->loop();
    // loop->QueueOneFunc(std::bind(&TcpConnection::ConnectionDestructor, conn));
}

void TcpServer::set_connection_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn){
    on_connect_ = std::move(fn);
}

void TcpServer::set_message_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn){
    on_message_ = std::move(fn);
}

TcpServer::~TcpServer(){ }

