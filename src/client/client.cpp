//客户机
//实现客户机类
#include <acl-lib/acl/lib_acl.h>
#include "types.h"
#include "util.h"
#include "conn.h"
#include "pool_manager.h"
#include "client.h"
#include "pool.h"

//套接字通信最大错误数
const int MAX_SOCKERRS = 10;

//静态成员变量只能在类外初始化
//线程池管理类
acl::connect_manager *client_c::s_pool_manager = nullptr;
// 跟踪服务器地址列表
std::vector<std::string> client_c::s_taddrs;
// 存储服务器连接池容量
int client_c::s_scount = 8;
// 密钥协商服务器连接池容量
int client_c::s_ecount = 8;

client_c::client_c()
{

}
client_c::~client_c()
{

}

//静态成员初始化   跟踪服务器地址列表 跟踪服务器连接池容量 存储服务器连接池容量
int client_c::init(char const* taddrs,int tcount /*= 16*/,int scount /*= 8*/)
{
    if(s_pool_manager)
        return OK;
    //检查跟踪服务器地址列表
    if(!taddrs || !strlen(taddrs))
    {
        logger_error("tracker server addresses is null");
        return ERROR;
    }
    splitstring(taddrs, s_taddrs);
    if(s_taddrs.empty())
    {
        logger_error("tracker server addresses is empty");
        return ERROR;
    }
    // 检查跟踪服务器最大连接数
    if(tcount <= 0)
    {
        logger_error("tracker server connect pool connected counts <= %d", tcount);
        return ERROR;
    }
    //检查存储服务器最大连接数
    if(scount <= 0)
    {
        logger_error("storage server connect pool connected counts <= %d", scount);
        return ERROR;
    }
    s_scount = scount;
    // 创建连接池管理对象
    if(!(s_pool_manager = new pool_manager_c()))
    {
        logger_error("create connect pool manager fail :%s",acl::last_serror());
        return ERROR;
    }
    // 连接池对象初始化
    s_pool_manager->init(NULL, taddrs, tcount);
    return OK;
}

// 静态成员销毁
void client_c::deinit()
{
    if(s_pool_manager)
    {
        delete s_pool_manager;
        s_pool_manager = NULL;
    }
    s_taddrs.clear();
}

//从跟踪服务器获取存储服务器地址列表
int client_c::saddrs(char const *appid, char const *userid, char const *fileid, std::string &saddrs)
{
    //检查跟踪服务器地址列表
    if(s_taddrs.empty())
    {
        logger_error("tracker server addresses is empty");
        return ERROR;
    }
    int result = ERROR;
    // 生成有限随机数
    srand(time(NULL));
    int ntaddrs = s_taddrs.size();
    int nrand = rand() % ntaddrs;
    // 随机抽取一个跟踪服务器地址
    for (int i = 0; i < ntaddrs; ++i)
    {
        char const *taddr = s_taddrs[i].c_str();
        nrand = (nrand + 1) % ntaddrs;
        // 获取跟踪服务器连接池
        pool_c *tpool = dynamic_cast<pool_c *>(s_pool_manager->get(taddr));
        if(!tpool)
        {
            logger_warn("tracker connection pool is null,taddr:%s", taddr);
            continue;  //尝试使用下一个地址
        }
        for (int j = 0; j < MAX_SOCKERRS; ++j)
        {
            // 获取一个跟踪服务器连接
            conn_c *tconn = dynamic_cast<conn_c *>(tpool->peek());
            if(!tconn)
            {
                logger_warn("tracker connection is null, taddr: %s",taddr);
                break; //连接池里取连接失败，退出本层循环重新获取连接池
            }
            // 从跟踪服务器获取存储服务器地址列表
            logger("get storage server addresses attempt #%d, appid:%s, userid:%s, fileid:%s", 
            i + 1, appid, userid, fileid);
            result = tconn->saddrs(appid, userid, fileid, saddrs);
            if(result == SOCKET_ERROR)
            {
                logger_warn("get storage addresses fail :%s", tconn->errdesc());
                tpool->put(tconn, false);
            }
            else
            {
                if(result == OK)
                {
                    tpool->put(tconn, true);
                }
                else
                {
                    logger_warn("get storage addresses fail :%s", tconn->errdesc());
                    tpool->put(tconn, false);
                }
                return result;
            }
        }
    }
    return result;
}

