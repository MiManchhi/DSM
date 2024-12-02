#include "cache.h"
#include "types.h"
#include "globals.h"

cache_c::cache_c()
{
}

cache_c::~cache_c()
{
}

int cache_c::get(char const *key, acl::string &value) const
{
    //构造键 --->添加前缀
    acl::string tracker_key;
    tracker_key.format("%s:%s",TRACKER_REDIS_PREFIX.c_str(),key);
    //检查Redis连接池 -->如果连接池为空打印错误日志
    if(!g_rconns)
    {
        logger_warn("redis connection pool is null, key:%s",tracker_key.c_str());
        return ERROR;
    }
    //从连接池(g_rconns)中获取一个Redis连接(redis_client*) --->peek()获取连接
    acl::redis_client* rconn = (acl::redis_client*)g_rconns->peek();
    if(!rconn)
    {
        logger_warn("peek redis connection fail, key:%s",tracker_key.c_str());
        return ERROR;
    }
    //构造Redis客户机---->持有此连接的对象为Redis客户机
    acl::redis redis;
    redis.set_client(rconn);
    //借助Redis客户机根据键获取值
    if(!redis.get(tracker_key.c_str(),value))
    {
        logger_warn("get cache fail key:%s",tracker_key.c_str());
        g_rconns->put(rconn,false);   //将连接放回连接池，并标记为不可用
        return ERROR;
    }
    //检查空值
    if(value.empty())
    {
        logger_warn("value is empty, key:%s",tracker_key.c_str());
        g_rconns->put(rconn,false);   //将连接放回连接池，并标记为不可用
        return ERROR;
    }
    logger("get cache ok,key:%s, value:%s",tracker_key.c_str(),value.c_str());
    g_rconns->put(rconn,true);  //将连接放回连接池
    return OK;
}

int cache_c::set(char const *key, char const *value, int timeout) const
{
    //构造键
    acl::string tracker_key;
    tracker_key.format("%s:%s",TRACKER_REDIS_PREFIX.c_str(),key);
    //检查Redis连接池
    if(!g_rconns)
    {
        logger_warn("redis connection pool is null, key:%s, value:%s, timeout:%d",tracker_key.c_str(),value,timeout);
        return ERROR;
    }
    //从Redis连接池中拿出一个连接
    acl::redis_client* rconn = (acl::redis_client*)g_rconns->peek();
    if(!rconn)
    {
        logger_warn("peek redis connection fail, key:%s, value:%s, timeout:%d",tracker_key.c_str(),value,timeout);
        return ERROR;
    }
    //构造Redis客户端
    acl::redis redis;
    redis.set_client(rconn);
    //设置键值对
    if(!redis.setex(tracker_key.c_str(),value,timeout))
    {
        logger_warn("set cache fail key:%s, value:%s, timeout:%d",tracker_key.c_str(),value,timeout);
        g_rconns->put(rconn,false);   //将连接放回连接池，并标记为不可用
        return ERROR;
    }
    logger("set key_value_pair ok ,key:%s, value:%s, timeout:%d",tracker_key.c_str(),value,timeout);
    g_rconns->put(rconn,true);
    return OK;
}

int cache_c::del(char const *key) const
{
    //构造键 --->添加前缀
    acl::string tracker_key;
    tracker_key.format("%s:%s",TRACKER_REDIS_PREFIX.c_str(),key);
    //检查Redis连接池 -->如果连接池为空打印错误日志
    if(!g_rconns)
    {
        logger_warn("redis connection pool is null, key:%s",tracker_key.c_str());
        return ERROR;
    }
    //从连接池(g_rconns)中获取一个Redis连接(redis_client*) --->peek()获取连接
    acl::redis_client* rconn = (acl::redis_client*)g_rconns->peek();
    if(!rconn)
    {
        logger_warn("peek redis connection fail, key:%s",tracker_key.c_str());
        return ERROR;
    }
    //构造Redis客户机---->持有此连接的对象为Redis客户机
    acl::redis redis;
    redis.set_client(rconn);
    //借助Redis客户机根据键删除值
    if(!redis.del_one(tracker_key.c_str()))
    {
        logger_warn("delete cache fail key:%s",tracker_key.c_str());
        g_rconns->put(rconn,false);   //将连接放回连接池，并标记为不可用
        return ERROR;
    }

    logger("delete cache ok,key:%s",tracker_key.c_str());
    g_rconns->put(rconn,true);  //将连接放回连接池
    return OK;
}
