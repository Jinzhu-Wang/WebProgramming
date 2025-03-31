#include "ThreadPool.h"

ThreadPool::ThreadPool(int size):stop_(false){
    for(int i=0;i<size;++i){
        threads_.emplace_back(std::thread([this](){
            while(true){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(tasks_mtx_);
                    cv_.wait(lock,[this](){
                        return stop_ || !tasks_.empty();
                    }); //检查条件是否满足，若为false，则释放线程锁并阻塞，等待条件变量cv_被通知。
                    if(stop_ && tasks_.empty()) return;
                    task = tasks_.front();
                    tasks_.pop();
                }
                task();
            }
        }));
    }
}

ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(tasks_mtx_);
        stop_ = true;
    }
    cv_.notify_all();
    for(std::thread& th : threads_){
        if(th.joinable()){
            th.join(); //调用 join() 的线程会阻塞，直到目标线程（即 std::thread 对象表示的线程）执行完毕。
        }
    }
}

