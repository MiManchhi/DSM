//跟踪服务器
//声明数据库访问类
#pragma once

#include <string>
#include <vector>
#include <mysql/mysql.h>

class db_c
{
private:
    MYSQL* m_mysql;
public:
    db_c(/* args */);
    ~db_c();
public:
    //连接数据库
    int connect();
    //根据ID获取组名
    int getGroupName(char const* userid,std::string& groupname) const;
    //设置ID和组名的对应关系
    int setIDGroupname(char const* appid,char const* userid,char const* groupname) const;
    //获取全部组名
    int getAllGroupname(std::vector<std::string>& groupnames) const;
};
