//跟踪服务器
//声明全局变量
#pragma once

#include <vector>
#include <string>
#include <map>
#include <list>
#include <acl-lib/acl_cpp/lib_acl.hpp>

#include "types.h"

//extern关键字用于声明以下变量在配置文件中定义，在此文件中使用
//配置信息
extern char* cfg_appids;                  //应用ID表
extern char* cfg_maddrs;                  //MySQL地址表
extern char* cfg_raddrs;                  //Redis地址表
extern acl::master_str_tbl cfg_str[];     //字符串配置表

extern int cfg_interval;                  //存储服务器状态检查间隔秒数
extern int cfg_mtimeout;                  //MySQl读写超时
extern int cfg_maxconns;                  //Redis连接池最大连接数
extern int cfg_ctimeout;                  //Redis连接超时
extern int cfg_rtimeout;                  //Redis读写超时
extern int cfg_ktimeout;                  //Redis键超时
extern acl::master_int_tbl cfd_int[];     //整形配置表

extern std::vector<std::string> g_appids; //应用ID表
extern std::vector<std::string> g_maddrs; //MySQL地址表
extern std::vector<std::string> g_raddrs; //Redis地址表
extern acl::redis_client_pool* g_rconns;  //Redis连接池
extern std::string g_hostname;            //主机名
extern std::map<std::string,std::list<storage_info_t>> g_groups;  //组表
extern pthread_mutex_t g_mutex;           //互斥锁，保护组表并发读写