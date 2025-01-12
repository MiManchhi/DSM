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
    case CMD_TRACKER_JOIN:
        result = join(conn,bodylen);
        break;
    case CMD_TRACKER_BEAT:
        result = beat(conn,bodylen);
        break;
    case CMD_TRACKER_SADDRS:
        result = saddrs(conn,bodylen);
        break;
    case CMD_ENCRYPT_JOIN:
        result = join_encrypt(conn, bodylen);
        break;
    case CMD_ENCRYPT_BEAT:
        result = beat_encrypt(conn, bodylen);
        break;
    case CMD_TRACKER_EADDRS:
        result = eaddrs(conn, bodylen);
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

//处理来自密钥协商服务器的加入包                    包体长度             
bool service_c::join_encrypt(acl::socket_stream* conn,long long bodylen) const
{
    //   |包体长度|命令|状态|encrypt_join_body_t|
    //   |   8   | 1  | 1  |                   |
    //检查包体长度
    long long expectedLength = sizeof(encrypt_join_body_t);
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
    encrypt_join_t ej;
    encrypt_join_body_t* ejb = (encrypt_join_body_t*)body;
    //版本
    strcpy(ej.ej_version,ejb->ejb_version);
    //组名
    strcpy(ej.ej_groupname,ejb->ejb_groupname);
    if(valid(ej.ej_groupname) != OK)
    {
        error(conn,-1,"valid groupname:%s",ej.ej_groupname);
        return false;
    }
    //主机名
    strcpy(ej.ej_hostname,ejb->ejb_hostname);
    //端口号
    ej.ej_port = ntos(ejb->ejb_port);
    if(!ej.ej_port)
    {
        error(conn,-1,"invalid port:%u",ej.ej_port);
        return false;
    }
    //启动时间
    ej.ej_stime = ntol(ejb->ejb_stime);
    //加入时间
    ej.ej_jtime = ntol(ejb->ejb_jtime);
    logger("encrypt join,version:%s, groupname:%s, hostname:%s, port:%u, stime:%s, jtime:%s",
    ej.ej_version,ej.ej_groupname,ej.ej_hostname,ej.ej_port,std::string(ctime(&ej.ej_stime)).c_str(),std::string(ctime(&ej.ej_jtime)).c_str());
    //将存储服务器加入组表
    if(join_encrypt(&ej,conn->get_peer()) != OK)
    {
        error(conn,-1,"join into groups fail");
        return false;
    }
    return ok(conn);
}

//处理来自密钥协商服务器的心跳包                    包体长度            
bool service_c::beat_encrypt(acl::socket_stream* conn,long long bodylen) const
{
    //   |包体长度|命令|状态|encrypt_beat_body_t|
    //   |   8   | 1  | 1  |                   |
    //检查包体长度
    long long expectedLength = sizeof(encrypt_beat_body_t);
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
    encrypt_beat_body_t* ebb = (encrypt_beat_body_t*)body;
    //组名
    char groupname[ENCRYPT_GROUPNAME_MAX+1];
    strcpy(groupname,ebb->ebb_groupname);
    //主机名
    char hostname[ENCRYPT_HOSTNAME_MAX+1];
    strcpy(hostname,ebb->ebb_hostname);
    logger("encrypt beat,groupname:%s, hostname:%s",groupname,hostname);
    //将存储服务器标记为活动
    if(beat_encrypt(groupname,hostname,conn->get_peer()) != OK)
    {
        error(conn,-1,"mark encrypt as active fail");
        return false;
    }
    return ok(conn);
}

//处理来自客户机的获取密钥协商服务器地址列表请求       包体长度           
bool service_c::eaddrs(acl::socket_stream* conn,long long bodylen) const
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
    if(eaddrs(conn,appid,userid) != OK)
        return false;
    return true;
}      

