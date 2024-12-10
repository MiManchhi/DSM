//存储服务器
//ID客户机类
#pragma once

class id_c
{
public:
    id_c();
    ~id_c();
public:
    //从ID服务器获取与ID键相对应的值
    long get(char const *key) const;
private:
    //向ID服务器发送请求，接收并解析响应。从中获取ID值
    int client(char const *requ, long long resqulen) const;
};