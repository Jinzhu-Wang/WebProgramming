#ifndef INETADDRESS_H
#define INETADDRESS_H
#include <arpa/inet.h>

class InetAddress{
private:
    struct sockaddr_in addr;
public:
    
    socklen_t addr_len;
    InetAddress();
    InetAddress(const char* port);
    InetAddress(const char* ip, const char* port);
    ~InetAddress();

    void set_inetaddr(sockaddr_in _addr);
    sockaddr_in get_addr();

};


#endif