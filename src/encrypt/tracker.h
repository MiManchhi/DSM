//密钥协商服务器
//声明跟踪客户端线程类
#pragma once
#include <acl-lib/acl_cpp/lib_acl.hpp>

class tracker_c : public acl::thread
{
public:
    tracker_c();
    tracker_c(char const *taddr);
    ~tracker_c();
public:
    //终止线程
    void stop();
protected:
    //线程过程
    void *run(void);
private:
    //向跟踪服务器发送加入包
    int join(acl::socket_stream *conn) const;
    // 向跟踪服务器发送心跳包
    int beat(acl::socket_stream *conn) const;
    // 是否终止
    bool m_stop;
    // 跟踪服务器地址
    acl::string m_taddr;
};
