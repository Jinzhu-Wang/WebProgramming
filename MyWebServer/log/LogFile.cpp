#include "LogFile.h"
#include "TimeStamp.h"
#include <string>
#include <iostream>
#include <cstring>

LogFile::LogFile(const char* filepath)
    : fp_(::fopen(filepath, "a+")),
      written_bytes_(0),
      lastwrite_(0),
      lastflush_(0) {
    if (!fp_) {
        // std::string DefaultPath = std::move("../LogFiles/LogFile_" +
        //                       TimeStamp::Now().TimeStamp::ToFormattedDefaultLogString() +
        //                       ".log");
        //对于临时对象没必要进行move，因为编译器通常会执行复制省略直接在目标位置构造对象，而无需复制或移动。
        std::string DefaultPath = "/home/wangjz/git_project/WebProgramming/MyWebServer/LogFiles/LogFile_" +
                                    TimeStamp::Now().TimeStamp::ToFormattedDefaultLogString() +
                                    ".log";

        fp_ = ::fopen(DefaultPath.data(), "a+");
        if (!fp_) {
            std::cerr << "Failed to open log file " << DefaultPath 
                      << ": " << strerror(errno) << std::endl;
            throw std::runtime_error("Cannot open log file: " + DefaultPath);
        }
    }
}

LogFile::~LogFile() {
    if(fp_){
        Flush();
        fclose(fp_);
    }
}

void LogFile::Write(const char* data, int len) {
    if (!fp_) {
        std::cerr << "Error: Log file pointer is null" << std::endl;
        return;
    }
    
    int pos = 0;
    while (pos != len) {
        // 使用无锁版本加快写入速度，一般一个系统只有一个后端日志系统。
        pos += static_cast<int>(fwrite_unlocked(data + pos, 
    										sizeof(char), len - pos, fp_));
    }
    time_t now = ::time(nullptr);
    // 更新当前状态
    if (len != 0) {
        lastwrite_ = now;
        written_bytes_ += len;
    }
    // 判断是否需要Flush
    if (lastwrite_ - lastflush_ > FlushInterval) {
        Flush();
        lastflush_ = now;
    }
}

int64_t LogFile::writtenbytes() const { return written_bytes_; }

void LogFile::Flush() {
    if (fp_) {
        fflush(fp_);
    } 
}