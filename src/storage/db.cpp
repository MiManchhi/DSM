//存储服务器
//实现数据库访问类
#include "types.h"
#include "globals.h"
#include "cache.h"
#include "db.h"


db_c::db_c(): m_mysql(mysql_init(NULL))
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
        else
        {
            logger_error("connect database fail:%s", mysql_error(m_mysql));
            return ERROR;
        }
    }
}

// 根据文件ID获取其对应的路径及大小
int db_c::get(char const *appid, char const *userid, char const *fileid, std::string &filepath, long long &filesize) const
{

}
// 设置文件ID和路径大小的对应关系
int db_c::set(char const *appid, char const *userid, char const *fileid, char const *filepath, long long filesize) const
{

}

// 删除文件ID
int db_c::del(char const *appid, char const *userid, char const *fileid) const
{

}

//根据用户ID获取其对应的表名
std::string db_c::table_of_user(char const *userid) const
{

}

// 计算哈希值
unsigned int db_c::hash(char const *buf, size_t len) const
{

}

