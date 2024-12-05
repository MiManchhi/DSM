//ID服务器
//业务服务类实现

#include "service.h"
#include "proto.h"
#include "globals.h"
#include "db.h"
#include "util.h"

service_c::service_c()
{

}

service_c::~service_c()
{

}

bool service_c::business(acl::socket_stream *conn, char const *head) const
{
    //     |包体长度|命令|状态|  包体  |
    //     |  8    | 1  | 1  |       |
    //解析包头  只收包头，包体在具体业务中再收
    //包体长度
    long long bodylen = ntoll(head);    //转换主机字节序
    if(bodylen < 0)
    {
        error(conn,-1,"invalid body length:%lld < 0",bodylen);
        return false;
    }
    //命令
    int command = head[BODYLEN_SIZE];
    //状态
    int status = head[BODYLEN_SIZE+COMMAND_SIZE];
    logger("bodylen:%lld, command:%d, status:%d",bodylen,command,status);
    //根据命令执行具体业务处理
    bool result;
    switch (command)
    {
    case CMD_ID_GET:
        result = get(conn,bodylen);
        break;
    default:
        error(conn,-1,"unknow command:%d",command);
        return false;
    }
    return result;
}

bool service_c::get(acl::socket_stream *conn, long long bodylen) const
{
    return false;
}

long service_c::get(char const *key) const
{
    return 0;
}

long service_c::valueFromDB(char const *key) const
{
    return 0;
}

bool service_c::id(acl::socket_stream *conn, long value) const
{
    return false;
}

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
    respond[BODYLEN_SIZE] = CMD_ID_REPLY;
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