#include "Buffer.h"
#include <string.h>
#include <iostream>
Buffer::Buffer(){}
Buffer::~Buffer(){}

void Buffer::append(const char* str, int size){
     for(int i = 0;i < size; i++){
        if(str[i]=='\0') break;
        buf_.push_back(str[i]);
     }
}

ssize_t Buffer::size(){
    return buf_.size();
}

const char* Buffer::c_str(){
    return buf_.c_str();
}

void Buffer::clear(){
    buf_.clear();
}

void Buffer::getline(){
    buf_.clear();
    std::getline(std::cin, buf_);
}