//处理来自客户机的获取组列表请求
bool service_c::groups(acl::socket_stream *conn) const
{
    //互斥锁加锁---->组表
    if((errno = pthread_mutex_lock(&g_mutex)))  //pthread_mutex_lock成功返回非零值，错误返回错误号
    {
        logger_error("call pthread_mutex_lock fail:%s",strerror(errno));
        return false;
    }
    //全组字符串
    acl::string group_all;
    group_all.format(
        "COUNT OF GROUPS:%lu\n",g_groups.size()  //组数
    );
    //遍历组表中的每一个组
    for(const auto& val:g_groups)
    {
        acl::string group_one;  //单组字符串  组名 存储服务器数  活动存储服务器数
        group_one.format("GROUPNAME:%s\n""COUNT OF STORAGES:%lu\n""COUNT OF ACTIVE STORAGE:%s\n",
        val.first.c_str(),val.second.size(),"%d");
        int act = 0;  //活动存储服务器数
        //遍历每个组的每一台存储服务器
        for(const auto& value:val.second)
        {
            acl::string storage;//存储服务器字符串：版本 主机名 IP：PORT 启动时间 加入时间 心跳时间 状态
            storage.format("VERSION:%s\n""HOSTNAME:%s\n""ADDRESS:%s:%u\n""STARTUP TIME:%s""JOIN TIME:%s""BEAT TIME:%s""STATUS:",
            value.si_version,value.si_hostname,value.si_addr,value.si_port,std::string(ctime(&value.si_stime)).c_str(),
            std::string(ctime(&value.si_jtime)).c_str(),std::string(ctime(&value.si_btime)).c_str());
            switch (value.si_status)
            {
            case STORAGE_STATUS_OFFLINE:
                storage += "OFFLINE";
                break;
            case STORAGE_STATUS_ONLINE:
                storage += "ONLINE";
                break;
            case STORAGE_STATUS_ACTIVE:
                storage += "ACTIVE";
                ++act;
                break;
            default:
                storage += "UNKNOWN";
                break;
            }
            group_one += storage + "\n";  //构造一个组的存储服务器
        }
        group_all += group_one.format(group_one,act); //构造全组的存储服务器
    }
    group_all = group_all.left(group_all.size() - 1); //去除多加的\n
    //互斥锁解锁
    if((errno = pthread_mutex_unlock(&g_mutex)))  //pthread_mutex_unlock成功返回非零值，错误返回错误号
    {
        logger_error("call pthread_mutex_unlock fail:%s",strerror(errno));
        return false;
    }
    //构造响应
    // |包体长度|命令|状态|组列表|
    // |  8    |  1 | 1 |包体长度|
    long long bodylen = group_all.size()+1;
    long long respondlen = HEADLEN + bodylen;
    char respond[respondlen] = {};
    llton(bodylen,respond); //包体长度转换为网络字节序后写到respond---->包体长度
    respond[BODYLEN_SIZE] = CMD_STORAGE_REPLY; //respond----->包体长度+命令
    respond[BODYLEN_SIZE+COMMAND_SIZE] = 0;  //respond----->包体长度+命令+状态
    strcpy(respond + HEADLEN,group_all.c_str());  //respond----->包体长度+命令+状态+包体
    //发送响应
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail:%s, respondlen:%lld, to:%s",acl::last_serror(),respondlen,conn->get_peer());
        return false;
    }
    return true;
}

