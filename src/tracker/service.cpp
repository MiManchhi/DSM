#include <algorithm>

#include "service.h"
#include "globals.h"
#include "proto.h"
#include "util.h"
#include "db.h"

service_c::service_c()
{
}

service_c::~service_c()
{
}

//业务处理
bool service_c::business(acl::socket_stream *conn, char const *head) const
{
    //     |包体长度|命令|状态|  包体  |
    //     |  8    | 1  | 1  |       |
    //解析包头
    //包体长度
    long long bodylen = ntoll(head);
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
    case CMD_TRACKER_JOIN:
        result = join(conn,bodylen);
        break;
    case CMD_TRACKER_BEAT:
        result = beat(conn,bodylen);
        break;
    case CMD_TRACKER_SADDRS:
        result = saddrs(conn,bodylen);
        break;
    case CMD_TRACKER_GROUPS:
        result = groups(conn);
        break;
    default:
        error(conn,-1,"unknow command:%d",command);
        return false;
    }
    return result;
}

//处理来自存储服务器的加入包
bool service_c::join(acl::socket_stream *conn, long long bodylen) const
{
    //   |包体长度|命令|状态|storage_join_body_t|
    //   |   8   | 1  | 1  |                   |
    //检查包体长度
    long long expectedLength = sizeof(storage_join_body_t);
    if(bodylen != expectedLength)
    {
        error(conn,-1,"invalid body length:%lld != %lld",bodylen,expectedLength);
        return false;
    }
    //接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail:%s, bodylen:%lld, from:%s",acl::last_serror(),bodylen,conn->get_peer());
        return false;
    }
    //解析包体
    storage_join_t sj;
    storage_join_body_t* sjb = (storage_join_body_t*)body;
    //版本
    strcpy(sj.sj_version,sjb->sjb_version);
    //组名
    strcpy(sj.sj_groupname,sjb->sjb_groupname);
    if(valid(sj.sj_groupname) != OK)
    {
        error(conn,-1,"valid groupname:%s",sj.sj_groupname);
        return false;
    }
    //主机名
    strcpy(sj.sj_hostname,sjb->sjb_hostname);
    //端口号
    sj.sj_port = ntos(sjb->sjb_port);
    if(!sj.sj_port)
    {
        error(conn,-1,"invalid port:%u",sj.sj_port);
        return false;
    }
    //启动时间
    sj.sj_stime = ntol(sjb->sjb_stime);
    //加入时间
    sj.sj_jtime = ntol(sjb->sjb_jtime);
    logger("storage join,version:%s, groupname:%s, hostname:%s, port:%u, stime:%s, jtime:%s",
    sj.sj_version,sj.sj_groupname,sj.sj_hostname,sj.sj_port,std::string(ctime(&sj.sj_stime)).c_str(),std::string(ctime(&sj.sj_jtime)).c_str());
    //将存储服务器加入组表
    if(join(&sj,conn->get_peer()) != OK)
    {
        error(conn,-1,"join into groups fail");
        return false;
    }
    return ok(conn);
}

//处理来自存储服务器的心跳包
bool service_c::beat(acl::socket_stream *conn, long long bodylen) const
{
    //   |包体长度|命令|状态|storage_beat_body_t|
    //   |   8   | 1  | 1  |                   |
    //检查包体长度
    long long expectedLength = sizeof(storage_beat_body_t);
    if(bodylen != expectedLength)
    {
        error(conn,-1,"invalid body length:%lld != %lld",bodylen,expectedLength);
        return false;
    }
    //接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail:%s, bodylen:%lld, from:%s",acl::last_serror(),bodylen,conn->get_peer());
        return false;
    }
    //解析包体
    storage_beat_body_t* sbb = (storage_beat_body_t*)body;
    //组名
    char groupname[STORAGE_GROUPNAME_MAX+1];
    strcpy(groupname,sbb->sbb_groupname);
    //主机名
    char hostname[STORAGE_HOSTNAME_MAX+1];
    strcpy(hostname,sbb->sbb_hostname);
    logger("storage beat,groupname:%s, hostname:%s",groupname,hostname);
    //将存储服务器标记为活动
    if(beat(groupname,hostname,conn->get_peer()) != OK)
    {
        error(conn,-1,"mark storage as active fail");
        return false;
    }
    return ok(conn);
}

//处理来自客户机的获取存储服务器地址列表请求
bool service_c::saddrs(acl::socket_stream *conn, long long bodylen) const
{
    //  |包体长度|命令|状态|应用ID|用户ID|文件ID|
    //  |  8    | 1  | 1  | 16  |  256 | 128 |
    //检查包体长度
    long long expectLength = APPID_SIZE + USERID_SIZE + FILEID_SIZE;
    if(bodylen != expectLength)
    {
        error(conn,-1,"invalid body length:%lld != %lld",bodylen,expectLength);
        return false;
    }
    //接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail:%s,bodylen:%lld, from:%s",acl::last_serror(),bodylen,conn->get_peer());
        return false;
    }
    //解析包体
    char appid[APPID_SIZE];
    strcpy(appid,body);
    char userid[USERID_SIZE];
    strcpy(userid,body + APPID_SIZE);
    char fileid[FILEID_SIZE];
    strcpy(fileid,body + APPID_SIZE + USERID_SIZE);
    //响应客户机存储服务器地址列表
    if(saddrs(conn,appid,userid) != OK)
        return false;
    return true;
}

//处理来自客户机的获取组列表请求
bool service_c::groups(acl::socket_stream *conn) const
{
    return false;
}

//将存储服务器加入组表
int service_c::join(storage_join_t const *sj, char const *saddr) const
{
    return 0;
}

//将存储服务器标记为活动
int service_c::beat(char const *groupname, char const *hostname, char const *saddr) const
{
    return 0;
}

//响应客户机存储服务器地址列表
int service_c::saddrs(acl::socket_stream *conn, char const *appid, char const *userid) const
{
    return 0;
}

//根据用户ID获取其对应的组名
int service_c::group_of_user(char const *appid, char const *userid, std::string &groupname) const
{
    return 0;
}

//根据组名获取存储服务器地址列表
int service_c::saddrs_of_group(char const *groupname, std::string &saddrs) const
{
    return 0;
}

//应答成功
bool service_c::ok(acl::socket_stream *conn) const
{
    return false;
}

//应答错误 
bool service_c::error(acl::socket_stream *conn, short errnumb, char const *format, ...) const
{
    return false;
}
