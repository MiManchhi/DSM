//存储服务器
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
    // 根据文件ID获取其对应的路径及大小
    int get(char const *appid, char const *userid, char const *fileid, std::string &filepath, long long &filesize) const;
    // 设置文件ID和路径大小的对应关系
    int set(char const *appid, char const *userid, char const *fileid, char const *filepath, long long filesize) const;
    // 删除文件ID
    int del(char const *appid, char const *userid, char const *fileid) const;
private:
    //根据用户ID获取其对应的表名
    std::string table_of_user(char const *userid) const;
    // 计算哈希值
    unsigned int hash(char const *buf, size_t len) const;

    MYSQL *m_mysql;
};