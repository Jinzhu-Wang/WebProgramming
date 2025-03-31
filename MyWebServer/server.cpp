#include "src/Server.h"
#include <iostream>
#include "src/Buffer.h"
#include "src/Connection.h"
#include "src/EventLoop.h"
#include "src/Socket.h"
#include "src/util.h"

int main(int argc,char*argv[]) {
    errif(argc!=2,"argc error");
    EventLoop *loop = new EventLoop();
    TcpServer *server = new TcpServer(loop,argv[1]);
    server->OnConnect([](TcpConnection* conn){
        conn->Read();
        if(conn->GetState()==ConnectionState::Closed){
            conn->Close();
            return;
        }
        std::cout<<"Message from client"<<conn->GetSocket()->getfd()<<":"<<conn->ReadBuffer()<<std::endl;
        conn->SetSendBuffer(conn->ReadBuffer());
        conn->Write();
    });


    loop->loop();
    return 0;
}