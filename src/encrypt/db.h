//密钥协商服务器
//声明数据库访问类
#pragma once

#include <string>
#include <mysql/mysql.h>

class db_c
{
public:
    db_c();
    ~db_c();
public:
    //连接数据库
    int connect();
    
    /*
     * @brief 根据userid查询客户端公钥
     * @param userid - 客户端ID
     * @param publicKey - 客户端公钥（返回参数）
     * @return OK 查询成功 ERROR 查询错误
    */
    int ClientPublicKey(const char *userid, std::string &publicKey) const;
    /*
     * @brief 设置客户端公钥
     * @param userid - 客户端ID
     * @param publicKey - 客户端公钥
     * @return OK 设置成功 ERROR 设置失败
    */
    int setClientPublicKey(const char *userid, const std::string &publicKey, const long long &keylen) const;
    /*
     * @brief 查询存储服务器公钥
     * @param serverid - 存储服务器ID（主机名）
     * @param publicKey - 客户端公钥（返回参数）
     * @return OK 查询成功 ERROR 查询错误
    */
    int ServerPublicKey(const char *serverid, std::string &publicKey) const;
    /*
     * @brief 设置存储服务器公钥
     * @param serverid - 存储服务器ID（主机名）
     * @param publicKey - 客户端公钥
     * @return OK 设置成功 ERROR 设置失败
    */
    int setServerPublicKey(const char *serverid, const std::string &publicKey, const long long &keylen) const;
    /*
     * @brief 设置客户端密钥
     * @param userid - 用户ID
     * @param Key - 客户端密钥
     * @return OK 设置成功 ERROR 设置失败
    */
    int setClientKey(const char *userid, const std::string &Key) const;
    /*
     * @brief 查找客户端密钥
     * @param userid - 用户ID
     * @param Key - 输出参数客户端密钥
     * @return OK 查找成功 ERROR 查找失败
     */
    int ClientKey(const char *userid, std::string &Key) const;
    /*
     * @brief 记录会话历史
     * @param userid - 客户端id
     * @param serverid - 存储服务器id
     * @param ukey - 客户端公钥
     * @param skey - 服务器公钥
     */
    int Addsession(const char *userid, const char *serverid, const std::string &ukey, const std::string &skey) const;

private:
    MYSQL *m_mysql;
};