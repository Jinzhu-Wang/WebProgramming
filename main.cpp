#include <iostream>
#include <sys/socket.h> // socket(), bind(), listen(), accept(), recv(), send()
#include <netinet/in.h> // sockaddr_in, INADDR_ANY, htons()
#include <cstring> // memset()
#include <unistd.h> // close()
#include <arpa/inet.h> // inet_addr()