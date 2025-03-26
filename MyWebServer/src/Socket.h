#ifndef SOCKET_H
#define SOCKET_H

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
    char* get_ip();
    uint16_t get_port();
};

class Socket{
private:
    int fd;

public:
    Socket();
    Socket(int fd);

    void bind(InetAddress* inet_addr);
    void listen();
    int accept(InetAddress* inet_addr);
    void connect(InetAddress *_addr);
    void set_nonblocking();
    int getfd();
    ~Socket();

};



#endif