//从跟踪服务器获取密钥协商服务器地址列表
int client_c::eaddrs(char const *appid, char const *userid, char const *fileid, std::string &eaddrs)
{
    //检查密钥协商服务器地址列表
    if(s_taddrs.empty())
    {
        logger_error("tracker server addresses is empty");
        return ERROR;
    }
    int result = ERROR;
    // 生成有限随机数
    srand(time(NULL));
    int ntaddrs = s_taddrs.size();
    int nrand = rand() % ntaddrs;
    // 随机抽取一个跟踪服务器地址
    for (int i = 0; i < ntaddrs; ++i)
    {
        char const *taddr = s_taddrs[i].c_str();
        nrand = (nrand + 1) % ntaddrs;
        // 获取跟踪服务器连接池
        pool_c *tpool = dynamic_cast<pool_c *>(s_pool_manager->get(taddr));
        if(!tpool)
        {
            logger_warn("tracker connection pool is null,taddr:%s", taddr);
            continue;  //尝试使用下一个地址
        }
        for (int j = 0; j < MAX_SOCKERRS; ++j)
        {
            // 获取一个跟踪服务器连接
            conn_c *tconn = dynamic_cast<conn_c *>(tpool->peek());
            if(!tconn)
            {
                logger_warn("tracker connection is null, taddr: %s",taddr);
                break; //连接池里取连接失败，退出本层循环重新获取连接池
            }
            // 从跟踪服务器获取密钥协商服务器地址列表
            logger("get encrypt server addresses attempt #%d, appid:%s, userid:%s, fileid:%s", 
            i + 1, appid, userid, fileid);
            result = tconn->eaddrs(appid, userid, fileid, eaddrs);
            if(result == SOCKET_ERROR)
            {
                logger_warn("get encrypt addresses fail :%s", tconn->errdesc());
                tpool->put(tconn, false);
            }
            else
            {
                if(result == OK)
                {
                    tpool->put(tconn, true);
                }
                else
                {
                    logger_warn("get encrypt addresses fail :%s", tconn->errdesc());
                    tpool->put(tconn, false);
                }
                return result;
            }
        }
    }
    return result;
}

// 从跟踪服务器获取组列表
int client_c::groups(std::string &groups)
{
     //检查跟踪服务器地址列表
    if(s_taddrs.empty())
    {
        logger_error("tracker server addresses is empty");
        return ERROR;
    }
    int result = ERROR;
    // 生成有限随机数
    srand(time(NULL));
    int ntaddrs = s_taddrs.size();
    int nrand = rand() % ntaddrs;
    // 随机抽取一个跟踪服务器地址
    for (int i = 0; i < ntaddrs; ++i)
    {
        char const *taddr = s_taddrs[i].c_str();
        nrand = (nrand + 1) % ntaddrs;
        // 获取跟踪服务器连接池
        pool_c *tpool = dynamic_cast<pool_c *>(s_pool_manager->get(taddr));
        if(!tpool)
        {
            logger_warn("tracker connection pool is null,taddr:%s", taddr);
            continue;  //尝试使用下一个地址
        }
        for (int j = 0; j < MAX_SOCKERRS; ++j)
        {
            // 获取一个跟踪服务器连接
            conn_c *tconn = dynamic_cast<conn_c *>(tpool->peek());
            if(!tconn)
            {
                logger_warn("tracker connection is null, taddr: %s",taddr);
                break; //连接池里取连接失败，退出本层循环重新获取连接池
            }
            // 从跟踪服务器获取存储服务器地址列表
            logger("get groups attempt #%d", i + 1);
            result = tconn->groups(groups);
            if(result == SOCKET_ERROR)
            {
                logger_warn("get groups fail :%s", tconn->errdesc());
                tpool->put(tconn, false);
            }
            else
            {
                if(result == OK)
                {
                    tpool->put(tconn, true);
                }
                else
                {
                    logger_warn("get groups fail :%s", tconn->errdesc());
                    tpool->put(tconn, false);
                }
                return result;
            }
        }
    }
    return result;
}

