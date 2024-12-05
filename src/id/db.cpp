//ID服务器
//实现数据库操作类
#include "globals.h"
#include "db.h"

db_c::db_c(void): m_mysql(mysql_init(NULL))
{
    if(!m_mysql)
        logger_error("create dao fail:%s", mysql_error(m_mysql));
}

db_c::~db_c(void)
{
    //销毁MySQL对象
    if(m_mysql)
    {
        mysql_close(m_mysql);
        m_mysql = nullptr;
    }
}

int db_c::connect(void)
{
    //遍历mysql地址列表，尝试连接数据库
    //mysql_real_connect(mysql,地址，用户名，密码，库名，0，NULL，0)
    MYSQL* mysql = m_mysql;
    for(const auto& val:g_maddrs)
    {
        //尝试连接所有地址列表，成功返回连接后的mysql，失败返回NULL
        if((m_mysql = mysql_real_connect(mysql,val.c_str(),"root","123456","dsm_idsdb",0,NULL,0)))
            return OK;
    }
    logger_error("connect database fail:%s",mysql_error(m_mysql = mysql));
    return ERROR;
}

int db_c::getID(char const *key, int inc, long &value) const
{
    //关闭自动提交
    mysql_autocommit(m_mysql, 0);
    // 查询数据库
    acl::string sql;
    sql.format("SELECT id_value FROM t_id_gen WHERE id='%s';", key);
    if(mysql_query(m_mysql,sql.c_str()))
    {
        logger_error("query database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        mysql_autocommit(m_mysql, 1);
        return ERROR;
    }
    // 获取查询结果
    MYSQL_RES *res = mysql_store_result(m_mysql);
    if(!res)
    {
        logger_error("result is null:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
        mysql_autocommit(m_mysql, 1);
        return ERROR;
    }
    //获取结果记录
    MYSQL_ROW row = mysql_fetch_row(res);
    if(row)
    {
        //有记录-->更新旧记录
        sql.format("UPDATE t_id_gen SET id_value="
                   "id_value+%d WHERE id='%s';",
                   inc, key);
        if(mysql_query(m_mysql,sql.c_str()))
        {
            logger_error("update database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
            mysql_autocommit(m_mysql, 1);
            return ERROR;
        }
        //提交数据库
        mysql_commit(m_mysql);
        // 数据库当前值
        value = atol(row[0]);
    }
    else  //无记录---->插入新记录
    {
        sql.format("INSERT INTO t_id_gen SET id='%s', id_value='%d';", key, inc);
        if(mysql_query(m_mysql,sql.c_str()))
        {
            logger_error("insert database fail:%s, sql:%s", mysql_error(m_mysql), sql.c_str());
            mysql_autocommit(m_mysql, 1);
            return ERROR;
        }
        //提交数据库
        mysql_commit(m_mysql);
        //当前缺省值
        value = 0;
    }
    //打开自动提交
    mysql_autocommit(m_mysql, 1);
    return OK;
}