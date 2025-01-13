//客户机
//声明客户机类
#pragma once
#include <vector>
#include <string>
#include <acl-lib/acl_cpp/lib_acl.hpp>

class client_c
{
public:
    client_c();
    ~client_c();
public:
    //静态成员初始化   跟踪服务器地址列表 跟踪服务器连接池容量 存储服务器连接池容量
    static int init(char const* taddrs,int tcount = 16,int scount = 8);
    // 静态成员销毁
    static void deinit();
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
    int enupload(char const *appid, char const *userid, char const *fileid, char const *filedata, long long filesize);
    // 向存储服务器加密上传文件
    int enupload(char const *appid, char const *userid, char const *fileid, char const *filepath);
    // 向存储服务器询问文件大小
    int filesize(char const *appid, char const *userid, char const *fileid, long long &filesize);
    // 从存储服务器下载文件
    int download(char const *appid, char const *userid, char const *fileid, long long offset, long long size, char **filedata, long long &filesize);
    // 从存储服务器加密下载文件
    int endownload(char const *appid, char const *userid, char const *fileid, long long offset, long long size, char **filedata, long long &filesize);
    // 删除存储服务器上的文件
    int del(char const *appid, char const *userid, char const *fileid);
private:
    // 向密钥协商服务器发送公钥注册请求
    int registerPublicKey(char const *appid, char const *userid, const long long &keylen, const char* publicKey, const char* signdata) const;
    //向密钥协商服务器发送密钥协商请求
    int getKey(char const *appid, char const *userid, char *&key, long long &keylen) const;
private:
    //线程池管理类
    static acl::connect_manager *s_pool_manager;
    // 跟踪服务器地址列表
    static std::vector<std::string> s_taddrs;
    // 存储服务器连接池容量
    static int s_scount;
    // 密钥协商服务器连接池容量
    static int s_ecount;
};

//静态成员变量只能在类外初始化