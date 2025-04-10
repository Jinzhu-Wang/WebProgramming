#ifndef TIMER_H
#define TIMER_H
#include <functional>
#include "common.h"
#include "TimeStamp.h"

class Timer
{
public:
    DISALLOW_COPY_AND_MOVE(Timer);
    Timer(TimeStamp timeStamp, std::function<void()>const &cb, double interval);
    ~Timer();

    void ReStart(TimeStamp now);
    void run() const;
    TimeStamp expiration() const;
    bool repeat() const;

private:
    TimeStamp expiration_; //定时器的到期时间（这是一个具体的时刻）
    std::function<void()> callback_; //到时后的回调函数
    double interval_; //如果循环监视，重复间隔
    bool repeat_;

};



#endif