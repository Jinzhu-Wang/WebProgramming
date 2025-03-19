#ifndef INETADDRESS_H
#define INETADDRESS_H
#include <arpa/inet.h>

class InetAddress{
public:
    struct sockaddr_in addr;
    socklen_t addr_len;
    InetAddress();
    InetAddress(const char* port);
    InetAddress(const char* ip, const char* port);
    ~InetAddress();

};


#endif