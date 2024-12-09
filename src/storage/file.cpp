//存储服务器
//实现文件操作类
#pragma once

#include "file.h"

file_c::file_c() : m_fd(-1)
{

}

file_c::file_c(char const *path, char flag)  //只有静态成员函数可以在初始化列表里调用
{
    file_c::open(path, flag);
}

file_c::~file_c()
{
    file_c::close();
}

//打开文件
int file_c::open(char const *path, char flag)
{
    return 0;
}

// 关闭文件
int file_c::close()
{
    return 0;
}

// 读取文件
int file_c::read(void *buf, size_t count) const
{
    return 0;
}

// 写入文件
int file_c::write(void const *buf, size_t count) const
{
    return 0;
}

// 设置偏移
int file_c::seek(off_t offset) const
{
    return 0;
}

// 删除文件
int file_c::del(char const *path)
{
    return 0;
}


