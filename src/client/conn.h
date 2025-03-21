//客户机
//声明连接类
#pragma once
#include <string>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class conn_c : public acl::connect_client
{
public:
    conn_c(char const *destaddr, int ctimeout = 10, int rtimeout = 30);
    ~conn_c();

public:
    //从跟踪服务器获取存储服务器地址列表
    int saddrs(char const *appid, char const *userid, char const *fileid, std::string &saddrs);
    //从跟踪服务器获取密钥协商服务器地址列表
    int eaddrs(char const *appid, char const *userid, char const *fileid, std::string &eaddrs);
    // 从跟踪服务器获取组列表
    int groups(std::string &groups);
    // 向存储服务器上传文件
    int upload(char const *appid, char const *userid, char const *fileid, char const *filedata, long long filesize);
    // 向存储服务器上传文件
    int upload(char const *appid, char const *userid, char const *fileid, char const *filepath);
    // 向存储服务器加密上传文件
    int enupload(char const *appid, char const *userid, char const *fileid, char const *filedata, long long filesize, const long long &keylen, char const *key);
    // 向存储服务器加密上传文件
    int enupload(char const *appid, char const *userid, char const *fileid, char const *filepath, const long long &keylen, char const *key);
    // 向存储服务器询问文件大小
    int filesize(char const *appid, char const *userid, char const *fileid, long long &filesize);
    // 从存储服务器下载文件
    int download(char const *appid, char const *userid, char const *fileid, long long offset, long long size, char **filedata, long long &filesize);
    // 从存储服务器加密下载文件
    int endownload(char const *appid, char const *userid, char const *fileid, long long offset, long long size, char **filedata, long long &filesize, const long long &keylen, char const *key);
    // 删除存储服务器上的文件
    int del(char const *appid, char const *userid, char const *fileid);
    // 向密钥协商服务器发送公钥注册请求
    int registerPublicKey(char const *appid, char const *userid, const long long &keylen, const char* publicKey, const char* signdata);
    //向密钥协商服务器发送密钥协商请求
    int getKey(char const *appid, char const *userid, char *&key, long long &keylen);
    // 获取错误号
    short errnumb() const;
    // 获取错误描述
    char const *errdesc() const;

protected:
    //打开连接
    bool open(void);
    // 关闭连接
    void close(void);

private:
    //构造请求（部分）（命令，appid，userid，fileid）
    int makerequest(char command, char const *appid, char const *userid, char const *fileid, char *request);
    // 接收包体---->返回包体，和包体长度
    int recvbody(char **body, long long &bodylen);
    // 接收包头--->返回包体长度
    int recvhead(long long &bodylen);

private:
    char *m_destaddr;           //目的地址
    int m_ctimeout;             // 连接超时
    int m_rtimeout;             //读写超时
    acl::socket_stream *m_conn; //链接对象
    short m_errnumb;            //错误号
    acl::string m_errdesc;      //错误描述
};
