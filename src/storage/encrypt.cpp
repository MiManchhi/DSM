//存储服务器
//ID客户机类
#include "encrypt.h"
#include "globals.h"
#include "proto.h"
#include "util.h"
#include "types.h"
#include <acl-lib/acl_cpp/lib_acl.hpp>

encrypto_c::encrypto_c()
{

}

encrypto_c::~encrypto_c()
{

}

//从encrypto服务器获取与userid相对应的密钥
int encrypto_c::getKey(char const *appid, char const *userid, char const *serverid, char *&key, long long &keylen) const
{
    //检查userid
    if(!userid)
    {
        logger_error("userid is null");
        return -1;
    }
    long useridlen = strlen(userid);
    if(!useridlen)
    {
        logger_error("userid is null");
        return -1;
    }
    if(useridlen > USERID_SIZE)
    {
        logger_error("userid too big:%ld > %d", useridlen, USERID_SIZE);
        return -1;
    }
    //检查serverid
    if(!serverid)
    {
        logger_error("serverid is null");
        return -1;
    }
    long serveridlen = strlen(serverid);
    if(!serveridlen)
    {
        logger_error("serverid is null");
        return -1;
    }
    if(serveridlen > SERVERID_SIZE)
    {
        logger_error("serverid too big:%ld > %d", serveridlen, SERVERID_SIZE);
        return -1;
    }
    //检查appid
    if(!appid)
    {
        logger_error("appid is null");
        return -1;
    }
    long appidlen = strlen(appid);
    if(!appidlen)
    {
        logger_error("appid is null");
        return -1;
    }
    if(appidlen > APPID_SIZE)
    {
        logger_error("appid too big:%ld > %d", appidlen, APPID_SIZE);
        return -1;
    }
    //////////////////////////////////////////////////////////////
    //                                                          //
    //    | 包体长度 | 命令 | 状态 | 应用ID | serverid | userid | //
    //    |    8    |  1   |  1   | 16    |   128    |  256   | //
    //                                                          //
    //////////////////////////////////////////////////////////////
    //构造请求
    long long bodylen = APPID_SIZE + SERVERID_SIZE + USERID_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    llton(bodylen, request);
    request[BODYLEN_SIZE] = CMD_SERVER_NEGOKEY;
    request[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    strcpy(request + HEADLEN, appid);
    strcpy(request + HEADLEN + APPID_SIZE, serverid);
    strcpy(request + HEADLEN + APPID_SIZE + SERVERID_SIZE, userid);
    // 向encrypt服务器发送请求，接收并解析响应。从中获取密钥和密钥长度
    return client(request, requestlen, key, keylen);
}

int encrypto_c::registerPublicKey(char const *appid, char const *serverid, const long long &keylen, const char* publicKey, const char* signdata) const
{
    //检查serverid
    if(!serverid)
    {
        logger_error("serverid is null");
        return -1;
    }
    long serveridlen = strlen(serverid);
    if(!serveridlen)
    {
        logger_error("serverid is null");
        return -1;
    }
    if(serveridlen > SERVERID_SIZE)
    {
        logger_error("serverid too big:%ld > %d", serveridlen, SERVERID_SIZE);
        return -1;
    }
    //检查appid
    if(!appid)
    {
        logger_error("appid is null");
        return -1;
    }
    long appidlen = strlen(appid);
    if(!appidlen)
    {
        logger_error("appid is null");
        return -1;
    }
    if(appidlen > APPID_SIZE)
    {
        logger_error("appid too big:%ld > %d", appidlen, APPID_SIZE);
        return -1;
    }
    // |包体长度|命令|状态|应用ID|serverID|公钥长度|  公钥 + 签名数据  |
    // |   8   | 1  | 1  |  16 |   256  |  8    |    256 + 256     |
    //构造请求
    long long bodylen = APPID_SIZE + SERVERID_SIZE + PUBLICKEY_SIZE + PUBLICKEY + SIGN_DATA_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    llton(bodylen, request);
    request[BODYLEN_SIZE] = CMD_SERVER_REGISTER_PUNLICKEY;
    request[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    strcpy(request + HEADLEN, appid);
    strcpy(request + HEADLEN + APPID_SIZE, serverid);
    llton(keylen, request + HEADLEN + APPID_SIZE + SERVERID_SIZE);
    strcpy(request + HEADLEN + APPID_SIZE + SERVERID_SIZE + PUBLICKEY_SIZE, publicKey);
    strcpy(request + HEADLEN + APPID_SIZE + SERVERID_SIZE + PUBLICKEY_SIZE + PUBLICKEY, signdata);
    // 向encrypt服务器发送请求
    return registerrequest(request, requestlen);
}

//向encrypt服务器发送请求，接收并解析响应。从中获取密钥
int encrypto_c::client(char const *requ, long long resqulen, char *&key, long long &keylen) const
{
    acl::socket_stream conn;
    //从encrypt服务器地址表中随机抽取一台服务器尝试连接
    srand(time(NULL));
    int nids = g_eaddrs.size();
    int nrand = rand() % nids;
    for (int i = 0; i < nids; ++i)
    {
        if(conn.open(g_eaddrs[nrand].c_str(),0,0))  //open连接
        {
            logger("connect encrypt success, addr:%s", g_eaddrs[nrand].c_str());
            break;
        }
        else
        {
            logger("connect encrypt fail, addr:%s", g_eaddrs[nrand].c_str());
            nrand = (nrand + 1) % nids;
        }
    }
    if(!conn.alive()) //没有连接成功
    {
        logger_error("connect encrypt fail, addrs:%s", cfg_eaddrs);
        return -1;
    }
    //向encrypt服务器发送请求
    if(conn.write(requ,resqulen) < 0)
    {
        logger_error("write fail:%s,requlen:%lld, to:%s", acl::last_serror(), resqulen, conn.get_peer());
        conn.close();
        return -1;
    }
    //从encrypt服务器接收响应
    //  |包体长度|命令|状态|密钥长度|密钥|
    //  |   8   | 1  | 1  |  8    | 16 |
    long long respondlen = HEADLEN + KEY_SIZE + KEY;
    char respond[respondlen] = {};
    if(conn.read(respond,respondlen) < 0)
    {
        logger_error("read fail:%s,respondlen:%lld,from:%s", acl::last_serror(), respondlen, conn.get_peer());
        conn.close();
        return -1;
    }
    //解析包体
    keylen = ntoll(respond + HEADLEN);
    key = new char[keylen];
    strcpy(key, respond + HEADLEN + KEY_SIZE);
    conn.close();
    return 0;
}

int encrypto_c::registerrequest(char const *requ, long long resqulen) const
{
    acl::socket_stream conn;
    //从encrypt服务器地址表中随机抽取一台服务器尝试连接
    srand(time(NULL));
    int nids = g_eaddrs.size();
    int nrand = rand() % nids;
    for (int i = 0; i < nids; ++i)
    {
        if(conn.open(g_eaddrs[nrand].c_str(),0,0))  //open连接
        {
            logger("connect encrypt success, addr:%s", g_eaddrs[nrand].c_str());
            break;
        }
        else
        {
            logger("connect encrypt fail, addr:%s", g_eaddrs[nrand].c_str());
            nrand = (nrand + 1) % nids;
        }
    }
    if(!conn.alive()) //没有连接成功
    {
        logger_error("connect encrypt fail, addrs:%s", cfg_eaddrs);
        return -1;
    }
    //向encrypt服务器发送请求
    if(conn.write(requ,resqulen) < 0)
    {
        logger_error("write fail:%s,requlen:%lld, to:%s", acl::last_serror(), resqulen, conn.get_peer());
        conn.close();
        return -1;
    }
    //从encrypt服务器接收响应
    //  |包体长度|命令|状态|
    //  |   8   | 1  | 1  |
    long long respondlen = HEADLEN + KEY_SIZE + KEY;
    char respond[respondlen] = {};
    if(conn.read(respond,respondlen) < 0)
    {
        logger_error("read fail:%s,respondlen:%lld,from:%s", acl::last_serror(), respondlen, conn.get_peer());
        conn.close();
        return -1;
    }
    //解析包头
    conn.close();
    return 0;
}