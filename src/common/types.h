//公共模块
//声明所有模块都会用到的数据类型和宏
#pragma once

#include <netinet/in.h>
#include <string>

//定义函数返回值
constexpr int OK = 0;             //成功
constexpr int ERROR = -1;         //本地错误
constexpr int SOCKET_ERROR = -2;  //套接字通信错误
constexpr int STATUS_ERROR = -3;  //服务器状态异常
constexpr int KEYNEGO_ERROR = -4; //密钥协商错误
constexpr int SIGN_ERROR = -5;    //验证签名错误

//缓存相关（redis）
constexpr const char* TRACKER_REDIS_PREFIX = "tracker";   //跟踪服务器redis前缀
constexpr const char* STORAGE_REDIS_PREFIX = "storage";   //存储服务器redis前缀
constexpr const char *KEYNEGO_REDIS_PREFIX = "keynego";   //密钥协商服务器redis前缀

//存储服务器状态
typedef enum storage_status
{
    STORAGE_STATUS_OFFLINE,   //离线
    STORAGE_STATUS_ONLINE,    //在线
    STORAGE_STATUS_ACTIVE     //活动
}storage_status_t;

//存储服务器加入和信息
constexpr int STORAGE_VERSION_MAX = 6;       //版本信息最大字符数
constexpr int STORAGE_GROUPNAME_MAX = 16;    //组名最大字符数
constexpr int STORAGE_HOSTNAME_MAX = 128;    //主机名最大字符数
constexpr int STORAGE_ADDR_MAX = 16;         //Ip地址最大字符数
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

//键值对
constexpr int ID_KEY_MAX = 64;
typedef struct id_pair
{
    char id_key[ID_KEY_MAX+1];                       //键
    long id_value;                                   //值
    int id_offset;                                   //偏移量
}id_pair_t;


//存储服务器读写磁盘文件缓冲区
constexpr int STORAGE_RCVWD_SIZE = 512*1024;
constexpr int STORAGE_RDSND_SIZE = 512*1024;


/*
优化宏定义，将define用constexpr代替
define在预处理阶段进行文本替换不做类型检查  不占用存储空间
constexpr定义的变量必须在编译阶段就可以计算出其值，在编译阶段进行计算，也可定义字面量（const char*）
constexpr是默认inline的，在编译阶段进行展开，不占用运行时空间，是强类型，进行类型检查，可调试

const定义的常量，在运行时分配存储空间

const 和 constexpr 变量之间的主要区别在于，const 变量的初始化可以推迟到运行时。 constexpr 变量必须在编译时进行初始化。 所有的 constexpr 变量都是 const。
如果一个变量具有文本类型并且已初始化，则可以使用 constexpr 声明该变量。 If 初始化由构造函数执行，构造函数必须声明为 constexpr。
当满足以下两个条件时，可以将引用声明为 constexpr：引用的对象由常量表达式初始化，并且在初始化期间所调用的任何隐式转换也均是常数表达式。
constexpr 变量或函数的所有声明都必须具有 constexpr 说明符。
*/