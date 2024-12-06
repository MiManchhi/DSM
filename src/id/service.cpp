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
    // |包体长度|命令|状态|ID键|
    // |  8    |  1 | 1 |64+1|
    //检查包体长度
    long long expectedlen = ID_KEY_MAX + 1;
    if(bodylen > expectedlen)
    {
        error(conn, -1, "invalid body length:%lld > %lld", bodylen, expectedlen);
        return false;
    }
    // 接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail:%s, bodylen:%lld, from:%s", acl::last_serror(), bodylen, conn->get_peer());
        return false;
    }
    // 根据ID的键获取其值
    long value = get(body);
    if(value < 0)
    {
        error(conn, -1, "get id fail,key:%s", body);
        return false;
    }
    logger("get id ok,key:%s, value:%ld", body, value);
    return id(conn, value);
}

long service_c::get(char const *key) const
{
    //互斥锁加锁
    if ((errno = pthread_mutex_lock(&g_mutex))) {
		logger_error("call pthread_mutex_lock fail: %s",
			strerror(errno));
		return -1;
	}
    long value = -1;
    // 在ID表中查找ID
    bool IsID = false;
    for (auto &id : g_ids)
    {
        //找到ID
        if(strcmp(id.id_key,key) == 0)
        {
            IsID = true;
            // ID偏移量未达上限---->获取ID并++偏移量
            if(id.id_offset < cfg_maxoffset)
            {
                value = id.id_value + id.id_offset;
                ++id.id_offset;
            }
            // 偏移量达到上限----->从数据库中获取ID----->更新ID表中的ID
            else if((value = valueFromDB(key)) >= 0) 
            {
                id.id_value = value;
                id.id_offset = 1;
            }
        }
    }
    //没找到ID---->从数据库中获取ID
    if(!IsID)
    {
        if((value = valueFromDB(key)) >= 0)
        {
            //在ID表中添加ID
            id_pair_t id;
            strcpy(id.id_key, key);
            id.id_value = value;
            id.id_offset = 1;
            g_ids.push_back(id);
        }
    }
    //互斥锁解锁
    if ((errno = pthread_mutex_unlock(&g_mutex))) {
		logger_error("call pthread_mutex_unlock fail: %s",
			strerror(errno));
		return -1;
	}
    return value;
}

long service_c::valueFromDB(char const *key) const
{
    //数据库访问对象
    db_c db;
    //连接数据库
    if(db.connect() != OK)
        return -1;
    long value;
    // 获取ID当前值，同时产生下一个值
    if(db.getID(key,cfg_maxoffset,value) != OK)
        return -1;
    return value;
}

bool service_c::id(acl::socket_stream *conn, long value) const
{
    // |包体长度|命令|状态|ID值|
    // |   8   | 1  | 1  | 8  |
    //构造响应
    long long bodylen = BODYLEN_SIZE;
    long long respondlen = HEADLEN + bodylen;
    char respond[respondlen] = {};
    llton(bodylen, respond);
    respond[BODYLEN_SIZE] = CMD_ID_REPLY;
    respond[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    lton(value, respond + HEADLEN);
    // 发送响应
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail:%s,respondlen:%lld,to:%s",acl::last_serror(),respondlen,conn->get_peer());
        return false;
    }
    return true;
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