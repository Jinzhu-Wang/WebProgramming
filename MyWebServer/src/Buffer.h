#ifndef BUFFER_H
#define BUFFER_H

#include <string>

class Buffer
{
private:
    std::string buf_;
public:
    Buffer();
    ~Buffer();
    void append(const char* str, int size);
    ssize_t size();
    const char* c_str();
    void clear();
    void getline();
    void set_buf(const char* buf);
};

#endif