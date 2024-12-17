//客户机连接池管理类
#include "pool.h"
#include "pool_manager.h"

pool_manager_c::pool_manager_c()
{

}
pool_manager_c::~pool_manager_c()
{

}
acl::connect_pool *pool_manager_c::create_pool(char const *destaddr, size_t count, size_t index)
{
    return new pool_c(destaddr, count, index);
}
