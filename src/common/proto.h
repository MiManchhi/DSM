//公共模块
//定义与报文规约相关的宏和数据类型
#pragma once
#include "types.h"

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//    | 包体长度 | 命令 | 状态 | 包体 |                                       //
//    |    8    |  1  |  1  | 包体长度 |                                    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


const int BODYLEN_SIZE = 8;                                         //包体长度字节数
const int COMMAND_SIZE = 1;                                         //命令字节数
const int STATUS_SIZE = 1;                                          //状态字节数
const int HEADLEN = (BODYLEN_SIZE + COMMAND_SIZE + STATUS_SIZE);    //包头字节数

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//    | 包体长度 | 命令 | 状态 | 错误号 | 错误描述 |                            //
//    |    8    |  1  |  1  | 2     | <=1024   |                           //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

const int ERROR_NUMB_SIZE = 2;                                     //错误号字节数
const int ERROR_DESC_SIZE = 1024;                                  //错误描述最大字节数（包含空字符）


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//    | 包体长度 | 命令 | 状态 | 应用ID | 用户ID | 文件ID |                     //
//    |    8    |  1  |  1  | 16     |   256 |  128  |                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

const int APPID_SIZE = 16;                                         //应用ID最大字节数（包含空字符）
const int USERID_SIZE = 256;                                       //用户ID最达字节数（包含空字符）
const int FILEID_SIZE = 128;                                       //文件ID最大字节数（包含空字符）

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

const int CMD_TRACKER_JOIN = 10;                                   //存储服务器向跟踪服务器发送加入包
const int CMD_TRACKER_BEAT = 11;                                   //存储服务器向跟踪服务器发送心跳包
const int CMD_TRACKER_SADDRS = 12;                                 //客户机从跟踪服务器获取存储服务器地址列表
const int CMD_TRACKER_GROUPS = 13;                                 //客户机从跟踪服务器获取组列表

const int CMD_ID_GET = 40;                                         //存储服务器从ID服务器获取ID

const int CMD_STORAGE_UPLOAD = 70;                                 //客户机向存储服务器上传文件
const int CMD_STORAGE_FILESIZE = 71;                               //客户机向存储服务器询问文件大小
const int CMD_STORAGE_DOWNLOAD = 72;                               //客户机从存储服务器下载文件
const int CMD_STORAGE_DELETE = 73;                                 //客户机删除存储服务器上的文件

const int CMD_TRACKER_REPLY = 100;                                 //跟踪服务器应答
const int CMD_ID_REPLY = 101;                                      //ID服务器应答
const int CMD_STORAGE_REPLY = 102;                                 //存储服务器应答