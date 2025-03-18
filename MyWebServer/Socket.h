#ifndef SOCKET_H
#define SOCKET_H

class InetAddress;
class Socket{
private:
    int fd;

public:
    Socket();
    Socket(int fd);

    void bind(const InetAddress* inet_addr);
    void listen();
    int accept(const InetAddress* inet_addr);
    void set_nonblocking();
    int getfd();
    ~Socket();

};

#endif