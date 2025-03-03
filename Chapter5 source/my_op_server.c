#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h> //read, write, close
#include <arpa/inet.h>  //将点分十进制的IP地址转换为返回一个 32 位无符号整数
#include <sys/socket.h> //socket, bind, listen, accept

#define BUF_SIZE 1024

void error_handing(char* message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

int main(int argc , char *argv[]){
    int calculate(int opnum, int* opnds, char oprator);
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr,clnt_addr;
    int str_len,recv_len;

    char message[BUF_SIZE];


    serv_sock = socket(PF_INET,SOCK_STREAM,0);
    if(serv_sock==-1)
        error_handing("serv_sock error");

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //把long类型的数据从主机字节序变成网络字节序（big endian）
    serv_addr.sin_port = atoi(argv[1]);

    if(bind(serv_sock,(struct sockaddr*) &serv_addr,sizeof(serv_addr))==-1)
        error_handing("bind error");

    if(listen(serv_sock,5)==-1)
        error_handing("listen error");
    socklen_t clnt_len =sizeof(clnt_addr);
    while(1){
        clnt_sock = accept(serv_sock,(struct sockaddr*)& clnt_addr, &clnt_len);
        if(clnt_sock==-1)
            error_handing("clnt_sock error");
        else
            puts("Connected client");

        int count_num=0;
        read(clnt_sock,&count_num,1); //read第二个参数是地址
        if(count_num==0)
            break;
        recv_len =0;
        while((count_num*4+1)>recv_len){
            str_len = read(clnt_sock,message,BUF_SIZE-1);
            recv_len += str_len;
        }

        int result = calculate(count_num,(int*) message,message[recv_len-1]);
        write(clnt_sock,&result,sizeof(result));

        close(clnt_sock);
    }
    close(serv_sock);
    return 0;    
}

int calculate(int count_num,int* operands, char oper){
    int result=operands[0];
    switch(oper){
        case '+':
            for(int i =1;i<count_num;i++) result += operands[i];
            break;
        case '-':
            for(int i =1;i<count_num;i++) result -= operands[i];
            break;
        case '*':
            for(int i =1;i<count_num;i++) result *= operands[i];
            break;
    }
    return result;
}