// 向存储服务器上传文件
int client_c::upload(char const *appid, char const *userid, char const *fileid, char const *filedata, long long filesize)
{
    //检查参数
    if(!appid || !strlen(appid))
    {
        logger_error("appid is null");
        return ERROR;
    }
    if(!userid || !strlen(userid))
    {
        logger_error("userid is null");
        return ERROR;
    }
    if(!fileid || !strlen(fileid))
    {
        logger_error("fileid is null");
        return ERROR;
    }
    if(!filedata || !filesize)
    {
        logger_error("filedata is null");
        return ERROR;
    }
    //从跟踪服务器获取存储服务器地址列表
    int result;
    std::string ssaddrs;
    if((result = saddrs(appid,userid,fileid,ssaddrs)) != OK)
    {
        logger_error("storage server addresses is null");
        return result;
    }
    std::vector<std::string> vsaddrs;
    splitstring(ssaddrs.c_str(), vsaddrs);
    if(vsaddrs.empty())
    {
        logger_error("storage server addresses is empty");
        return ERROR;
    }
    result = ERROR;
    // 遍历存储服务器地址列表尝试创建存储服务器连接池
    for(const auto& saddr : vsaddrs)
    {
        //尝试创建存储服务器连接池
        pool_c *spool = dynamic_cast<pool_c *>(s_pool_manager->get(saddr.c_str()));
        if(!spool)
        {
            //如果当前地址没有连接池也可能是没有创建，创建后再次获取连接池
            s_pool_manager->set(saddr.c_str(), s_scount);
            if(!(spool = dynamic_cast<pool_c*>(s_pool_manager->get(saddr.c_str()))))
            {
                logger_warn("storage server connection pool is null saddr:%s",saddr.c_str());
                continue;
            }
        }
        // 获取存储服务器连接
        for (int i = 0; i < MAX_SOCKERRS; ++i)
        {
            conn_c *sconn = dynamic_cast<conn_c *>(spool->peek());
            if(!sconn)
            {
                logger_error("storage server connection is null saddr:%s",saddr.c_str());
                break;
            }
            // 向存储服务器上传文件
            logger("upload attempt #%d, saddr:%s, appid:%s, userid:%s, fileid:%s", 
            i + 1, saddr.c_str(), appid, userid, fileid);
            result = sconn->upload(appid,userid,fileid,filedata,filesize);
            if(result == SOCKET_ERROR)
            {
                logger_warn("upload file fail:%s",sconn->errdesc());
                spool->put(sconn, false);
            }
            else
            {
                if(result == OK)
                    spool->put(sconn, true);
                else
                {
                    logger_error("upload file fail:%s", sconn->errdesc());
                    spool->put(sconn, false);
                }
                return result;
            }
        }
    }
    return result;
}

// 向存储服务器上传文件
int client_c::upload(char const *appid, char const *userid, char const *fileid, char const *filepath)
{
    //检查参数
    if(!appid || !strlen(appid))
    {
        logger_error("appid is null");
        return ERROR;
    }
    if(!userid || !strlen(userid))
    {
        logger_error("userid is null");
        return ERROR;
    }
    if(!fileid || !strlen(fileid))
    {
        logger_error("fileid is null");
        return ERROR;
    }
    if(!filepath || !strlen(filepath))
    {
        logger_error("filepath is null");
        return ERROR;
    }
    //从跟踪服务器获取存储服务器地址列表
    int result;
    std::string ssaddrs;
    if((result = saddrs(appid,userid,fileid,ssaddrs)) != OK)
    {
        logger_error("storage server addresses is null");
        return result;
    }
    std::vector<std::string> vsaddrs;
    splitstring(ssaddrs.c_str(), vsaddrs);
    if(vsaddrs.empty())
    {
        logger_error("storage server addresses is empty");
        return ERROR;
    }
    result = ERROR;
    // 遍历存储服务器地址列表尝试创建存储服务器连接池
    for(const auto& saddr : vsaddrs)
    {
        //尝试创建存储服务器连接池
        pool_c *spool = dynamic_cast<pool_c *>(s_pool_manager->get(saddr.c_str()));
        if(!spool)
        {
            //如果当前地址没有连接池也可能是没有创建，创建后再次获取连接池
            s_pool_manager->set(saddr.c_str(), s_scount);
            if(!(spool = dynamic_cast<pool_c*>(s_pool_manager->get(saddr.c_str()))))
            {
                logger_warn("storage server connection pool is null saddr:%s",saddr.c_str());
                continue;
            }
        }
        // 获取存储服务器连接
        for (int i = 0; i < MAX_SOCKERRS; ++i)
        {
            conn_c *sconn = dynamic_cast<conn_c *>(spool->peek());
            if(!sconn)
            {
                logger_error("storage server connection is null saddr:%s",saddr.c_str());
                break;
            }
            // 向存储服务器上传文件
            logger("upload attempt #%d, saddr:%s, appid:%s, userid:%s, fileid:%s, filepath:%s", 
            i + 1, saddr.c_str(), appid, userid, fileid, filepath);
            result = sconn->upload(appid, userid, fileid, filepath);

            if(result == SOCKET_ERROR)
            {
                logger_warn("upload file fail:%s",sconn->errdesc());
                spool->put(sconn, false);
            }
            else
            {
                if(result == OK)
                    spool->put(sconn, true);
                else
                {
                    logger_error("upload file fail:%s", sconn->errdesc());
                    spool->put(sconn, false);
                }
                return result;
            }
        }
    }
    return result;
}

