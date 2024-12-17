//客户机连接池管理类
#pragma once

#include <acl-lib/acl_cpp/lib_acl.hpp>

class pool_manager_c : public acl::connect_manager
{
public:
    pool_manager_c();
    ~pool_manager_c();
protected:
    acl::connect_pool *create_pool(char const *destaddr, size_t count, size_t index);
};

//pool_manager_c ---->根据目的地址创建一个与目的地址对应的连接池   ----->继承connect_manager
//pool_c         ---->创建连接，获取连接  --->返回connect_client   ---->继承connect_pool
//conn_c         ---->与服务器进行通信  --->继承connect_client