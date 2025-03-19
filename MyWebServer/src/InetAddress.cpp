#include "InetAddress.h"
#include <string.h>
#include <stdlib.h>

//初始化列表是“一次性构造”，效率更高。
//函数体内是“先构造再修改”，多一步操作
InetAddress::InetAddress(): addr_len(sizeof(addr)){
    memset(&addr,0,addr_len);
}

InetAddress::InetAddress(const char* port): addr_len(sizeof(addr)){
    memset(&addr,0,addr_len);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    addr.sin_port = htons(atoi(port));
}

InetAddress::InetAddress(const char* ip, const char* port): addr_len(sizeof(addr)){
    memset(&addr,0,addr_len);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip); //将点分十进制的 IPv4 地址字符串（例如 "127.0.0.1"）转换为 32 位网络字节序的整数
    addr.sin_port = htons(atoi(port));
}

InetAddress::~InetAddress(){
    
}