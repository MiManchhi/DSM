//存储服务器
//初始化全局变量

#include "globals.h"

//extern关键字用于声明以下变量在配置文件中定义，在此文件中使用
//配置信息
char *cfg_gpname;                  //隶属组名
char *cfg_spaths;                  //存储路径表
char *cfg_taddrs;                  //跟踪服务器地址表
char *cfg_iaddrs;                  //id服务器地址表
char *cfg_eaddrs;                  //encrypt服务器地址表
char *cfg_maddrs;                  // MySQL地址表
char* cfg_raddrs;                  //Redis地址表
acl::master_str_tbl cfg_str[] = {  // 字符串配置表
    {"dsm_group_name", "group001", &cfg_gpname},
    {"dsm_store_paths", "../data", &cfg_spaths},
    {"dsm_tracker_addrs", "127.0.0.1:21000", &cfg_taddrs},
    {"dsm_ids_addrs", "127.0.0.1:22000", &cfg_iaddrs},
    {"dsm_encrypt_addrs", "127.0.0.1:24000", &cfg_eaddrs},
    {"mysql_addrs", "127.0.0.1", &cfg_maddrs},
    {"redis_addrs", "127.0.0.1:6379", &cfg_raddrs},
    {0, 0, 0}};

int cfg_bindport;                  //绑定端口
int cfg_interval;                  //心跳间隔秒数
int cfg_mtimeout;                  // MySQl读写超时
int cfg_maxconns;                  //Redis连接池最大连接数
int cfg_ctimeout;                  //Redis连接超时
int cfg_rtimeout;                  //Redis读写超时
int cfg_ktimeout;                  //Redis键超时
acl::master_int_tbl cfg_int[] = {  // 整形配置表
    {"dsm_storage_port", 23000, &cfg_bindport, 0, 0},
    {"dsm_heart_beat_interval", 10, &cfg_interval, 0, 0},
    {"mysql_rw_timeout", 30, &cfg_mtimeout, 0, 0},
    {"redis_max_conn_num", 600, &cfg_maxconns, 0, 0},
    {"redis_conn_timeout", 10, &cfg_ctimeout, 0, 0},
    {"redis_rw_timeout", 10, &cfg_rtimeout, 0, 0},
    {"redis_key_timeout", 60, &cfg_ktimeout, 0, 0},
    {0, 0, 0, 0, 0}};

std::vector<std::string> g_spaths; //存储地址表
std::vector<std::string> g_taddrs; //跟踪服务器地址表
std::vector<std::string> g_iaddrs; //id服务器地址表
std::vector<std::string> g_eaddrs; //encrypt服务器地址表
std::vector<std::string> g_maddrs; // MySQL地址表
std::vector<std::string> g_raddrs; //Redis地址表
acl::redis_client_pool* g_rconns;  //Redis连接池
std::string g_hostname;            //主机名
char const *g_version = "1.0";     // 版本
time_t g_stime;                    //启动时间