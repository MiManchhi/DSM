//跟踪服务器
//实现存储服务器状态检查线程类
#include <unistd.h>

#include "status.h"
#include "globals.h"

status_c::status_c() : m_stop(false)
{
}

status_c::~status_c()
{
}

void status_c::stop()
{
    m_stop = true;
}

void *status_c::run(void)
{
    for(time_t last = time(NULL); !m_stop; sleep(1))
    {
        time_t now = time(NULL); //当前时间
        //检查当前时间和心跳时间之间的间隔
        if(now - last >= cfg_interval)
        {
            check(); //检查存储服务器状态
            last = now; //更新最后一次检查时间
        }
    }
    return nullptr;
}

int status_c::check(void) const
{
    //当前时间
    time_t now = time(NULL);
    //互斥锁加锁
    if((errno = pthread_mutex_lock(&g_mutex)))
    {
        logger_error("call pthread_mutex_lock fail:%s",strerror(errno));
        return ERROR;
    }
    //遍历组表中的每一个组
    for(auto& group:g_groups)
    {
        //遍历该组中的每一台存储服务器
        for(auto& storage:group.second)
        {
            //若该存储服务器心跳时间太久
            if(now - storage.si_btime >= cfg_interval)
            {
                //将状态标记为离线
                storage.si_status = STORAGE_STATUS_OFFLINE;
            }
        }
    }
    //互斥锁解锁
    if((errno = pthread_mutex_unlock(&g_mutex)))
    {
        logger_error("call pthread_mutex_unlock fail:%s",strerror(errno));
        return ERROR;
    }
    return 0;
}
