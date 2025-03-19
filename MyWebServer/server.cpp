#include "src/EventLoop.h"
#include "src/Server.h"
#include "src/util.h"

int main(int argc,char*argv[]) {
    errif(argc!=2,"argc error");
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop,argv[1]);
    loop->loop();
    return 0;
}