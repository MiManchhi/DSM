//连接池类
#include "pool.h"
#include "conn.h"

pool_c::pool_c(char const *destaddr, int count, size_t index/* = 0*/) : acl::connect_pool(destaddr,count,index),m_ctimeout(30),m_rtimeout(60),m_itimeout(90)
{

}
pool_c::~pool_c()
{

}
//设置超时
void pool_c::settimeout(int ctimeout/* = 30*/, int rtimeout/* = 60*/, int itimeout/* = 90*/)
{
    m_ctimeout = ctimeout;
    m_rtimeout = rtimeout;
    m_itimeout = itimeout;
}
//获取连接
acl::connect_client *pool_c::peek(void)
{
    connect_pool::check_idle(static_cast<time_t>(m_itimeout));  //检测线程状态，销毁空闲超时的线程
    return connect_pool::peek();
}

//创建连接
acl::connect_client *pool_c::create_connect(void)
{
    return new conn_c(addr_, m_ctimeout, m_rtimeout);  //addr_从基类继承下来，使用destaddr进行初始化了
}
