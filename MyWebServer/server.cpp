#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include "util.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void handleReadEvent(int sockfd);
int main(int argc, char* argv[]){

    char usage[64];
    snprintf(usage, sizeof(usage), "Usage: %s <port>\n", argv[0]);
    errif(argc != 2, usage);

    Socket *serv_sock = new Socket;
    InetAddress *serv_addr = new InetAddress((argv[1]));

    serv_sock->bind(serv_addr);
    serv_sock->listen();
    serv_sock->set_nonblocking();
    int serv_sockfd = serv_sock->getfd();

    Epoll *ep = new Epoll();
    Channel* serv_channel = new Channel(ep,serv_sockfd);
    serv_channel->enable_reading();

    while(1){
        std::vector<Channel*> active_channels = ep->poll(-1);
        int nfds = active_channels.size();
        
        for(int i =0;i<nfds;i++){
            int chfd = active_channels[i]->get_fd();
            if(chfd==serv_sockfd){ //新客户端链接
                InetAddress *clnt_addr = new InetAddress();
                Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr)); 
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getfd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
                clnt_sock->set_nonblocking();
                ep->addFd(clnt_sock->getfd(), EPOLLIN | EPOLLET);
                
            } else if(active_channels[i]->get_revents() & EPOLLIN){ //检查 EPOLLIN 位是否为 1无论其他位如何。更灵活，适合多事件组合。
                handleReadEvent(chfd);
            
            } else{
                printf("something else happened\n");
            }

        }

    }
    delete serv_sock;
    delete serv_addr;
    return 0;

}
void handleReadEvent(int sockfd){
    char buf[READ_BUFFER];
    while(1){
        int str_len = read(sockfd,buf,READ_BUFFER-1);
        if(str_len > 0){
            buf[str_len] = '\0';
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd,buf,str_len);
        } else if(str_len==-1 && errno==EINTR){ //客户端正常中断，继续读取
            printf("continue reading");
            continue;
        } else if(str_len==-1 && ((errno==EAGAIN) || (errno == EWOULDBLOCK))){ //非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        } else{ //EOF，客户端断开连接 str_len==0
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd); //关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}