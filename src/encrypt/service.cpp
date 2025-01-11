//密钥协商服务器
//实现业务服务类
#include "proto.h"
#include "util.h"
#include "globals.h"
#include "db.h"
#include "service.h"
#include "rsacrypto.h"
#include "aescrypto.h"

service_c::service_c()
{}
service_c::~service_c()
{}
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
    case CMD_CLIENT_REGISTER_PUNLICKEY:
        result = clientRegisterPublicKey(conn, bodylen);
        break;
    case CMD_SERVER_REGISTER_PUNLICKEY:
        result = serverRegisterPublicKey(conn, bodylen);
        break;
    case CMD_CLIENT_NEGOKEY:
        result = clientKeyNego(conn, bodylen);
        break;
    case CMD_SERVER_NEGOKEY:
        result = serverKeyNego(conn, bodylen);
        break;
    default:
        error(conn, -1, "unknown command:%d", command);
        return false;
    }
    return result;
}
//处理来自客户机公钥注册请求
bool service_c::clientRegisterPublicKey(acl::socket_stream *conn, long long bodylen) const
{
    // |包体长度|命令|状态|应用ID|用户ID|公钥长度|  公钥 + 签名数据  |
    // |   8   | 1  | 1  |  16 | 256  |  8    |    256 + 256     |
    //检查包体长度
    long long expectedlen = APPID_SIZE + USERID_SIZE + PUBLICKEY_SIZE + PUBLICKEY + SIGN_DATA_SIZE;
    if(expectedlen != bodylen)
    {
        error(conn, -1, "invalid body length:%lld != %lld", bodylen, expectedlen);
        return false;
    }
    // 接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail :%s, bodylen:%lld, from:%s", acl::last_serror(), bodylen, conn->get_peer());
        return false;
    }
    // 解析包体
    char appid[APPID_SIZE];
    strcpy(appid, body);
    char userid[USERID_SIZE];
    strcpy(userid, body + APPID_SIZE);
    long long publicKeyLen = ntoll(body + APPID_SIZE + USERID_SIZE);
    char publicKey[PUBLICKEY];
    strcpy(publicKey, body + APPID_SIZE + USERID_SIZE + PUBLICKEY_SIZE);
    char signData[SIGN_DATA_SIZE];
    strcpy(signData, body + APPID_SIZE + USERID_SIZE + PUBLICKEY_SIZE + PUBLICKEY);
    //rsa类对象
    RsaCrypto rsa(publicKey, false, false);
    // 校验签名
    bool isValid = false;
    rsa.rsaVerify(publicKey, strlen(publicKey), signData, isValid);
    if (!isValid)
    {
        logger_error("rsaVerify is fail,userid:%s, publicKey:%s", userid, publicKey);
        error(conn, SIGN_ERROR, "rsaVerify is fail,userid:%s, publicKey:%s", userid, publicKey);
        return false;
    }
    logger("rsaVerify succeed!,userid:%s, publickey:%s", userid, publicKey);
    //  数据库对象
    db_c db;
    // 连接数据库
    if(db.connect() != OK)
        return false;
    // 注册公钥
    if(db.setClientPublicKey(userid,std::string(publicKey),publicKeyLen) != OK)
    {
        error(conn, -1, "write database fail, userid:%s, publicKey:%s, publicKeyLen:%lld", userid, publicKey, publicKeyLen);
        return false;
    }
    // 发送响应
    return ok(conn);
}
//处理来自存储服务器公钥注册请求
bool service_c::serverRegisterPublicKey(acl::socket_stream *conn, long long bodylen) const
{
    // |包体长度|命令|状态|应用ID|serverID|公钥长度|  公钥 + 签名数据  |
    // |   8   | 1  | 1  |  16 |   256  |  8    |    256 + 256     |
    //检查包体长度
    long long expectedlen = APPID_SIZE + SERVERID_SIZE + PUBLICKEY_SIZE + PUBLICKEY + SIGN_DATA_SIZE;
    if(expectedlen != bodylen)
    {
        error(conn, -1, "invalid body length:%lld != %lld", bodylen, expectedlen);
        return false;
    }
    // 接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail :%s, bodylen:%lld, from:%s", acl::last_serror(), bodylen, conn->get_peer());
        return false;
    }
    // 解析包体
    char appid[APPID_SIZE];
    strcpy(appid, body);
    char serverid[SERVERID_SIZE];
    strcpy(serverid, body + APPID_SIZE);
    long long publicKeyLen = ntoll(body + APPID_SIZE + SERVERID_SIZE);
    char publicKey[PUBLICKEY];
    strcpy(publicKey, body + APPID_SIZE + SERVERID_SIZE + PUBLICKEY_SIZE);
    char signData[SIGN_DATA_SIZE];
    strcpy(signData, body + APPID_SIZE + SERVERID_SIZE + PUBLICKEY_SIZE + PUBLICKEY);
    //rsa类对象
    RsaCrypto rsa(publicKey, false, false);
    // 校验签名
    bool isValid = false;
    rsa.rsaVerify(publicKey, strlen(publicKey), signData, isValid);
    if (!isValid)
    {
        logger_error("rsaVerify is fail,serverid:%s, publicKey:%s", serverid, publicKey);
        error(conn, SIGN_ERROR, "rsaVerify is fail,serverid:%s, publicKey:%s", serverid, publicKey);
        return false;
    }
    logger("rsaVerify succeed!,serverid:%s, publickey:%s", serverid, publicKey);
    // 数据库对象
    db_c db;
    // 连接数据库
    if(db.connect() != OK)
        return false;
    // 注册公钥
    if(db.setServerPublicKey(serverid,std::string(publicKey),publicKeyLen) != OK)
    {
        error(conn, -1, "write database fail, serverid:%s, publicKey:%s, publicKeyLen:%lld", serverid, publicKey, publicKeyLen);
        return false;
    }
    // 发送响应
    return ok(conn);
}
// 处理来自客户机的密钥协商请求
bool service_c::clientKeyNego(acl::socket_stream *conn, long long bodylen) const
{
    // |包体长度|命令|状态|应用ID|用户ID|
    // |  8    |  1 | 1  |  16 | 256  |
    //检查包体长度
    long long expectedlen = APPID_SIZE + USERID_SIZE;
    if(expectedlen != bodylen)
    {
        error(conn, -1, "invalid body length:%lld != %lld", bodylen, expectedlen);
        return false;
    }
    // 接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail :%s, bodylen:%lld, from:%s", acl::last_serror(), bodylen, conn->get_peer());
        return false;
    }
    // 解析包体
    char appid[APPID_SIZE];
    strcpy(appid, body);
    char userid[USERID_SIZE];
    strcpy(userid, body + APPID_SIZE);
    // 数据库对象
    db_c db;
    // 连接数据库
    if(db.connect() != OK)
        return false;
    // 根据用户ID获取对应的公钥
    std::string clientKey;
    if (db.ClientPublicKey(userid,clientKey) != OK)
    {
        error(conn, -1, "read database fail, userid:%s", userid);
        return false;
    }
    //查找客户端对应的密钥如果密钥不存在生成aes密钥
    std::string key;
    int keylen = 0;
    if (db.ClientKey(userid, key) != OK)
    {
        char *tempkey = nullptr;
        AesCrypto::generateKey(16, tempkey, keylen);
        key = std::string(tempkey);
        // 将密钥和userid进行绑定并存储
        if(db.setClientKey(userid,key) != OK)
        {
            logger_error("clientKeyNego::write database fail,userid:%s", userid);
            error(conn, -1, "write database fail, userid:%s", userid);
            return false;
        }
    }
    // rsa类对象
    RsaCrypto rsa(clientKey.c_str(), false, false);
    // 使用客户端公钥加密aes密钥
    char* EnKey = nullptr;
    rsa.rsaPubKeyEncrypt(key.c_str(), strlen(key.c_str()), &EnKey);
    // 构造响应
    //  |包体长度|命令|状态|密钥长度|密钥|
    //  |   8   | 1  | 1  |  8    | 16 |
    bodylen = KEY_SIZE + KEY;
    long long respondlen = HEADLEN + bodylen;
    char respond[respondlen] = {};
    llton(bodylen, respond);
    respond[BODYLEN_SIZE] = CMD_KEYNEGO_SERVEER_REPLY;
    respond[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    llton(key.length(), respond + HEADLEN);
    strcpy(respond + HEADLEN + KEY_SIZE, EnKey);
    // 将加密后的密钥给客户端
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail: %s, respondlen:%lld, to:%s", acl::last_serror(), respondlen, conn->get_peer());
        return false;
    }
    //释放加密后的密钥
    delete[] EnKey;
    return true;
}
// 处理来自存储服务器的密钥协商请求
bool service_c::serverKeyNego(acl::socket_stream *conn, long long bodylen) const
{
    // |包体长度|命令|状态|应用ID|serverID|userid|
    // |  8    |  1 | 1  |  16 |   128  | 256  |
    //检查包体长度
    long long expectedlen = APPID_SIZE + SERVERID_SIZE + USERID_SIZE;
    if(expectedlen != bodylen)
    {
        error(conn, -1, "invalid body length:%lld != %lld", bodylen, expectedlen);
        return false;
    }
    // 接收包体
    char body[bodylen];
    if(conn->read(body,bodylen) < 0)
    {
        logger_error("read fail :%s, bodylen:%lld, from:%s", acl::last_serror(), bodylen, conn->get_peer());
        return false;
    }
    // 解析包体
    char appid[APPID_SIZE];
    strcpy(appid, body);
    char serverid[SERVERID_SIZE];
    strcpy(serverid, body + APPID_SIZE);
    char userid[USERID_SIZE];
    strcpy(userid, body + APPID_SIZE + SERVERID_SIZE);
    // 数据库对象
    db_c db;
    // 连接数据库
    if(db.connect() != OK)
        return false;
    // 根据serverID获取对应的公钥
    std::string serverKey;
    if (db.ClientPublicKey(userid,serverKey) != OK)
    {
        error(conn, -1, "read database fail, userid:%s", serverid);
        return false;
    }
    //查找客户端对应的密钥
    std::string key;
    if (db.ClientKey(userid, key) != OK)
    {
        logger_error("read database fail or clienntkey not exits, serverid:%s, userid:%s", serverid, userid);
        error(conn, -1, "read database fail or clienntkey not exits, serverid:%s, userid:%s", serverid, userid);
        return false;
    }
    // rsa类对象
    RsaCrypto rsa(serverKey.c_str(), false, false);
    // 使用客户端公钥加密aes密钥
    char* EnKey = nullptr;
    rsa.rsaPubKeyEncrypt(key.c_str(), strlen(key.c_str()), &EnKey);
    // 构造响应
    //  |包体长度|命令|状态|密钥长度|密钥|
    //  |   8   | 1  | 1  |  8    | 16 |
    bodylen = KEY_SIZE + KEY;
    long long respondlen = HEADLEN + bodylen;
    char respond[respondlen] = {};
    llton(bodylen, respond);
    respond[BODYLEN_SIZE] = CMD_KEYNEGO_SERVEER_REPLY;
    respond[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    llton(key.length(), respond + HEADLEN);
    strcpy(respond + HEADLEN + KEY_SIZE, EnKey);
    // 将加密后的密钥给存储服务器
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail: %s, respondlen:%lld, to:%s", acl::last_serror(), respondlen, conn->get_peer());
        return false;
    }
    //释放加密后的密钥
    delete[] EnKey;
    return true;
}
// 应答成功
bool service_c::ok(acl::socket_stream *conn) const
{
    // |包体长度|命令|状态|
    // |   8   |  1 |  1 |
    //构造响应
    long long bodylen = 0;
    long long respondlen = HEADLEN+bodylen;
    char respond[respondlen] = {};
    llton(bodylen,respond);
    respond[BODYLEN_SIZE] = CMD_KEYNEGO_SERVEER_REPLY;
    respond[BODYLEN_SIZE+COMMAND_SIZE] = 0;
    //发送响应
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail:%s, respondlen:%lld, to:%s",acl::last_serror(),respondlen,conn->get_peer());
        return false;
    }
    return true;
}
// 应答错误
bool service_c::error(acl::socket_stream *conn, short errnumb, char const *format, ...) const
{
    //错误描述
    char errdesc[ERROR_DESC_SIZE];
    va_list ap; 
    va_start(ap,format);  //获取格式化参数列表
    vsnprintf(errdesc,ERROR_DESC_SIZE,format,ap);  //算终止空白字符，总是加终止字符
    va_end(ap);
    logger_error("%s",errdesc);
    acl::string desc;
    desc.format("[%s] %s",g_hostname.c_str(),errdesc);
    memset(errdesc,0,sizeof(errdesc));
    strncpy(errdesc,desc.c_str(),ERROR_DESC_SIZE - 1); //不算空白字符，长度够加终止字符，不够不加
    size_t desclen = strlen(errdesc);
    desclen += desclen != 0; //如果描述信息长度不为零，加上空字符\0
    //构造响应
    long long bodylen = ERROR_NUMB_SIZE+desclen;
    long long respondlen = HEADLEN+bodylen;
    char respond[respondlen] = {};
    llton(bodylen,respond);
    respond[BODYLEN_SIZE] = CMD_KEYNEGO_SERVEER_REPLY;
    respond[BODYLEN_SIZE+COMMAND_SIZE] = STATUS_ERROR;
    ston(errnumb,respond+HEADLEN);
    if(desclen)
        strcpy(respond+HEADLEN+ERROR_NUMB_SIZE,errdesc);
    //发送响应
    if(conn->write(respond,respondlen) < 0)
    {
        logger_error("write fail:%s,respondlen:%lld,to:%s",acl::last_serror(),respondlen,conn->get_peer());
        return false;
    }
    return true;
}