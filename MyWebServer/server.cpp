#include <stdio.h>
#include <stdlib.h> //atoi()
#include <sys/socket.h> //memset()
#include <string.h>
#include <arpa/inet.h> // inet_addr()将IPv4 地址的字符串形式转换为 32 位网络字节序整数。字节序转换函数 htons htonl
#include <fcntl.h> // 设置非阻塞模式（O_NONBLOCK）
#include <unistd.h> // 提供 close、read 等基础系统调用
#include <errno.h> //处理错误码（如 EINTR、EAGAIN）
#include <sys/epoll.h>
#include "util.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void set_nonblocking(int fd){
    /*设置套接字非阻塞模式*/
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK); 
}


int main(int argc, char* argv[]){

    char usage[64];
    snprintf(usage, sizeof(usage), "Usage: %s <port>\n", argv[0]);
    errif(argc != 2, usage);

    int serv_sock,clnt_sock;
    struct sockaddr_in serv_addr,clnt_addr;


    serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    set_nonblocking(serv_sock);
    int opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    errif(bind(serv_sock,(const struct sockaddr*) &serv_addr,sizeof(serv_addr))==-1,"bind error!");

    errif(listen(serv_sock, SOMAXCONN)==-1,"listen error!");

    int epfd = epoll_create1(0);
    errif(epfd == -1, "epoll creat error");
     //0：默认行为，与 epoll_create 类似。
    // EPOLL_CLOEXEC：设置文件描述符的“关闭时执行”标志（close-on-exec），即在新进程执行 exec 时自动关闭该描述符。

    struct epoll_event events[MAX_EVENTS],ev;
    memset(events,0,sizeof(events));
    ev.events = EPOLLIN;
    ev.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &ev);

    while(1){
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        errif(nfds==-1,"epoll wait error");
        for(int i =0;i<nfds;i++){
            if(events[i].data.fd==serv_sock){ //新客户端链接
                socklen_t clnt_addr_len = sizeof(clnt_addr); 
                clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_len );
                errif(clnt_sock == -1, "socket accept error");
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sock, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clnt_sock;
                set_nonblocking(clnt_sock);
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &ev);
            } else if(events[i].events & EPOLLIN){ //检查 EPOLLIN 位是否为 1无论其他位如何。更灵活，适合多事件组合。
                char buf[READ_BUFFER];
                while(1){
                    int str_len = read(events[i].data.fd,buf,READ_BUFFER-1);
                    if(str_len > 0){
                        buf[str_len] = '\0';
                        printf("message from client fd %d: %s\n", events[i].data.fd, buf);
                        write(events[i].data.fd,buf,str_len);
                    } else if(str_len==-1 && errno==EINTR){ //客户端正常中断，继续读取
                        printf("continue reading");
                        continue;
                    } else if(str_len==-1 && ((errno==EAGAIN) || (errno == EWOULDBLOCK))){ //非阻塞IO，这个条件表示数据全部读取完毕
                        printf("finish reading once, errno: %d\n", errno);
                        break;
                    } else{ //EOF，客户端断开连接 str_len==0
                        printf("EOF, client fd %d disconnected\n", events[i].data.fd);
                        close(events[i].data.fd); //关闭socket会自动将文件描述符从epoll树上移除
                        break;
                    }
                }

            } else{
                printf("something else happened\n");
            }

        }

    }
    close(serv_sock);   
    close(epfd);
    return 0;

}
