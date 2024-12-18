//公共模块
//声明与报文规约相关的宏和数据类型
#pragma once
#include "types.h"

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//    | 包体长度 | 命令 | 状态 | 包体 |                                       //
//    |    8    |  1  |  1  | 包体长度 |                                    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


constexpr int BODYLEN_SIZE = 8;                                         //包体长度字节数
constexpr int COMMAND_SIZE = 1;                                         //命令字节数
constexpr int STATUS_SIZE = 1;                                          //状态字节数
constexpr int HEADLEN = (BODYLEN_SIZE + COMMAND_SIZE + STATUS_SIZE);    //包头字节数

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//    | 包体长度 | 命令 | 状态 | 错误号 | 错误描述 |                            //
//    |    8    |  1  |  1  | 2     | <=1024   |                           //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

constexpr int ERROR_NUMB_SIZE = 2;                                     //错误号字节数
constexpr int ERROR_DESC_SIZE = 1024;                                  //错误描述最大字节数（包含空字符）


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//    | 包体长度 | 命令 | 状态 | 应用ID | 用户ID | 文件ID |                     //
//    |    8    |  1  |  1  | 16     |   256 |  128  |                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

constexpr int APPID_SIZE = 16;                                         //应用ID最大字节数（包含空字符）
constexpr int USERID_SIZE = 256;                                       //用户ID最达字节数（包含空字符）
constexpr int FILEID_SIZE = 128;                                       //文件ID最大字节数（包含空字符）

//存储服务器加入包和心跳包
//使用结构体组织信息 成员均使用char类型，防止内存补齐
typedef struct storage_join_body
{
    char sjb_version[STORAGE_VERSION_MAX+1];                       //版本
    char sjb_groupname[STORAGE_GROUPNAME_MAX+1];                   //组名
    char sjb_hostname[STORAGE_HOSTNAME_MAX+1];                     //主机名
    char sjb_port[sizeof(in_addr_t)];                              //端口号
    char sjb_stime[sizeof(time_t)];                                //启动时间
    char sjb_jtime[sizeof(time_t)];                                //加入时间
}storage_join_body_t;

typedef struct storage_beat_body
{
    char sbb_groupname[STORAGE_GROUPNAME_MAX+1];                   //组名
    char sbb_hostname[STORAGE_HOSTNAME_MAX+1];                     //主机名
}storage_beat_body_t;

//命令

constexpr int CMD_TRACKER_JOIN = 10;                                   //存储服务器向跟踪服务器发送加入包
constexpr int CMD_TRACKER_BEAT = 11;                                   //存储服务器向跟踪服务器发送心跳包
constexpr int CMD_TRACKER_SADDRS = 12;                                 //客户机从跟踪服务器获取存储服务器地址列表
constexpr int CMD_TRACKER_GROUPS = 13;                                 //客户机从跟踪服务器获取组列表

constexpr int CMD_ID_GET = 40;                                         //存储服务器从ID服务器获取ID

constexpr int CMD_STORAGE_UPLOAD = 70;                                 //客户机向存储服务器上传文件
constexpr int CMD_STORAGE_FILESIZE = 71;                               //客户机向存储服务器询问文件大小
constexpr int CMD_STORAGE_DOWNLOAD = 72;                               //客户机从存储服务器下载文件
constexpr int CMD_STORAGE_DELETE = 73;                                 //客户机删除存储服务器上的文件

constexpr int CMD_TRACKER_REPLY = 100;                                 //跟踪服务器应答
constexpr int CMD_ID_REPLY = 101;                                      //ID服务器应答
constexpr int CMD_STORAGE_REPLY = 102;                                 //存储服务器应答

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