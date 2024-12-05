//ID服务器
//声明数据库操作类
#pragma once
#include <mysql/mysql.h>

class db_c
{
public:
    db_c(void);
    ~db_c(void);

    //连接数据库
    int connect(void);

    //获取ID当前值，同时产生下一个值
    int getID(char const *key, int inc, long &value) const;
private:
    MYSQL *m_mysql;
};
