//存储服务器
//ID客户机类
#include "id.h"
#include "globals.h"
#include "proto.h"
#include "util.h"
#include "types.h"
#include <acl-lib/acl_cpp/lib_acl.hpp>

id_c::id_c()
{

}

id_c::~id_c()
{

}

//从ID服务器获取与ID键相对应的值
long id_c::get(char const *key) const
{
    //检查ID键
    if(!key)
    {
        logger_error("key is null");
        return -1;
    }
    size_t keylen = strlen(key);
    if(!keylen)
    {
        logger_error("key is null");
        return -1;
    }
    if(keylen > ID_KEY_MAX)
    {
        logger_error("key too big:%lu > %d", keylen, ID_KEY_MAX);
        return -1;
    }
    //  |包体长度|命令| 状态 |  ID键  |
    //  |  8    |  1 |  1  |包体长度 |
    //构造请求
    long long bodylen = keylen + 1;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    llton(bodylen, request);
    request[BODYLEN_SIZE] = CMD_ID_GET;
    request[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    strcpy(request + HEADLEN, key);
    // 向ID服务器发送请求，接收并解析响应。从中获取ID值
    return client(request, requestlen);
}

//向ID服务器发送请求，接收并解析响应。从中获取ID值
int id_c::client(char const *requ, long long resqulen) const
{
    acl::socket_stream conn;
    //从ID服务器地址表中随机抽取一台服务器尝试连接
    srand(time(NULL));
    int nids = g_iaddrs.size();
    int nrand = rand() % nids;
    for (int i = 0; i < nids; ++i)
    {
        if(conn.open(g_iaddrs[nrand].c_str(),0,0))  //open连接
        {
            logger("connect id success, addr:%s", g_iaddrs[nrand].c_str());
            break;
        }
        else
        {
            logger("connect id fail, addr:%s", g_iaddrs[nrand].c_str());
            nrand = (nrand + 1) % nids;
        }
    }
    if(!conn.alive()) //没有连接成功
    {
        logger_error("connect id fail, addrs:%s", cfg_iaddrs);
        return -1;
    }
    //向ID服务器发送请求
    if(conn.write(requ,resqulen) < 0)
    {
        logger_error("write fail:%s,requlen:%lld, to:%s", acl::last_serror(), resqulen, conn.get_peer());
        conn.close();
        return -1;
    }
    //从ID服务器接收响应
    long long respondlen = HEADLEN + BODYLEN_SIZE;
    char respond[respondlen] = {};
    if(conn.read(respond,respondlen) < 0)
    {
        logger_error("read fail:%s,respondlen:%lld,from:%s", acl::last_serror(), respondlen, conn.get_peer());
        conn.close();
        return -1;
    }
    long value = ntol(respond + HEADLEN);
    conn.close();
    return value;
}
