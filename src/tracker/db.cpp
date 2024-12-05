#include "db.h"
#include "globals.h"
#include "cache.h"

db_c::db_c():m_mysql(mysql_init(nullptr)) //创建mysql对象
{
    if(!m_mysql)
    {
        logger_error("create mysql fail:%s",mysql_error(m_mysql));
    }
}

db_c::~db_c()
{
    if(m_mysql)
    {
        mysql_close(m_mysql);
        m_mysql = nullptr;
    }
}

int db_c::connect()
{
    //遍历mysql地址列表，尝试连接数据库
    //mysql_real_connect(mysql,地址，用户名，密码，库名，0，NULL，0)
    MYSQL* mysql = m_mysql;
    for(const auto& val:g_maddrs)
    {
        //尝试连接所有地址列表，成功返回连接后的mysql，失败返回NULL
        if((m_mysql = mysql_real_connect(mysql,val.c_str(),"root","123456","dsm_trackerdb",0,NULL,0)))
            return OK;
    }
    logger_error("connect database fail:%s",mysql_error(m_mysql = mysql));
    return ERROR;
}

int db_c::getGroupName(char const *userid, std::string &groupname) const
{
    //构造键
    acl::string key;
    key.format("userid:%s",userid);
    //首先尝试从缓存中获取组名
    cache_c cache;
    acl::string value;
    if(cache.get(key,value) == OK)
    {
        groupname = value.c_str();
        return OK;
    }
    //如果缓存中没有再从数据库中获取
    acl::string sql;
    sql.format("SELECT group_name FROM t_router WHERE userid='%s';",userid);
    //查询数据库并获取结果集
    if(mysql_query(m_mysql,sql.c_str()))  //成功返回0，失败返回非零值
    {
        logger_error("query database fail:%s,sql:%s",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    MYSQL_RES* res = mysql_store_result(m_mysql);
    if(!res)
    {
        logger_error("result is null:%s,sql:%s",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    //从结果集中获取组名
    MYSQL_ROW row = mysql_fetch_row(res);
    if(!row)
    {
        logger_warn("result is empty:%s,sql:%s",mysql_error(m_mysql),sql.c_str());
    }
    else
    {
        groupname = row[0];
        //将组名添加到缓存
        cache.set(key,groupname.c_str());
    }
    return OK;
}

int db_c::setIDGroupname(char const *appid, char const *userid, char const *groupname) const
{
    //插入记录
    acl::string sql;
    sql.format("INSERT INTO t_router SET ""appid='%s',userid='%s',group_name='%s';",
    appid,userid,groupname);
    if(mysql_query(m_mysql,sql.c_str()))
    {
        logger_error("insert database fail:%s,sql:%s",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    //检查插入结果
    MYSQL_RES* res = mysql_store_result(m_mysql);
    if(!res && mysql_field_count(m_mysql))
    {
        logger_error("insert database fail:%s,sql:%s",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    return OK;
}

int db_c::getAllGroupname(std::vector<std::string> &groupnames) const
{
    acl::string sql;
    sql.format("SELECT group_name FROM t_groups_info;");
    if(mysql_query(m_mysql,sql.c_str()))
    {
        logger_error("query database fail:%s,sql:%s",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    MYSQL_RES* res = mysql_store_result(m_mysql);
    if(!res)
    {
        logger_error("result is null:%s,sql:%s",mysql_error(m_mysql),sql.c_str());
        return ERROR;
    }
    int nrows = mysql_num_rows(res);
    for(int i = 0; i < nrows; ++i)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        if(!row)
            break;
        groupnames.push_back(row[0]);
    }
    return OK;
}