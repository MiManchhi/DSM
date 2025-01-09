//密钥协商服务器
//声明业务服务类
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
    //处理来自客户机公钥注册请求
    bool clientRegisterPublicKey(acl::socket_stream *conn, long long bodylen) const;
    //处理来自存储服务器公钥注册请求
    bool serverRegisterPublicKey(acl::socket_stream *conn, long long bodylen) const;
    // 处理来自客户机的密钥协商请求
    bool clientKeyNego(acl::socket_stream *conn, long long bodylen) const;
    // 处理来自存储服务器的密钥协商请求
    bool serverKeyNego(acl::socket_stream *conn, long long bodylen) const;
    // 应答成功
    bool ok(acl::socket_stream *conn) const;
    // 应答错误
    bool error(acl::socket_stream *conn, short errnumb, char const *format, ...) const;
};