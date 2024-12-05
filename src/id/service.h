//ID服务器
//业务服务类声明
#pragma once
#include <acl-lib/acl_cpp/lib_acl.hpp>

class service_c
{
public:
    service_c();
    ~service_c();
public:
    //业务处理
    bool business(acl::socket_stream *conn, char const *head) const;
private:
    //处理来自存储服务器的获取ID请求
    bool get(acl::socket_stream *conn, long long bodylen) const;
    // 根据ID的键获取其值
    long get(char const *key) const;
    // 从数据库中获取ID值
    long valueFromDB(char const *key) const;
    // 应答ID
    bool id(acl::socket_stream *conn, long value) const;
    // 应答错误
    bool error(acl::socket_stream *conn, short errnumb, char const *format, ...) const;
};
