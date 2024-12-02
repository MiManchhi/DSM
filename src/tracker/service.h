//跟踪服务器
//声明业务服务类
#pragma once

#include <acl-lib/acl_cpp/lib_acl.hpp>
#include "types.h"

class service_c
{
public:
    service_c(/* args */);
    ~service_c();
public:
    //业务处理          socket通信               包头
    bool business(acl::socket_stream* conn,char const* head) const;
private:
////////////////////   业务处理函数   //////////////////////////////////////
                                                                         //
    //处理来自存储服务器的加入包                    包体长度                //
    bool join(acl::socket_stream* conn,long long bodylen) const;         //
    //处理来自存储服务器的心跳包                    包体长度                //
    bool beat(acl::socket_stream* conn,long long bodylen) const;         //
    //处理来自客户机的获取存储服务器地址列表请求       包体长度              //
    bool saddrs(acl::socket_stream* conn,long long bodylen) const;       //
    //处理来自客户机的获取组列表请求                                       //
    bool groups(acl::socket_stream* conn) const;                        //
                                                                        //
///////////////////////  辅助业务处理函数  /////////////////////////////////

    //将存储服务器加入组表  存储服务器加入信息结构体   ip地址
    int join(storage_join_t const* sj,char const* saddr) const;
    //将存储服务器标记为活动  组名                 主机名               ip
    int beat(char const* groupname,char const* hostname,char const* saddr) const;
    //响应客户机存储服务器地址列表
    int saddrs(acl::socket_stream* conn,char const* appid,char const* userid) const;
    //根据用户ID获取其对应的组名                                          返回参数
    int group_of_user(char const* appid,char const* userid,std::string& groupname) const;
    //根据组名获取存储服务器地址列表
    int saddrs_of_group(char const* groupname,std::string& saddrs) const;
    //应答成功
    bool ok(acl::socket_stream* conn) const;
    //应答错误                                 错误号              错误信息
    bool error(acl::socket_stream* conn,short errnumb,char const* format,...) const;
};
