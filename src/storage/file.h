//存储服务器
//声明文件操作类
#pragma once

#include <sys/types.h>

class file_c
{
public:
    file_c();
    file_c(char const *path, char flag);
    ~file_c();

public:
    //打开文件
    int open(char const *path, char flag);

    // 关闭文件
    int close();

    // 读取文件
    int read(void *buf, size_t count) const;

    // 写入文件
    int write(void const *buf, size_t count) const;

    // 设置偏移
    int seek(off_t offset) const;

    // 删除文件
    static int del(char const *path);

    // 打开标志
    static const char O_READ = 'r';
    static const char O_WRITE = 'w';

private:
    //文件描述符
    int m_fd;
};
