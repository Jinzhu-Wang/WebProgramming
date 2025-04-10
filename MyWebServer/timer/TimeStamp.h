#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <stdio.h>
#include <sys/time.h>
#include <ctime>

#include <string>

const int kMicrosecond2Second = 1000 * 1000;

class TimeStamp{
public:
    TimeStamp() : micro_seconds_(0) {}
    explicit TimeStamp(int64_t micro_seconds) : micro_seconds_(micro_seconds) {} //防止隐式转换（比如 TimeStamp ts = 123; 会被禁止，必须用 TimeStamp ts(123);
    
    bool operator<(const TimeStamp &rhs) const{
        return micro_seconds_ < rhs.microseconds();
    }
    bool operator==(const TimeStamp &rhs) const{
        return micro_seconds_ == rhs.microseconds();
    }
    std::string ToFormattedString() const { // 将时间戳转换为人类可读的字符串，格式为 YYYY-MM-DD HH:MM:SS.µµµµµµ
        char buf[64] = {0};
        time_t seconds = static_cast<time_t>(micro_seconds_ / kMicrosecond2Second);
        struct tm tm_time;
        localtime_r(&seconds, &tm_time); //将秒数转换为本地时间的 struct tm
        int microseconds = static_cast<int>(micro_seconds_ % kMicrosecond2Second); //取余计算剩余的微秒部分。
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d", //%02d：两位数字，补零
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds); 
        return buf; //格式化输出，年份加 1900（tm_year 从 1900 开始），月份加 1（tm_mon 从 0 开始），微秒用 6 位填充。
    }

    int64_t microseconds() const { return micro_seconds_; };
    static TimeStamp Now();
    static TimeStamp AddTime(TimeStamp timestamp, double add_seconds);
    

private:
    int64_t micro_seconds_;

};

inline TimeStamp TimeStamp::Now(){
    struct timeval time;
    gettimeofday(&time, NULL); //从 <sys/time.h> 获取当前时间，填充 struct timeval，包含秒 (tv_sec) 和微秒 (tv_usec)。
    return TimeStamp(time.tv_sec * kMicrosecond2Second + time.tv_usec);//将秒转换为微秒（乘以 1,000,000），加上微秒部分
};

inline TimeStamp TimeStamp::AddTime(TimeStamp timestamp, double add_seconds){
    int64_t add_microseconds = static_cast<int64_t>(add_seconds) * kMicrosecond2Second;   
    return TimeStamp(timestamp.microseconds() + add_microseconds);
};

#endif