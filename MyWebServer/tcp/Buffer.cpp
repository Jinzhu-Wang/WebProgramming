#include "Buffer.h"
#include <string.h>
#include <iostream>

const std::string &Buffer::buf() const { return buf_; }
const char* Buffer::c_str() const{ return buf_.c_str();}

void Buffer::set_buf(const char* buf){
    std::string new_buf(buf); //将 C 风格字符串转换为 C++ 的 std::string，便于后续操作。
    buf_.swap(new_buf); //用新传入的内容替换 buf_ 的旧内容，同时释放旧内容
}

ssize_t Buffer::Size() const{ return buf_.size();}

void Buffer::Append(const char* str, int size){
     for(int i = 0;i < size; i++){
        if(str[i]=='\0') break;
        buf_.push_back(str[i]);
     }
}

void Buffer::Clear(){ buf_.clear();}