//将存储服务器加入组表
int service_c::join(storage_join_t const *sj, char const *saddr) const
{
    //互斥锁加锁
    if((errno = pthread_mutex_lock(&g_mutex)))
    {
        logger_error("call pthread_mutex_lock fail:%s",strerror(errno));
        return ERROR;
    }
    //在组表中查找待加入存储服务器所隶属的组
    auto group = g_groups.find(sj->sj_groupname);
    if(group != g_groups.end())  //若找到该组
    {
        //遍历该组的存储服务器列表
        bool Isfind = false;
        for(auto& value:group->second)
        {
            //若待加入存储服务器在列表中
            if(!strcmp(value.si_hostname,sj->sj_hostname) && !strcmp(value.si_addr,saddr))
            {
                //更新记录
                strcpy(value.si_version,sj->sj_version); //版本号
                value.si_port = sj->sj_port;             //端口号
                value.si_stime = sj->sj_stime;           //启动时间
                value.si_jtime = sj->sj_jtime;           //加入时间
                value.si_btime = sj->sj_jtime;           //心跳时间
                value.si_status = STORAGE_STATUS_ONLINE; //状态
                Isfind = true;
                break;
            }
        }
        //若不在存储服务器列表中
        if(!Isfind)
        {
            //将待加入存储服务器加入该列表
            storage_info_t si;
			strcpy(si.si_version,  sj->sj_version);  // 版本
			strcpy(si.si_hostname, sj->sj_hostname); // 主机名
			strcpy(si.si_addr,     saddr);           // IP地址
			si.si_port   = sj->sj_port;              // 端口号
			si.si_stime  = sj->sj_stime;             // 启动时间
			si.si_jtime  = sj->sj_jtime;             // 加入时间
			si.si_btime  = sj->sj_jtime;             // 心跳时间
			si.si_status = STORAGE_STATUS_ONLINE;    // 状态
			group->second.push_back(si);
        }
    }
    else  //若没有找到该组
    {
        //将待加入存储服务器所隶属的组加入组表
        g_groups[sj->sj_groupname] = std::list<storage_info_t>();
        //将待加入存储服务器加入该组的存储服务器列表
        storage_info_t si;
			strcpy(si.si_version,  sj->sj_version);  // 版本
			strcpy(si.si_hostname, sj->sj_hostname); // 主机名
			strcpy(si.si_addr,     saddr);           // IP地址
			si.si_port   = sj->sj_port;              // 端口号
			si.si_stime  = sj->sj_stime;             // 启动时间
			si.si_jtime  = sj->sj_jtime;             // 加入时间
			si.si_btime  = sj->sj_jtime;             // 心跳时间
			si.si_status = STORAGE_STATUS_ONLINE;    // 状态
			g_groups[sj->sj_groupname].push_back(si);
    }
    //互斥锁解锁
    if((errno = pthread_mutex_unlock(&g_mutex)))
    {
        logger_error("call pthread_mutex_unlock fail:%s",strerror(errno));
        return ERROR;
    }
    return OK;
}

//将存储服务器标记为活动
int service_c::beat(char const *groupname, char const *hostname, char const *saddr) const
{
    //互斥锁加锁
    if((errno = pthread_mutex_lock(&g_mutex)))
    {
        logger_error("call pthread_mutex_lock fail:%s",strerror(errno));
        return ERROR;
    }
    int result = OK;
    //在组表中查找待加入存储服务器所隶属的组
    auto group = g_groups.find(groupname);
    if(group != g_groups.end())  //若找到该组
    {
        //遍历该组的存储服务器列表
        bool Isfind = false;
        for(auto& value:group->second)
        {
            //若待加入存储服务器在列表中
            if(!strcmp(value.si_hostname,hostname) && !strcmp(value.si_addr,saddr))
            {
                //更新记录
                value.si_btime = time(NULL);             //心跳时间
                value.si_status = STORAGE_STATUS_ACTIVE; //状态
                Isfind = true;
                break;
            }
        }
        //若不在存储服务器列表中
        if(!Isfind)
        {
            logger_error("storage not found,groupname:%s, hostname:%s, saddr:%s",groupname,hostname,saddr);
            result = ERROR;
        }
    }
    else  //若没有找到该组
    {
        logger_error("group ont found,groupname:%s",groupname);
        result = ERROR;
    }
    //互斥锁解锁
    if((errno = pthread_mutex_unlock(&g_mutex)))
    {
        logger_error("call pthread_mutex_unlock fail:%s",strerror(errno));
        return ERROR;
    }
    return result;
}