// 向存储服务器加密上传文件
int client_c::enupload(char const *appid, char const *userid, char const *fileid, char const *filedata, long long filesize)
{
    return 0;
}
// 向存储服务器加密上传文件
int client_c::enupload(char const *appid, char const *userid, char const *fileid, char const *filepath)
{
    return 0;
}

// 向存储服务器询问文件大小
int client_c::filesize(char const *appid, char const *userid, char const *fileid, long long &filesize)
{
    //检查参数
    if(!appid || !strlen(appid))
    {
        logger_error("appid is null");
        return ERROR;
    }
    if(!userid || !strlen(userid))
    {
        logger_error("userid is null");
        return ERROR;
    }
    if(!fileid || !strlen(fileid))
    {
        logger_error("fileid is null");
        return ERROR;
    }
    //从跟踪服务器获取存储服务器地址列表
    int result;
    std::string ssaddrs;
    if((result = saddrs(appid,userid,fileid,ssaddrs)) != OK)
    {
        logger_error("storage server addresses is null");
        return result;
    }
    std::vector<std::string> vsaddrs;
    splitstring(ssaddrs.c_str(), vsaddrs);
    if(vsaddrs.empty())
    {
        logger_error("storage server addresses is empty");
        return ERROR;
    }
    result = ERROR;
    // 遍历存储服务器地址列表尝试创建存储服务器连接池
    for(const auto& saddr : vsaddrs)
    {
        //尝试创建存储服务器连接池
        pool_c *spool = dynamic_cast<pool_c *>(s_pool_manager->get(saddr.c_str()));
        if(!spool)
        {
            //如果当前地址没有连接池也可能是没有创建，创建后再次获取连接池
            s_pool_manager->set(saddr.c_str(), s_scount);
            if(!(spool = dynamic_cast<pool_c*>(s_pool_manager->get(saddr.c_str()))))
            {
                logger_warn("storage server connection pool is null saddr:%s",saddr.c_str());
                continue;
            }
        }
        // 获取存储服务器连接
        for (int i = 0; i < MAX_SOCKERRS; ++i)
        {
            conn_c *sconn = dynamic_cast<conn_c *>(spool->peek());
            if(!sconn)
            {
                logger_error("storage server connection is null saddr:%s",saddr.c_str());
                break;
            }
            // 向存储服务器询问文件大小
            logger("requry filesize #%d, saddr:%s, appid:%s, userid:%s, fileid:%s", 
            i + 1, saddr.c_str(), appid, userid, fileid);
            result = sconn->filesize(appid, userid, fileid, filesize);
            if (result == SOCKET_ERROR)
            {
                logger_warn("get file size fail:%s",sconn->errdesc());
                spool->put(sconn, false);
            }
            else
            {
                if(result == OK)
                    spool->put(sconn, true);
                else
                {
                    logger_error("get file size fail:%s", sconn->errdesc());
                    spool->put(sconn, false);
                }
                return result;
            }
        }
    }
    return result;
}

