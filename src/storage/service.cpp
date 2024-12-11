//存储服务器
//实现业务服务类
#include <linux/limits.h>
#include <algorithm>
#include "proto.h"
#include "util.h"
#include "globals.h"
#include "db.h"
#include "file.h"
#include "id.h"
#include "service.h"

service_c::service_c()
{

}
service_c::~service_c()
{

}

//业务处理
bool service_c::business(acl::socket_stream *conn, char const *head) const
{
    //  |包体长度|命令|状态|  包体 |
    //  |  8    |  1 | 1  |bodylen|
    //解析包头
    long long bodylen = ntoll(head);
    if(bodylen < 0)
    {
        error(conn, -1, "invalid body length: %lld < 0", bodylen);
        return false;
    }
    int command = head[BODYLEN_SIZE];  //命令
    int status = head[BODYLEN_SIZE + COMMAND_SIZE]; //状态
    logger("bodylen:%lld, command:%d,status:%d", bodylen, command, status);
    bool result;
    // 根据命令执行业务处理
    switch (command)
    {
    case CMD_STORAGE_UPLOAD:
        result = upload(conn, bodylen);
        break;
    case CMD_STORAGE_FILESIZE:
        result = filesize(conn, bodylen);
        break;
    case CMD_STORAGE_DOWNLOAD:
        result = download(conn, bodylen);
        break;
    case CMD_STORAGE_DELETE:
        result = delfile(conn, bodylen);
        break;
    default:
        error(conn, -1, "unknown command:%d", command);
        return false;
    }
    return result;
}

//处理来自客户机的上传文件请求
bool service_c::upload(acl::socket_stream *conn, long long bodylen) const
{
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|文件大小|文件内容|
    // |  8    | 1  | 1 |  16  | 256  | 128  |  8    |文件大小|
    //检查包体长度
    long long expectedlen = APPID_SIZE + USERID_SIZE + FILEID_SIZE + BODYLEN_SIZE;
    if(bodylen < expectedlen)
    {
        error(conn, -1, "invalid body length:%lld < %lld", bodylen, expectedlen);
        return false;
    }
    // 接收包体
    char body[expectedlen]; //不接收文件内容
    if(conn->read(body,expectedlen) < 0)
    {
        logger_error("read fail:%s,expectedlen:%lld, from:%s", acl::last_serror(), expectedlen, conn->get_peer());
        return false;
    }
    // 解析包体
    char appid[APPID_SIZE];
    strcpy(appid, body);
    char userid[USERID_SIZE];
    strcpy(userid, body + APPID_SIZE);
    char fileid[FILEID_SIZE];
    strcpy(fileid, body + APPID_SIZE + USERID_SIZE);
    long long filesize = ntoll(body + APPID_SIZE + USERID_SIZE + FILEID_SIZE);
    // 检查文件大小
    if(filesize != bodylen - expectedlen)
    {
        logger_error("inavlid file size:%lld != %lld", filesize, bodylen - expectedlen);
        error(conn, -1, "inavlid file size:%lld != %lld", filesize, bodylen - expectedlen);
        return false;
    }
    // 生成文件路径
    char filepath[PATH_MAX + 1];
    if(genpath(filepath) != OK)
    {
        error(conn, -1, "get filepath fail");
        return false;
    }
    logger("upload file, appid: %s, userid: %s, "
		"fileid: %s, filesize: %lld, filepath: %s",
		appid, userid, fileid, filesize, filepath);
    // 接收并保存文件
    int result = save(conn, appid, userid, fileid, filesize, filepath);
    if(result == SOCKET_ERROR)
        return false;
    else if(result == ERROR)
    {
        error(conn, -1, "receive and save file fail,fileid:%s", fileid);
        return false;
    }
    return ok(conn);
}

// 处理来自客户机的询问文件大小请求
bool service_c::filesize(acl::socket_stream *conn, long long bodylen) const
{
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|
    // |   8   | 1  | 1  | 16  |  256 |  128 |
    //检查包体长度
    long long expectedlen = APPID_SIZE + USERID_SIZE + FILEID_SIZE;
    if(bodylen != expectedlen)
    {
        error(conn, -1, "invalid body length:%lld != %lld", bodylen, expectedlen);
        return false;
    }
    // 接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail :%s ,bodylen:%lld, from:%s", acl::last_serror(), bodylen, conn->get_peer());
        return false;
    }
    // 解析包体
    char appid[APPID_SIZE];
    strcpy(appid, body);
    char userid[USERID_SIZE];
    strcpy(userid, body + APPID_SIZE);
    char fileid[FILEID_SIZE];
    strcpy(fileid, body + APPID_SIZE + USERID_SIZE);
    // 数据库对象
    db_c db;
    // 连接数据库
    if(db.connect() != OK)
        return false;
    // 根据文件ID获取其对应的路径及大小
    std::string filepath;
    long long filesize;
    if(db.get(appid,userid,fileid,filepath,filesize) != OK)
    {
        error(conn, -1, "read database fail,fileid:%s", fileid);
        return false;
    }
    logger("appid:%s, userid:%s, fileid:%s, filepath:%s, filesize:%lld", appid, userid, fileid, filepath.c_str(), filesize);
    // 构造响应
    //  |包体长度|命令|状态|文件大小|
    //  |  8    | 1  | 1  |  8    |
    bodylen = BODYLEN_SIZE;
    long long respondlen = HEADLEN + bodylen;
    char respond[respondlen] = {};
    llton(bodylen, respond);
    respond[BODYLEN_SIZE] = CMD_STORAGE_REPLY;
    respond[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    llton(filesize, respond + HEADLEN);
    // 发送响应
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail: %s, respondlen:%lld, to:%s", acl::last_serror(), respondlen, conn->get_peer());
        return false;
    }
    return true;
}

// 处理来自客户机的下载文件请求
bool service_c::download(acl::socket_stream *conn, long long bodylen) const
{
    return false;
}

// 处理来自客户机的删除文件请求
bool service_c::delfile(acl::socket_stream *conn, long long bodylen) const
{
    return false;
}

//生成文件路径
bool service_c::genpath(char *filepath) const
{
    return false;
}

// 将ID转换为512进制
long service_c::idTo512(long id) const
{
    return -1;
}

// 用文件id生产文件路径
int service_c::idGenpath(char const *spath, long fileid, char *filepath) const
{
    return -1;
}

// 接收并保存文件
int service_c::save(acl::socket_stream *conn, char const *appid, char const *userid, char const *fileid, long long filesize, char const *filepath) const
{   
    return -1;
}

//读取并发送文件
int service_c::send(acl::socket_stream *conn, char const *filepath, long long offset, long long size) const
{
    return -1;
}

// 应答成功
bool service_c::ok(acl::socket_stream *conn) const
{
    return false;
}

// 应答错误
bool service_c::error(acl::socket_stream *conn, short errnumb, char const *format, ...) const
{
    return false;
}