//响应客户机存储服务器地址列表
int service_c::saddrs(acl::socket_stream *conn, char const *appid, char const *userid) const
{
    //应用ID是否合法
    if(valid(appid) != OK)
    {
        error(conn,-1,"invalid appid:%s",appid);
        return ERROR;
    }
    //应用ID是否存在
    if(std::find(g_appids.begin(),g_appids.end(),appid) == g_appids.end())
    {
        error(conn,-1,"unknown appid:%s",appid);
        return ERROR;
    }
    //根据用户ID获取对应组名
    std::string groupname;
    if(group_of_user(appid,userid,groupname) != OK)
    {
        error(conn,-1,"get groupname fail");
        return ERROR;
    }
    //根据组名获取存储服务器地址列表
    std::string saddrs;
    if(saddrs_of_group(groupname.c_str(),saddrs) != OK)
    {
        error(conn,-1,"get storage address fail");
        return ERROR;
    }
    logger("appid:%s, userid:%s, groupname:%s, saddrs:%s",appid,userid,groupname.c_str(),saddrs.c_str());
    //构造响应
    //|包体长度|命令|状态|组名+存储服务器地址列表|
    //|   8   | 1  | 1  |      包体长度        |
    long long bodylen = STORAGE_GROUPNAME_MAX + 1 + saddrs.size() + 1;
    long long respondlen = HEADLEN + bodylen;
    char respond[respondlen] = {};
    llton(bodylen,respond);
    respond[BODYLEN_SIZE] = CMD_TRACKER_REPLY;
    respond[BODYLEN_SIZE+COMMAND_SIZE] = 0;
    strncpy(respond+HEADLEN,groupname.c_str(),STORAGE_GROUPNAME_MAX);
    strcpy(respond+HEADLEN+STORAGE_GROUPNAME_MAX+1,saddrs.c_str());
    //发送响应
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail:%s, respondlen:%lld, to:%s",acl::last_serror(),respondlen,conn->get_peer());
        return ERROR;
    }
    return OK;
}

//将密钥协商服务器加入组表  密钥协商服务器加入信息结构体   ip地址
int service_c::join_encrypt(encrypt_join_t const* ej,char const* eaddr) const
{
    //互斥锁加锁
    if((errno = pthread_mutex_lock(&g_encrypt_mutex)))
    {
        logger_error("call pthread_mutex_lock fail:%s",strerror(errno));
        return ERROR;
    }
    //在组表中查找待加入密钥协商服务器所隶属的组
    auto group = g_encrypt_groups.find(ej->ej_groupname);
    if(group != g_encrypt_groups.end())  //若找到该组
    {
        //遍历该组的密钥协商服务器列表
        bool Isfind = false;
        for(auto& value:group->second)
        {
            //若待加入密钥协商服务器在列表中
            if(!strcmp(value.ei_hostname,ej->ej_hostname) && !strcmp(value.ei_addr,eaddr))
            {
                //更新记录
                strcpy(value.ei_version,ej->ej_version); //版本号
                value.ei_port = ej->ej_port;             //端口号
                value.ei_stime = ej->ej_stime;           //启动时间
                value.ei_jtime = ej->ej_jtime;           //加入时间
                value.ei_btime = ej->ej_jtime;           //心跳时间
                value.ei_status = ENCRYPT_STATUS_ONLINE; //状态
                Isfind = true;
                break;
            }
        }
        //若不在密钥协商服务器列表中
        if(!Isfind)
        {
            //将待加入密钥协商服务器加入该列表
            encrypt_info_t ei;
			strcpy(ei.ei_version,  ej->ej_version);  // 版本
			strcpy(ei.ei_hostname, ej->ej_hostname); // 主机名
			strcpy(ei.ei_addr,     eaddr);           // IP地址
			ei.ei_port   = ej->ej_port;              // 端口号
			ei.ei_stime  = ej->ej_stime;             // 启动时间
			ei.ei_jtime  = ej->ej_jtime;             // 加入时间
			ei.ei_btime  = ej->ej_jtime;             // 心跳时间
			ei.ei_status = ENCRYPT_STATUS_ONLINE;    // 状态
			group->second.push_back(ei);
        }
    }
    else  //若没有找到该组
    {
        //将待加入密钥协商服务器所隶属的组加入组表
        g_encrypt_groups[ej->ej_groupname] = std::list<encrypt_info_t>();
        //将待加入密钥协商服务器加入该组的密钥协商服务器列表
        encrypt_info_t ei;
			strcpy(ei.ei_version,  ej->ej_version);  // 版本
			strcpy(ei.ei_hostname, ej->ej_hostname); // 主机名
			strcpy(ei.ei_addr,     eaddr);           // IP地址
			ei.ei_port   = ej->ej_port;              // 端口号
			ei.ei_stime  = ej->ej_stime;             // 启动时间
			ei.ei_jtime  = ej->ej_jtime;             // 加入时间
			ei.ei_btime  = ej->ej_jtime;             // 心跳时间
			ei.ei_status = ENCRYPT_STATUS_ONLINE;    // 状态
			g_encrypt_groups[ej->ej_groupname].push_back(ei);
    }
    //互斥锁解锁
    if((errno = pthread_mutex_unlock(&g_encrypt_mutex)))
    {
        logger_error("call pthread_mutex_unlock fail:%s",strerror(errno));
        return ERROR;
    }
    return OK;
}

