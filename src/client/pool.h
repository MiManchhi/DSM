//连接池类
#pragma once

#include <acl-lib/acl_cpp/lib_acl.hpp>

class pool_c : public acl::connect_pool
{
public:                         //最大连接数
    pool_c(char const *destaddr, int count, size_t index = 0);
    ~pool_c();
    //设置超时
    void settimeout(int ctimeout = 30, int rtimeout = 60, int itimeout = 90);
    //获取连接
    acl::connect_client *peek(void);

protected:
    //创建连接
    acl::connect_client *create_connect(void);
private:
    //连接超时
    int m_ctimeout;
    // 读写超时
    int m_rtimeout;
    // 空闲超时
    int m_itimeout;
};
