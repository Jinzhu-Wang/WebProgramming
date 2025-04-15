#ifndef COMMON_H
#define COMMON_H

class Epoller;
class Channel;
class Buffer;
class EventLoop;
class TcpConnection;
class ThreadPool;
class Acceptor;
class TcpServer;

class HttpServer;
class HttpContext;
class HttpRequest;
class HttpResponse;

// Macros to disable copying and moving
#define DISALLOW_COPY(cname)     \
  cname(const cname &) = delete; \
  cname &operator=(const cname &) = delete;      //删除拷贝构造  //删除赋值运算符

#define DISALLOW_MOVE(cname) \
  cname(cname &&) = delete;  \
  cname &operator=(cname &&) = delete;

#define DISALLOW_COPY_AND_MOVE(cname) \
  DISALLOW_COPY(cname);               \
  DISALLOW_MOVE(cname);

enum RC {
  RC_UNDEFINED,
  RC_SUCCESS,
  RC_SOCKET_ERROR,
  RC_POLLER_ERROR,
  RC_CONNECTION_ERROR,
  RC_ACCEPTOR_ERROR,
  RC_UNIMPLEMENTED
};
#endif