//将密钥协商服务器标记为活动  组名                 主机名               ip
int service_c::beat_encrypt(char const* groupname,char const* hostname,char const* eaddr) const
{
    //互斥锁加锁
    if((errno = pthread_mutex_lock(&g_encrypt_mutex)))
    {
        logger_error("call pthread_mutex_lock fail:%s",strerror(errno));
        return ERROR;
    }
    int result = OK;
    //在组表中查找待加入密钥协商服务器所隶属的组
    auto group = g_encrypt_groups.find(groupname);
    if(group != g_encrypt_groups.end())  //若找到该组
    {
        //遍历该组的密钥协商服务器列表
        bool Isfind = false;
        for(auto& value:group->second)
        {
            //若待加入密钥协商服务器在列表中
            if(!strcmp(value.ei_hostname,hostname) && !strcmp(value.ei_addr,eaddr))
            {
                //更新记录
                value.ei_btime = time(NULL);             //心跳时间
                value.ei_status = ENCRYPT_STATUS_ACTIVE; //状态
                Isfind = true;
                break;
            }
        }
        //若不在密钥协商服务器列表中
        if(!Isfind)
        {
            logger_error("encrypt not found,groupname:%s, hostname:%s, eaddr:%s",groupname,hostname,eaddr);
            result = ERROR;
        }
    }
    else  //若没有找到该组
    {
        logger_error("group ont found,groupname:%s",groupname);
        result = ERROR;
    }
    //互斥锁解锁
    if((errno = pthread_mutex_unlock(&g_encrypt_mutex)))
    {
        logger_error("call pthread_mutex_unlock fail:%s",strerror(errno));
        return ERROR;
    }
    return result;
}

