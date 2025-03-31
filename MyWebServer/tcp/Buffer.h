#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"
#include <memory>
#include <string>

class Buffer
{
private:
    std::string buf_;
public:
    DISALLOW_COPY_AND_MOVE(Buffer);
    Buffer() = default;
    ~Buffer() = default;
    
    const std::string& buf() const; //成员函数为const,且返回类型为const
    const char* c_str() const;

    void set_buf(const char* buf);

    ssize_t Size() const;
    void Append(const char* str, int size);
    void Clear();
    
};

#endif