// 从存储服务器下载文件
int client_c::download(char const *appid, char const *userid, char const *fileid, long long offset, long long size, char **filedata, long long &filesize)
{   
    //检查参数
    if(!appid || !strlen(appid))
    {
        logger_error("appid is null");
        return ERROR;
    }
    if(!userid || !strlen(userid))
    {
        logger_error("userid is null");
        return ERROR;
    }
    if(!fileid || !strlen(fileid))
    {
        logger_error("fileid is null");
        return ERROR;
    }
    //从跟踪服务器获取存储服务器地址列表
    int result;
    std::string ssaddrs;
    if((result = saddrs(appid,userid,fileid,ssaddrs)) != OK)
    {
        logger_error("storage server addresses is null");
        return result;
    }
    std::vector<std::string> vsaddrs;
    splitstring(ssaddrs.c_str(), vsaddrs);
    if(vsaddrs.empty())
    {
        logger_error("storage server addresses is empty");
        return ERROR;
    }
    result = ERROR;
    // 遍历存储服务器地址列表尝试创建存储服务器连接池
    for(const auto& saddr : vsaddrs)
    {
        //尝试创建存储服务器连接池
        pool_c *spool = dynamic_cast<pool_c *>(s_pool_manager->get(saddr.c_str()));
        if(!spool)
        {
            //如果当前地址没有连接池也可能是没有创建，创建后再次获取连接池
            s_pool_manager->set(saddr.c_str(), s_scount);
            if(!(spool = dynamic_cast<pool_c*>(s_pool_manager->get(saddr.c_str()))))
            {
                logger_warn("storage server connection pool is null saddr:%s",saddr.c_str());
                continue;
            }
        }
        // 获取存储服务器连接
        for (int i = 0; i < MAX_SOCKERRS; ++i)
        {
            conn_c *sconn = dynamic_cast<conn_c *>(spool->peek());
            if(!sconn)
            {
                logger_error("storage server connection is null saddr:%s",saddr.c_str());
                break;
            }
            // 从存储服务器下载文件
            logger("download file #%d, saddr:%s, appid:%s, userid:%s, fileid:%s, offset:%lld, size:%lld", 
            i + 1, saddr.c_str(), appid, userid, fileid, offset, size);
            result = sconn->download(appid,userid,fileid,offset,size,filedata,filesize);
            if (result == SOCKET_ERROR)
            {
                logger_warn("download file fail:%s",sconn->errdesc());
                spool->put(sconn, false);
            }
            else
            {
                if(result == OK)
                    spool->put(sconn, true);
                else
                {
                    logger_error("download file fail:%s", sconn->errdesc());
                    spool->put(sconn, false);
                }
                return result;
            }
        }
    }
    return result;
}

// 从存储服务器加密下载文件
int client_c::endownload(char const *appid, char const *userid, char const *fileid, long long offset, long long size, char **filedata, long long &filesize)
{
    return 0;
}

// 删除存储服务器上的文件
int client_c::del(char const *appid, char const *userid, char const *fileid)
{
    //检查参数
    if(!appid || !strlen(appid))
    {
        logger_error("appid is null");
        return ERROR;
    }
    if(!userid || !strlen(userid))
    {
        logger_error("userid is null");
        return ERROR;
    }
    if(!fileid || !strlen(fileid))
    {
        logger_error("fileid is null");
        return ERROR;
    }
    //从跟踪服务器获取存储服务器地址列表
    int result;
    std::string ssaddrs;
    if((result = saddrs(appid,userid,fileid,ssaddrs)) != OK)
    {
        logger_error("storage server addresses is null");
        return result;
    }
    std::vector<std::string> vsaddrs;
    splitstring(ssaddrs.c_str(), vsaddrs);
    if(vsaddrs.empty())
    {
        logger_error("storage server addresses is empty");
        return ERROR;
    }
    result = ERROR;
    // 遍历存储服务器地址列表尝试创建存储服务器连接池
    for(const auto& saddr : vsaddrs)
    {
        //尝试创建存储服务器连接池
        pool_c *spool = dynamic_cast<pool_c *>(s_pool_manager->get(saddr.c_str()));
        if(!spool)
        {
            //如果当前地址没有连接池也可能是没有创建，创建后再次获取连接池
            s_pool_manager->set(saddr.c_str(), s_scount);
            if(!(spool = dynamic_cast<pool_c*>(s_pool_manager->get(saddr.c_str()))))
            {
                logger_warn("storage server connection pool is null saddr:%s",saddr.c_str());
                continue;
            }
        }
        // 获取存储服务器连接
        for (int i = 0; i < MAX_SOCKERRS; ++i)
        {
            conn_c *sconn = dynamic_cast<conn_c *>(spool->peek());
            if(!sconn)
            {
                logger_error("storage server connection is null saddr:%s",saddr.c_str());
                break;
            }
            // 删除存储服务器上的文件
            logger("delete file #%d, saddr:%s, appid:%s, userid:%s, fileid:%s", 
            i + 1, saddr.c_str(), appid, userid, fileid);
            result = sconn->del(appid,userid,fileid);
            if (result == SOCKET_ERROR)
            {
                logger_warn("delete file fail:%s",sconn->errdesc());
                spool->put(sconn, false);
            }
            else
            {
                if(result == OK)
                    spool->put(sconn, true);
                else
                {
                    logger_error("delete file fail:%s", sconn->errdesc());
                    spool->put(sconn, false);
                }
                return result;
            }
        }
    }
    return result;
}   

// 向密钥协商服务器发送公钥注册请求
int client_c::registerPublicKey(char const *appid, char const *userid, const long long &keylen, const char* publicKey, const char* signdata) const
{
    return 0;
}
//向密钥协商服务器发送密钥协商请求
int client_c::getKey(char const *appid, char const *userid, char *&key, long long &keylen) const
{
    return 0;
}