//响应客户机密钥协商服务器地址列表
int service_c::eaddrs(acl::socket_stream* conn,char const* appid,char const* userid) const
{
    //应用ID是否合法
    if(valid(appid) != OK)
    {
        error(conn,-1,"invalid appid:%s",appid);
        return ERROR;
    }
    //应用ID是否存在
    if(std::find(g_appids.begin(),g_appids.end(),appid) == g_appids.end())
    {
        error(conn,-1,"unknown appid:%s",appid);
        return ERROR;
    }
    //根据用户ID获取对应组名
    std::string groupname;
    if(group_of_user(appid,userid,groupname) != OK)
    {
        error(conn,-1,"get groupname fail");
        return ERROR;
    }
    //根据组名获取密钥协商服务器地址列表
    std::string eaddrs;
    if(eaddrs_of_group(groupname.c_str(),eaddrs) != OK)
    {
        error(conn,-1,"get encrypt address fail");
        return ERROR;
    }
    logger("appid:%s, userid:%s, groupname:%s, eaddrs:%s",appid,userid,groupname.c_str(),eaddrs.c_str());
    //构造响应
    //|包体长度|命令|状态|组名+密钥协商服务器地址列表|
    //|   8   | 1  | 1  |       包体长度          |
    long long bodylen = ENCRYPT_GROUPNAME_MAX + 1 + eaddrs.size() + 1;
    long long respondlen = HEADLEN + bodylen;
    char respond[respondlen] = {};
    llton(bodylen,respond);
    respond[BODYLEN_SIZE] = CMD_TRACKER_REPLY;
    respond[BODYLEN_SIZE+COMMAND_SIZE] = 0;
    strncpy(respond+HEADLEN,groupname.c_str(),ENCRYPT_GROUPNAME_MAX);
    strcpy(respond+HEADLEN+ENCRYPT_GROUPNAME_MAX+1,eaddrs.c_str());
    //发送响应
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail:%s, respondlen:%lld, to:%s",acl::last_serror(),respondlen,conn->get_peer());
        return ERROR;
    }
    return OK;
}

//根据用户ID获取其对应的组名
int service_c::group_of_user(char const *appid, char const *userid, std::string &groupname) const
{
    //数据库访问对象
    db_c db;
    //连接数据库
    if(db.connect() != OK)
    {
        return ERROR;
    }
    //根据用户ID获取对应的组名
    if(db.getGroupName(userid,groupname) != OK)
    {
        return ERROR;
    }
    //组名为空表示该用户没有组，为其随机分配一个
    if(groupname.empty())
    {
        logger("groupname is empty,appid:%s,userid:%s, allocate one",appid,userid);
        //获取全部组名
        std::vector<std::string> groupnames;
        if(db.getAllGroupname(groupnames) != OK)
        {
            return ERROR;
        }
        if(groupnames.empty())
        {
            logger_error("groupnames is empty,appid:%s,userid:%s",appid,userid);
            return ERROR;
        }
        //随机抽取组名
        srand(time(NULL));
        groupname = groupnames[rand()%groupnames.size()];
        //设置用户ID和组名的对应关系
        if(db.setIDGroupname(appid,userid,groupname.c_str()) != OK)
        {
            return ERROR;
        }
    }
    return OK;
}

//根据组名获取存储服务器地址列表
int service_c::saddrs_of_group(char const *groupname, std::string &saddrs) const
{
    //互斥锁加锁
    if((errno = pthread_mutex_lock(&g_mutex)))
    {
        logger_error("call pthread_mutex_lock fail:%s",strerror(errno));
        return ERROR;
    }
    //根据组名查找组
    int result = OK;
    const auto group = g_groups.find(groupname);
    //若找到该组
    if(group != g_groups.end())
    {
        //若该组存储服务器列表非空
        if(!group->second.empty())
        {
            //在该组的存储服务器列表中，从随机位置开始最多抽取三台处于活动状态的存储服务器
            srand(time(NULL));
            int nsis = group->second.size();
            int nrand = rand() % nsis;
            auto iter = group->second.begin();
            int nacts = 0;
            for (int i = 0; i < nsis+nrand; ++i,++iter)
            {
                if(iter == group->second.end())
                    iter = group->second.begin();
                logger("i:%d, nrand:%d, addr:%s, port:%u, status:%d",i,nrand,iter->si_addr,iter->si_port,iter->si_status);
                if(i >= nrand && iter->si_status == STORAGE_STATUS_ACTIVE)
                {
                    char saddr[256];
                    sprintf(saddr,"%s:%d",iter->si_addr,iter->si_port);
                    saddrs += saddr;
                    saddrs += ";";
                    if(++nacts >= 3)
                        break;
                }
            }
            //若没有处于活动状态的存储服务器
            if(!nacts)
            {
                logger_error("no active storage in group %s",groupname);
                result = ERROR;
            }
        }
        //若存储服务器列表为空
        else
        {
            logger_error("no storage in group %s",groupname);
            result = ERROR;
        }
    }
    //若没有该组
    else
    {
        logger_error("not found group %s",groupname);
        result = ERROR;
    }
    //互斥锁解锁
    if((errno = pthread_mutex_unlock(&g_mutex)))
    {
        logger_error("call pthread_mutex_unlock fail:%s",strerror(errno));
        return ERROR;
    }
    return result;
}

