//密钥协商服务器
//实现业务服务类
#include "proto.h"
#include "util.h"
#include "globals.h"
#include "db.h"
#include "service.h"

service_c::service_c()
{}
service_c::~service_c()
{}
//业务处理
bool service_c::business(acl::socket_stream *conn, char const *head) const
{
    //  |包体长度|命令|状态|  包体 |
    //  |  8    |  1 | 1  |bodylen|
    //解析包头
    long long bodylen = ntoll(head);
    if(bodylen < 0)
    {
        error(conn, -1, "invalid body length: %lld < 0", bodylen);
        return false;
    }
    int command = head[BODYLEN_SIZE];  //命令
    int status = head[BODYLEN_SIZE + COMMAND_SIZE]; //状态
    logger("bodylen:%lld, command:%d,status:%d", bodylen, command, status);
    bool result;
    // 根据命令执行业务处理
    switch (command)
    {
    case CMD_CLIENT_REGISTER_PUNLICKEY:
        result = clientRegisterPublicKey(conn, bodylen);
        break;
    case CMD_SERVER_REGISTER_PUNLICKEY:
        result = serverRegisterPublicKey(conn, bodylen);
        break;
    case CMD_CLIENT_NEGOKEY:
        result = clientKeyNego(conn, bodylen);
        break;
    case CMD_SERVER_NEGOKEY:
        result = serverKeyNego(conn, bodylen);
        break;
    default:
        error(conn, -1, "unknown command:%d", command);
        return false;
    }
    return result;
}
//处理来自客户机公钥注册请求
bool service_c::clientRegisterPublicKey(acl::socket_stream *conn, long long bodylen) const
{
    // |包体长度|命令|状态|应用ID|用户ID|公钥长度|公钥|
    // |   8   | 1  | 1  |  16 | 256  |  8    |    |
    return true;
}
//处理来自存储服务器公钥注册请求
bool service_c::serverRegisterPublicKey(acl::socket_stream *conn, long long bodylen) const
{
    // |包体长度|命令|状态|应用ID|服务器ID|公钥长度|公钥|
    // |   8   | 1  | 1  |  16 |  256   |  8    |    |
    return true;
}
// 处理来自客户机的密钥协商请求
bool service_c::clientKeyNego(acl::socket_stream *conn, long long bodylen) const
{
    // |包体长度|命令|状态|应用ID|用户ID|目标主机|
    // |  8    |  1 | 1  |  16 | 256  |       |

    // |包体长度|命令|状态|密钥长度|密钥|
    // |   8   | 1  | 1  |  8    |    |
    return true;
}
// 处理来自存储服务器的密钥协商请求
bool service_c::serverKeyNego(acl::socket_stream *conn, long long bodylen) const
{
    // |包体长度|命令|状态|应用ID|服务器ID|目标主机|
    // |  8    |  1 | 1  |  16 | 256  |       |

    // |包体长度|命令|状态|密钥长度|密钥|
    // |   8   | 1  | 1  |  8    |    |
    return true;
}
// 应答成功
bool service_c::ok(acl::socket_stream *conn) const
{
    // |包体长度|命令|状态|
    // |   8   |  1 |  1 |
    //构造响应
    long long bodylen = 0;
    long long respondlen = HEADLEN+bodylen;
    char respond[respondlen] = {};
    llton(bodylen,respond);
    respond[BODYLEN_SIZE] = CMD_KEYNEGO_SERVEER_REPLY;
    respond[BODYLEN_SIZE+COMMAND_SIZE] = 0;
    //发送响应
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail:%s, respondlen:%lld, to:%s",acl::last_serror(),respondlen,conn->get_peer());
        return false;
    }
    return true;
}
// 应答错误
bool service_c::error(acl::socket_stream *conn, short errnumb, char const *format, ...) const
{
    //错误描述
    char errdesc[ERROR_DESC_SIZE];
    va_list ap; 
    va_start(ap,format);  //获取格式化参数列表
    vsnprintf(errdesc,ERROR_DESC_SIZE,format,ap);  //算终止空白字符，总是加终止字符
    va_end(ap);
    logger_error("%s",errdesc);
    acl::string desc;
    desc.format("[%s] %s",g_hostname.c_str(),errdesc);
    memset(errdesc,0,sizeof(errdesc));
    strncpy(errdesc,desc.c_str(),ERROR_DESC_SIZE - 1); //不算空白字符，长度够加终止字符，不够不加
    size_t desclen = strlen(errdesc);
    desclen += desclen != 0; //如果描述信息长度不为零，加上空字符\0
    //构造响应
    long long bodylen = ERROR_NUMB_SIZE+desclen;
    long long respondlen = HEADLEN+bodylen;
    char respond[respondlen] = {};
    llton(bodylen,respond);
    respond[BODYLEN_SIZE] = CMD_KEYNEGO_SERVEER_REPLY;
    respond[BODYLEN_SIZE+COMMAND_SIZE] = STATUS_ERROR;
    ston(errnumb,respond+HEADLEN);
    if(desclen)
        strcpy(respond+HEADLEN+ERROR_NUMB_SIZE,errdesc);
    //发送响应
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail:%s,respondlen:%lld,to:%s",acl::last_serror(),respondlen,conn->get_peer());
        return false;
    }
    return true;
}