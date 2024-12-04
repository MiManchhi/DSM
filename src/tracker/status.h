//跟踪服务器
//声明存储服务器状态检查线程类

#pragma once
#include <acl-lib/acl_cpp/lib_acl.hpp>

class status_c : public acl::thread
{
public:
    status_c();
    ~status_c();

    //终止线程
    void stop();
protected:
    //线程过程--->重新基类
    void* run(void);
private:
    //检查存储服务器状态
    int check(void) const;
    //是否终止
    bool m_stop;
};
