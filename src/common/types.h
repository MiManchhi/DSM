//公共模块
//定义所有模块都会用到的数据类型和宏
#pragma once

#include <netinet/in.h>
#include <string>

//定义函数返回值
const int OK = 0;             //成功
const int ERROR = -1;         //本地错误
const int SOCKET_ERROR = -2;  //套接字通信错误
const int STATUS_ERROR = -3;  //服务器状态异常

//缓存相关（redis）
const std::string TRACKER_REDIS_PREFIX = "tracker";   //跟踪服务器redis前缀
const std::string STORAGE_REDIS_PREFIX = "storage";   //存储服务器redis前缀

//存储服务器状态
typedef enum storage_status
{
    STORAGE_STATUS_OFFLINE,   //离线
    STORAGE_STATUS_ONLINE,    //在线
    STORAGE_STATUS_ACTIVE     //活动
}storage_status_t;

//存储服务器加入和信息
const int STORAGE_VERSION_MAX = 6;       //版本信息最大字符数
const int STORAGE_GROUPNAME_MAX = 16;    //组名最大字符数
const int STORAGE_HOSTNAME_MAX = 128;    //主机名最大字符数
const int STORAGE_ADDR_MAX = 16;         //Ip地址最大字符数
//加入信息
typedef struct storage_join
{
    char sj_version[STORAGE_VERSION_MAX+1];          //版本
    char sj_groupname[STORAGE_GROUPNAME_MAX+1];      //组名
    char sj_hostname[STORAGE_HOSTNAME_MAX+1];        //主机名
    in_port_t sj_port;                               //端口号
    time_t sj_stime;                                 //启动时间
    time_t sj_jtime;                                 //加入时间
}storage_join_t;
//描述信息
typedef struct storage_info
{
    char si_version[STORAGE_VERSION_MAX+1];          //版本
    char si_hostname[STORAGE_HOSTNAME_MAX+1];        //主机名
    char si_addr[STORAGE_ADDR_MAX+1];                //ip地址
    in_port_t si_port;                               //端口号
    time_t si_stime;                                 //启动时间
    time_t si_jtime;                                 //加入时间
    time_t si_btime;                                 //心跳时间
    storage_status_t si_status;                      //存储服务器状态
}storage_info_t;