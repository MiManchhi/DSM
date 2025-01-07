//密钥协商服务器
//实现数据库访问类
#include "db.h"
#include "types.h"
#include "globals.h"
#include "cache.h"

db_c::db_c(): m_mysql(mysql_init(nullptr))
{   
    if(!m_mysql)
        logger_error("create dao fail:%s", mysql_error(m_mysql));
}
db_c::~db_c()
{
    if(m_mysql)
    {
        mysql_close(m_mysql);
        m_mysql = nullptr;
    }
}
//连接数据库
int db_c::connect()
{
    MYSQL *mysql = m_mysql;
    //遍历MySQL地址表，尝试连接数据库
    for(const auto &maddr:g_maddrs)
    {
        if((m_mysql = mysql_real_connect(mysql,maddr.c_str(),"root","123456","dsm_storagedb",0,NULL,0)))
        {
            //成功返回连接后的mysql，失败返回NULL
            return OK;
        }
    }
    logger_error("connect database fail:%s", mysql_error(m_mysql));
    return ERROR;
}

/*
 * @brief 根据userid查询客户端公钥
 * @param userid - 客户端ID
 * @param publicKey - 客户端公钥（返回参数）
 * @return OK 查询成功 ERROR 查询错误
*/
int ClientPublicKey(const char *userid, std::string &publicKey)
{
    return OK;
}
/*
 * @brief 设置客户端公钥
 * @param userid - 客户端ID
 * @param publicKey - 客户端公钥
 * @return OK 设置成功 ERROR 设置失败
*/
int setClientPublicKey(const char *userid, const std::string publicKey)
{
    return OK;
}
/*
 * @brief 查询存储服务器公钥
 * @param serverid - 存储服务器ID（主机名）
 * @param publicKey - 客户端公钥（返回参数）
 * @return OK 查询成功 ERROR 查询错误
*/
int ServerPublicKey(const char *serverid, std::string &publicKey)
{
    return OK;
}
/*
 * @brief 设置存储服务器公钥
 * @param serverid - 存储服务器ID（主机名）
 * @param publicKey - 客户端公钥
 * @return OK 设置成功 ERROR 设置失败
*/
int setServerPublicKey(const char *serverid, const std::string publicKey)
{
    return OK;
}
/*
 * @brief 记录会话历史
 * @param userid - 客户端id
 * @param serverid - 存储服务器id
 * @param ukey - 客户端公钥
 * @param skey - 服务器公钥
*/
int Addsession(const char *userid, const char *serverid, const std::string ukey, const std::string skey)
{
    return OK;
}
