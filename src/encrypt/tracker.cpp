//存储服务器
//实现跟踪客户端线程类
#include <unistd.h>
#include "proto.h"
#include "util.h"
#include "tracker.h"
#include "globals.h"


tracker_c::tracker_c() : m_stop(false)
{

}
tracker_c::tracker_c(char const* taddr) : m_stop(false), m_taddr(taddr)
{

}
tracker_c::~tracker_c()
{

}
//终止线程
void tracker_c::stop()
{
    m_stop = true;
}

//线程过程
void *tracker_c::run(void)
{
    acl::socket_stream conn;  
    while (!m_stop)
    {
        // 连接跟踪服务器
        if(!conn.open(m_taddr,10,30))
        {
            logger_error("connect tracker fail,taddr:%s", m_taddr.c_str());
            sleep(2);
            // 失败重连
            continue;
        }
        //向跟踪服务器发送加入包
        if(join(&conn) != OK)
        {
            conn.close();
            sleep(2);
            // 失败重连
            continue;
        }
        //上次心跳
        time_t last = time(NULL);
        while (!m_stop)
        {
            // 现在
            time_t now = time(NULL);
            // 现在距离上次心跳已经足够久，再跳一次
            if(now - last >= cfg_interval)
            {   
                // 向跟踪服务器发送心跳包
                if(beat(&conn) != OK)
                    break;
                last = now;
            }
            sleep(1);
        }
        conn.close();
    }
    return nullptr;
}

//向跟踪服务器发送加入包
int tracker_c::join(acl::socket_stream *conn) const
{
    //构造请求
    // |包体长度|命令|状态|encrypt_join_body_t|
    // |  8    | 1  | 1  |   包体长度        |
    long long bodylen = sizeof(encrypt_join_body_t);
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    llton(bodylen, request);
    request[BODYLEN_SIZE] = CMD_ENCRYPT_JOIN;
    request[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    encrypt_join_body_t *ejb = reinterpret_cast<encrypt_join_body_t *>(request + HEADLEN);
    // 版本--组名--主机名--端口号--启动时间--加入时间
    strcpy(ejb->ejb_version, g_version);
    strcpy(ejb->ejb_groupname, cfg_gpname);
    strcpy(ejb->ejb_hostname, g_hostname.c_str());
    ston(cfg_bindport, ejb->ejb_port);
    lton(g_stime, ejb->ejb_stime);
    lton(time(NULL), ejb->ejb_jtime);
    // 发送请求
    if(conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s,requestlen:%lld,to:%s", acl::last_serror(), requestlen, conn->get_peer());
        return SOCKET_ERROR;
    }
    // 接收包头
    char head[HEADLEN];
    if(conn->read(head,HEADLEN) < 0)
    {
        logger_error("read fail:%s,from:%s", acl::last_serror(), conn->get_peer());
        return SOCKET_ERROR;
    }
    //  |包体长度|命令|状态|
    //  |   8   | 1  | 1  |
    // 解析包头
    if((bodylen = ntoll(head)) < 0)
    {
        logger_error("invalild body length:%lld < 0", bodylen);
        return ERROR;
    }
    int command = head[BODYLEN_SIZE];
    int status = head[BODYLEN_SIZE + COMMAND_SIZE];
    logger("bodylen:%lld,command:%d,status:%d", bodylen, command, status);
    // 检查命令
    if(command != CMD_TRACKER_REPLY)
    {
        logger_error("unknown command:%d", command);
        return ERROR;
    }
    // 应答成功
    if(!status)
        return OK;
    //  |包体长度|命令|状态|错误号|错误描述|
    //  |   8   | 1  | 1  |  2  | <=1024|
    // 检查包体长度
    long long expectedlen = ERROR_NUMB_SIZE + ERROR_DESC_SIZE;
    if(bodylen > expectedlen)
    {
        logger_error("invalid body length:%lld > %lld", bodylen, expectedlen);
        return ERROR;
    }
    // 接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail:%s,bodylen:%lld,from:%s", acl::last_serror(), bodylen, conn->get_peer());
        return SOCKET_ERROR;
    }
    // 解析包体
    short errnumb = ntos(body);
    char const *errdesc = "";
    if(bodylen > ERROR_NUMB_SIZE)
        errdesc = body + ERROR_NUMB_SIZE;
    logger_error("join fail,errnumb:%d,errdesc:%s", errnumb, errdesc);
    return ERROR;
}

// 向跟踪服务器发送心跳包
int tracker_c::beat(acl::socket_stream *conn) const
{
    //构造请求
    // |包体长度|命令|状态|encrypt_beat_body_t|
    // |  8    | 1  | 1  |   包体长度        |
    long long bodylen = sizeof(encrypt_beat_body_t);
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    llton(bodylen, request);
    request[BODYLEN_SIZE] = CMD_ENCRYPT_BEAT;
    request[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    encrypt_beat_body_t *ebb = reinterpret_cast<encrypt_beat_body_t *>(request + HEADLEN);
    //组名--主机名
    strcpy(ebb->ebb_groupname, cfg_gpname);
    strcpy(ebb->ebb_hostname, g_hostname.c_str());
    // 发送请求
    if(conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s,requestlen:%lld,to:%s", acl::last_serror(), requestlen, conn->get_peer());
        return SOCKET_ERROR;
    }
    // 接收包头
    char head[HEADLEN];
    if(conn->read(head,HEADLEN) < 0)
    {
        logger_error("read fail:%s,from:%s", acl::last_serror(), conn->get_peer());
        return SOCKET_ERROR;
    }
    //  |包体长度|命令|状态|
    //  |   8   | 1  | 1  |
    // 解析包头
    if((bodylen = ntoll(head)) < 0)
    {
        logger_error("invalild body length:%lld < 0", bodylen);
        return ERROR;
    }
    int command = head[BODYLEN_SIZE];
    int status = head[BODYLEN_SIZE + COMMAND_SIZE];
    logger("bodylen:%lld,command:%d,status:%d", bodylen, command, status);
    // 检查命令
    if(command != CMD_TRACKER_REPLY)
    {
        logger_error("unknown command:%d", command);
        return ERROR;
    }
    // 应答成功
    if(!status)
        return OK;
    //  |包体长度|命令|状态|错误号|错误描述|
    //  |   8   | 1  | 1  |  2  | <=1024|
    // 检查包体长度
    long long expectedlen = ERROR_NUMB_SIZE + ERROR_DESC_SIZE;
    if(bodylen > expectedlen)
    {
        logger_error("invalid body length:%lld > %lld", bodylen, expectedlen);
        return ERROR;
    }
    // 接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail:%s,bodylen:%lld,from:%s", acl::last_serror(), bodylen, conn->get_peer());
        return SOCKET_ERROR;
    }
    // 解析包体
    short errnumb = ntos(body);
    char const *errdesc = "";
    if(bodylen > ERROR_NUMB_SIZE)
        errdesc = body + ERROR_NUMB_SIZE;
    logger_error("beat fail,errnumb:%d,errdesc:%s", errnumb, errdesc);
    return ERROR;
}