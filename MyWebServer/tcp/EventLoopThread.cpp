#include "EventLoopThread.h"
#include "EventLoop.h"
#include <iostream>

EventLoopThread::EventLoopThread():loop_(nullptr){}

EventLoopThread::~EventLoopThread(){
    std::cout<< "EventLoopThread 析构调用" << std::endl;
    if (thread_.joinable()) {
        std::cout<< "等待子线程结束" << std::endl;
        thread_.join();
    }
}

EventLoop* EventLoopThread::StartLoop(){
    //绑定当前线程的所执行的函数，并创建子线程
    //在这个线程中创建EventLoop
    thread_ = std::thread(std::bind(&EventLoopThread::ThreadFunc,this)); //创建线程运行 func,这里的this是子线程(前面创建的sub_reactor)
    // 成员函数不能直接作为线程的入口，需要一个对象实例来调用。thread_ = std::thread([this]() { this->ThreadFunc(); });
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_==nullptr){
            cv_.wait(lock); // 当IO线程未创建LOOP时，阻塞且释放锁
        }
        // 将IO线程创建的loop_赋给主线程。
        loop = loop_;
    }
    // 返回创建好的线程。
    return loop;
}
void EventLoopThread::ThreadFunc(){
    // 由IO线程创建EventLoop对象
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&loop; // 获取子线程的地址
        cv_.notify_one(); // loop_被创建成功，发送通知，唤醒主线程。
    }
    loop_->Loop(); //开始循环，线程阻塞在这里直到析构
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = nullptr;
    }
}