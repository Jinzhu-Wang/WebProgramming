#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handing(char* message);

int main(int argc,char* argv[]){
    if(argc!=3){
        printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
    }

    int count,result;
    char message[BUF_SIZE];

    struct sockaddr_in serv_addr;

    int clnt_sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]); //将点分十进制的 IP 地址字符串转换为 32 位无符号整数
    serv_addr.sin_port = atoi(argv[2]);

    if(connect(clnt_sock,(struct sockaddr*)& serv_addr, (socklen_t)sizeof(serv_addr))==-1)
    	error_handing("connect() error!");
	  else
		  puts("Connected...........");
    fputs("Operand count (0 is Quit): ", stdout);
    scanf("%d",&count);
    message[0]=(char) count;

    if(count==0)
        write(clnt_sock,message,1);

    else{
        for(int i =0;i<count;i++){
            printf("Operand %d: ",i+1);
            scanf("%d",(int*)&message[i*4+1]);
        }
        fgetc(stdin);
        fputs("Operator: ",stdout);
        scanf("%s",&message[4*count+1]);
        if(write(clnt_sock,message,count*4+2)==-1)
            error_handing("write error");
        read(clnt_sock,&result,4);
        printf("Operation result: %d \n",result);
    }
    close(clnt_sock);    
    return 0;

}
void error_handing(char* message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}