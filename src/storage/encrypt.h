//存储服务器
//encrypto客户机类
#pragma once

class encrypto_c
{
public:
    encrypto_c();
    ~encrypto_c();
public:
    /*
    * @brief 从encrypto服务器获取与userid相对应的密钥和密钥长度
    * @param key 返回参数 密钥（在栈上分配，需要调用者手动释放delete[])
    */
    int getKey(char const *appid, char const *userid, char const *serverid, char *&key, long long &keylen) const;
    int registerPublicKey(char const *appid, char const *serverid, const long long &keylen, const char* publicKey, const char* signdata) const;

private:
    //向encrypto服务器发送请求，接收并解析响应。从中获取密钥和密钥长度
    int client(char const *requ, long long resqulen, char *&key, long long &keylen) const;
    int registerrequest(char const *requ, long long resqulen) const;
};