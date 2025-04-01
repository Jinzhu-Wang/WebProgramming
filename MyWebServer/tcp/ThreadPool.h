#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include "common.h"

class ThreadPool{
private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex tasks_mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
public:
    ThreadPool(int size = std::thread::hardware_concurrency());
    ~ThreadPool();

    // void add(std::function<void()>);
    template<class F, class... Args>
    auto Add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>; //将推导出的返回类型包装成一个 std::future 对象，表示异步操作的结果。
    //上面必须用typename,因为std::result_of<F(Args...)>::type是依赖于模板参数的类型，编译器需要 typename 来明确这是一个类型名。
};

template<class F, class... Args>
auto ThreadPool::Add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...) //对参数包 args 展开并逐个应用 std::forward，保留每个参数的值类别。
    );  //std::make_shared<T>(args...) 创建一个类型为 T 的对象，并返回一个 std::shared_ptr<T> 管理它。

    std::future<return_type> res = task->get_future(); //返回一个 std::future，与该任务的结果绑定。
    {
        std::unique_lock<std::mutex> lock (tasks_mtx_);

        // don't allow enqueueing after stopping the pool
        if(stop_)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks_.emplace([task](){(*task)();});
    }
    cv_.notify_one();
    return res;
    
}



#endif