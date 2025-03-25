#ifndef SOCKET_H
#define SOCKET_H

class InetAddress;
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