//根据组名获取密钥协商服务器地址列表
int service_c::eaddrs_of_group(char const* groupname,std::string& eaddrs) const
{
    //互斥锁加锁
    if((errno = pthread_mutex_lock(&g_encrypt_mutex)))
    {
        logger_error("call pthread_mutex_lock fail:%s",strerror(errno));
        return ERROR;
    }
    //根据组名查找组
    int result = OK;
    const auto group = g_encrypt_groups.find(groupname);
    //若找到该组
    if(group != g_encrypt_groups.end())
    {
        //若该组密钥协商服务器列表非空
        if(!group->second.empty())
        {
            //在该组的密钥协商服务器列表中，从随机位置开始最多抽取三台处于活动状态的密钥协商服务器
            srand(time(NULL));
            int nsis = group->second.size();
            int nrand = rand() % nsis;
            auto iter = group->second.begin();
            int nacts = 0;
            for (int i = 0; i < nsis+nrand; ++i,++iter)
            {
                if(iter == group->second.end())
                    iter = group->second.begin();
                logger("i:%d, nrand:%d, addr:%s, port:%u, status:%d",i,nrand,iter->ei_addr,iter->ei_port,iter->ei_status);
                if(i >= nrand && iter->ei_status == ENCRYPT_STATUS_ACTIVE)
                {
                    char eaddr[256];
                    sprintf(eaddr,"%s:%d",iter->ei_addr,iter->ei_port);
                    eaddrs += eaddr;
                    eaddrs += ";";
                    if(++nacts >= 3)
                        break;
                }
            }
            //若没有处于活动状态的密钥协商服务器
            if(!nacts)
            {
                logger_error("no active encrypt in group %s",groupname);
                result = ERROR;
            }
        }
        //若密钥协商服务器列表为空
        else
        {
            logger_error("no encrypt in group %s",groupname);
            result = ERROR;
        }
    }
    //若没有该组
    else
    {
        logger_error("not found group %s",groupname);
        result = ERROR;
    }
    //互斥锁解锁
    if((errno = pthread_mutex_unlock(&g_encrypt_mutex)))
    {
        logger_error("call pthread_mutex_unlock fail:%s",strerror(errno));
        return ERROR;
    }
    return result;
}

//应答成功
bool service_c::ok(acl::socket_stream *conn) const
{
    // |包体长度|命令|状态|
    // |   8   |  1 |  1 |
    //构造响应
    long long bodylen = 0;
    long long respondlen = HEADLEN+bodylen;
    char respond[respondlen] = {};
    llton(bodylen,respond);
    respond[BODYLEN_SIZE] = CMD_TRACKER_REPLY;
    respond[BODYLEN_SIZE+COMMAND_SIZE] = 0;
    //发送响应
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail:%s, respondlen:%lld, to:%s",acl::last_serror(),respondlen,conn->get_peer());
        return false;
    }
    return true;
}

//应答错误 
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
    respond[BODYLEN_SIZE] = CMD_TRACKER_REPLY;
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
