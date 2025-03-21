//存储服务器
//实现文件操作类
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include "file.h"
#include "types.h"

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
    //检查路径
    if(!path || !strlen(path))
    {
        logger_error("path is null");
        return ERROR;
    }
    //打开文件
    if(flag == O_READ)
        m_fd = ::open(path, O_RDONLY);
    else if(flag == O_WRITE)
        m_fd = ::open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    else
    {
        logger_error("unknow open flag:%c", flag);
        return ERROR;
    }
    //打开失败
    if(m_fd == -1)
    {
        logger_error("call open fail:%s, path:%s, flag:%c", strerror(errno), path, flag);
        return ERROR;
    }
    return OK;
}

// 关闭文件
int file_c::close()
{
    if(m_fd != -1)
    {
        if(::close(m_fd) == -1)
        {
            logger_error("call close fail:%s", strerror(errno));
            return ERROR;
        }
        m_fd = -1;
    }
    return OK;
}

// 读取文件
int file_c::read(void *buf, size_t count) const
{
    //检查文件描述符
    if(m_fd == -1)
    {
        logger_error("invalid file descriptor");
        return ERROR;
    }
    //检查文件缓冲区
    if(!buf)
    {
        logger_error("invalid file buffer");
        return ERROR;
    }
    //读取给定字节数 如果返回读取的字节数不等于count均视为错误
    /*
        ssize_t read(int fd, void *buf, size_t count);
        读错误返回-1
        读到文件尾返回0
        正常情况返回读取的字节数
        返回值<= count
    */
    ssize_t bytes = ::read(m_fd, buf, count);
    if(bytes == -1)
    {
        logger_error("call read fail:%s", strerror(errno));
        return ERROR;
    }
    if((size_t)bytes != count)
    {
        logger_error("unable to read expected bytes:%ld != %lu", bytes, count);
        return ERROR;
    }
    return OK;
}

// 写入文件
int file_c::write(void const *buf, size_t count) const
{
    //检查文件描述符
    if(m_fd == -1)
    {
        logger_error("invalid file descriptor");
        return ERROR;
    }
    //检查文件缓冲区
    if(!buf)
    {
        logger_error("invalid file buffer");
        return ERROR;
    }
    //写入指定字节数
    if(::write(m_fd,buf,count) == -1)
    {
        logger_error("call write fail:%s", strerror(errno));
        return ERROR;
    }
    return OK;
}

// 设置偏移
int file_c::seek(off_t offset) const
{
    //检查文件描述符
    if(m_fd == -1)
    {
        logger_error("invalid file descriptor");
        return ERROR;
    }
    //检查偏移是否合法
    if(offset < 0)
    {
        logger_error("invalid file offset");
        return ERROR;
    }
    //设置偏移
    if(lseek(m_fd,offset,SEEK_SET) == -1)
    {
        logger_error("call lseek fail :%s, offset:%ld", strerror(errno), offset);
        return ERROR;
    }
    return OK;
}

// 删除文件
int file_c::del(char const *path)
{
    //检查路径
    if(!path||!strlen(path))
    {
        logger_error("path is null");
        return ERROR;
    }
    if(unlink(path) == -1)
    {
        logger_error("call unlink fail:%s,path:%s", strerror(errno), path);
        return ERROR;
    }
    return 0;
}


