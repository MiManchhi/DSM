#include <unistd.h>
#include "util.h"
#include "proto.h"
#include "globals.h"
#include "service.h"
#include "server.h"

server_c::server_c()
{
}

server_c::~server_c()
{
}

//进程启动时
void server_c::proc_on_init(void)
{
    // MySQL地址表
    if(!cfg_maddrs || !strlen(cfg_maddrs))
        logger_fatal("mysql addresses is null");
    splitstring(cfg_maddrs, g_maddrs);
    if(g_maddrs.empty())
        logger_fatal("mysql addresses is empty");
    //主机名
    char hostname[256 + 1] = {};
    if(gethostname(hostname,sizeof(hostname)-1))
        logger_error("call gethostname fail:%s", strerror(errno));
    g_hostname = hostname;
    //最大偏移量不能太小
    if(cfg_maxoffset < 10)
        logger_fatal("invalid maximum offset:%d < 10", cfg_maxoffset);
    // 打印配置信息
    logger("cfg_maddrs:%s, cfg_mtimeout:%d, cfg_maxoffset:%d",
           cfg_maddrs, cfg_mtimeout, cfg_maxoffset);
}

//进程意图退出时
bool server_c::proc_exit_timer(size_t nclients, size_t nthreads)
{
    //ture立即退出
    //false 查看配置文件，若配置项ioctl_quick_abort非0，子进程立即退出
    //若配置项ioctl_quick_abort为0，等待所有客户机连接关闭后退出
    if(!nclients || !nthreads)
    {
        logger("nclients:%lu,nthreads:%lu", nclients, nthreads);
        return true;
    }
    return false;
}


//线程获得连接时，返回true连接继续可用，false连接关闭
bool server_c::thread_on_accept(acl::socket_stream *conn)
{
    logger("connect, from :%s", conn->get_peer());
    return true;
}

//连接可读时，返回true连接继续可用，false连接关闭
bool server_c::thread_on_read(acl::socket_stream *conn)
{
    //接收包体
    char head[HEADLEN];
    if(conn->read(head,HEADLEN) < 0)
    {
        if(conn->eof()) //客户机关闭了连接
        {
            logger("connect has been closed, from:%s", conn->get_peer());
        }
        else
        {
            logger_error("read fail:%s, from:%s", acl::last_serror(), conn->get_peer());
            return false;
        }
    }
    // 业务处理
    service_c service;
    return service.business(conn,head);
}

//线程读写超时时调用，返回true连接继续可用，false连接关闭
bool server_c::thread_on_timeout(acl::socket_stream *conn)
{
    logger("read timeout, from:%s", conn->get_peer());
    return true; // 长连接
}

//与线程绑定的连接关闭时被调用
void server_c::thread_on_close(acl::socket_stream *conn)
{
    logger("client disconnect, from:%s", conn->get_peer());
}
