#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Acceptor.h"
#include "Connection.h"
#include <functional>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define READ_BUFFER 1024

Server::Server(EventLoop* loop,char* port): loop_(loop),acceptor_(nullptr){
    acceptor_ = new Acceptor(loop_,port);
    std::function<void(Socket*)> cb = std::bind(&Server::new_connection,this,std::placeholders::_1) ;
    acceptor_->set_new_connection_callback(cb);
}



void Server::new_connection(Socket* clnt_sock){
    Connection* conn = new Connection(loop_, clnt_sock);
    std::function<void(Socket*)> cb = std::bind(&Server::delete_connection, this, std::placeholders::_1);
    conn->set_delete_connection_callback(cb);
    connections[clnt_sock->getfd()] = conn;
}

void Server::delete_connection(Socket * sock){
    Connection *conn = connections[sock->getfd()];
    connections.erase(sock->getfd()); //从容器中移除键 sock->getfd() 对应的键值对。
    delete conn; //释放 conn 指向的动态分配的 Connection 对象内存。
}


Server::~Server()
{
    delete acceptor_;
    
}


// void Server::handle_read_event(int sockfd){
//     char buf[READ_BUFFER];
//     while(1){
//         int str_len = read(sockfd,buf,READ_BUFFER-1);
//         if(str_len > 0){
//             buf[str_len] = '\0';
//             printf("message from client fd %d: %s\n", sockfd, buf);
//             write(sockfd,buf,str_len);
//         } else if(str_len==-1 && errno==EINTR){ //客户端正常中断，继续读取
//             printf("continue reading");
//             continue;
//         } else if(str_len==-1 && ((errno==EAGAIN) || (errno == EWOULDBLOCK))){ //非阻塞IO，这个条件表示数据全部读取完毕
//             printf("finish reading once, errno: %d\n", errno);
//             break;
//         } else{ //EOF，客户端断开连接 str_len==0
//             printf("EOF, client fd %d disconnected\n", sockfd);
//             close(sockfd); //关闭socket会自动将文件描述符从epoll树上移除
//             break;
//         }
//     }
// }