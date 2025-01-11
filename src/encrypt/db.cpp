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
        if((m_mysql = mysql_real_connect(mysql,maddr.c_str(),"root","123456","dsm_keynego",0,NULL,0)))
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
int db_c::ClientPublicKey(const char *userid, std::string &publicKey) const
{
    //先尝试从缓存中拿
    cache_c cache;
    acl::string key;
    key.format("uid:publickey:%s", userid);
    acl::string value;
    if(cache.get(key.c_str(),value) == OK)
    {
        logger("from cache publickey:%s", value.c_str());
        publicKey = std::string(value.c_str());
        return OK;
    }
    // 缓存中没有从数据库中获取
    // 根据userid获取客户端公钥
    acl::string sql;
    sql.format("SELECT PublicKey FORM clientkeys WHERE UserID='%s';", userid);
    // 执行sql
    if(mysql_query(m_mysql,sql.c_str()))
    {
        logger_error("query database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    // 获取结果集
    MYSQL_RES *res = mysql_store_result(m_mysql);
    if(!res)
    {
        logger_error("result is null:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    // 获取结果
    MYSQL_ROW row = mysql_fetch_row(res);
    if(!row)
    {
        logger_error("result is empty:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    publicKey = std::string(row[0]);
    logger("from database publickey:%s, userid:%s", publicKey.c_str(), userid);
    // 将结果写入缓存
    value = publicKey;
    if(cache.set(key,value.c_str()) != OK)
    {
        logger_error("set key_value to cache fail:key:%s, value:%s", key.c_str(), value.c_str());
    }
    return OK;
}
/*
 * @brief 设置客户端公钥
 * @param userid - 客户端ID
 * @param publicKey - 客户端公钥
 * @return OK 设置成功 ERROR 设置失败
*/
int db_c::setClientPublicKey(const char *userid, const std::string &publicKey, const long long &keylen) const
{
    //查询客户端对应的公钥是否存在如果存在则直接返回，不存在则插入
    std::string temp;
    if (ClientPublicKey(userid, temp) == OK)
        return OK;
    // 插入记录
    acl::string sql;
    sql.format("INSERT INTO clientkeys SET UserID='%s', PublicKey='%s', KeyLength=%lld;", userid, publicKey.c_str(), keylen);
    if(mysql_query(m_mysql,sql.c_str()))
    {
        logger_error("insert database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    // 检查插入结果
    MYSQL_RES *res = mysql_store_result(m_mysql);
    if(!res && mysql_field_count(m_mysql))
    {
        logger_error("insert database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    return OK;
}
/*
 * @brief 查询存储服务器公钥
 * @param serverid - 存储服务器ID（主机名）
 * @param publicKey - 客户端公钥（返回参数）
 * @return OK 查询成功 ERROR 查询错误
*/
int db_c::ServerPublicKey(const char *serverid, std::string &publicKey) const
{
    //先尝试从缓存中拿
    cache_c cache;
    acl::string key;
    key.format("serverid:%s", serverid);
    acl::string value;
    if(cache.get(key.c_str(),value) == OK)
    {
        logger("from cache publickey:%s", value.c_str());
        publicKey = std::string(value.c_str());
        return OK;
    }
    // 缓存中没有从数据库中获取
    // 根据serverid获存储服务器公钥
    acl::string sql;
    sql.format("SELECT PublicKey FORM serverkeys WHERE ServerID='%s';", serverid);
    // 执行sql
    if(mysql_query(m_mysql,sql.c_str()))
    {
        logger_error("query database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    // 获取结果集
    MYSQL_RES *res = mysql_store_result(m_mysql);
    if(!res)
    {
        logger_error("result is null:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    // 获取结果
    MYSQL_ROW row = mysql_fetch_row(res);
    if(!row)
    {
        logger_error("result is empty:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    publicKey = std::string(row[0]);
    logger("from database publickey:%s, serverid:%s", publicKey.c_str(), serverid);
    // 将结果写入缓存
    value = publicKey;
    if(cache.set(key,value.c_str()) != OK)
    {
        logger_error("set key_value to cache fail:key:%s, value:%s", key.c_str(), value.c_str());
    }
    return OK;
}
/*
 * @brief 设置存储服务器公钥
 * @param serverid - 存储服务器ID（主机名）
 * @param publicKey - 客户端公钥
 * @return OK 设置成功 ERROR 设置失败
*/
int db_c::setServerPublicKey(const char *serverid, const std::string &publicKey, const long long &keylen) const
{
    //查询服务器公钥是否存在如果存在则直接返回，不存在则插入
    std::string temp;
    if (ServerPublicKey(serverid, temp) == OK)
        return OK;
    // 插入记录
    acl::string sql;
    sql.format("INSERT INTO clientkeys SET ServerID='%s', PublicKey='%s', KeyLength=%lld;", serverid, publicKey.c_str(), keylen);
    if(mysql_query(m_mysql,sql.c_str()))
    {
        logger_error("insert database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    // 检查插入结果
    MYSQL_RES *res = mysql_store_result(m_mysql);
    if(!res && mysql_field_count(m_mysql))
    {
        logger_error("insert database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    return OK;
}
/*
 * @brief 记录会话历史
 * @param userid - 客户端id
 * @param serverid - 存储服务器id
 * @param ukey - 客户端公钥
 * @param skey - 服务器公钥
*/
int db_c::Addsession(const char *userid, const char *serverid, const std::string &ukey, const std::string &skey) const
{
    //插入记录
    acl::string sql;
    sql.format("INSERT INTO sessionkeys SET UserID='%s', ServerID='%s', EncryptedKeyForClient='%s', EncryptedKeyForServer='%s';", userid, serverid, ukey.c_str(), skey.c_str());
    if (mysql_query(m_mysql, sql.c_str()))
    {
        logger_error("insert database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    // 检查插入结果
    MYSQL_RES *res = mysql_store_result(m_mysql);
    if(!res && mysql_field_count(m_mysql))
    {
        logger_error("insert database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    return OK;
}


int db_c::ClientKey(const char *userid, std::string &Key) const
{
    //先尝试从缓存中拿
    cache_c cache;
    acl::string key;
    key.format("uid:key:%s", userid);
    acl::string value;
    if(cache.get(key.c_str(),value) == OK)
    {
        logger("from cache Key:%s", value.c_str());
        Key = std::string(value.c_str());
        return OK;
    }
    // 缓存中没有从数据库中获取
    // 根据userid获取客户端密钥
    acl::string sql;
    sql.format("SELECT PrivateKey FORM clientprivatekeys WHERE UserID='%s';", userid);
    // 执行sql
    if(mysql_query(m_mysql,sql.c_str()))
    {
        logger_error("query database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    // 获取结果集
    MYSQL_RES *res = mysql_store_result(m_mysql);
    if(!res)
    {
        logger_error("result is null:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    // 获取结果
    MYSQL_ROW row = mysql_fetch_row(res);
    if(!row)
    {
        logger_error("result is empty:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    Key = std::string(row[0]);
    logger("from database publickey:%s, userid:%s", Key.c_str(), userid);
    // 将结果写入缓存
    value = Key;
    if(cache.set(key,value.c_str()) != OK)
    {
        logger_error("set key_value to cache fail:key:%s, value:%s", key.c_str(), value.c_str());
    }
    return OK;
}
int db_c::setClientKey(const char *userid, const std::string &Key) const
{
    //查询客户端对应的密钥是否存在如果存在则直接返回，不存在则插入
    std::string temp;
    if (ClientKey(userid, temp) == OK)
        return OK;
    // 插入记录
    acl::string sql;
    sql.format("INSERT INTO clientprivatekeys SET UserID='%s', PrivateKey='%s';", userid, Key.c_str());
    if(mysql_query(m_mysql,sql.c_str()))
    {
        logger_error("insert database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    // 检查插入结果
    MYSQL_RES *res = mysql_store_result(m_mysql);
    if(!res && mysql_field_count(m_mysql))
    {
        logger_error("insert database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        return ERROR;
    }
    return OK;
}
