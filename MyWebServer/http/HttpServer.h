#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include <functional>
#include <memory>
#include <stdio.h>
#include "common.h"


class TcpServer;
class TcpConnection;
class HttpRequest;
class HttpResponse;
class EventLoop;

class HttpServer
{
public:
    DISALLOW_COPY_AND_MOVE(HttpServer);

    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void(const HttpRequest &, HttpResponse *)> HttpResponseCallback;

    HttpServer(EventLoop *loop, const char *ip, const int port);
    ~HttpServer();

    void HttpDefaultCallBack(const HttpRequest &request, HttpResponse *resp);

    void SetHttpCallback(const HttpResponseCallback &cb);

    void start();

    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn);
    void onRequest(const TcpConnectionPtr &conn, const HttpRequest &request);

    void SetThreadNums(int thread_nums);

private:
    EventLoop *loop_;
    std::unique_ptr<TcpServer> server_;

    HttpResponseCallback response_callback_;
};


#endif