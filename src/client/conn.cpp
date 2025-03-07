//客户机
//实现连接类
#include <sys/sendfile.h>
#include <acl-lib/acl/lib_acl.h>
#include "proto.h"
#include "util.h"
#include "conn.h"
#include "aescrypto.h"

conn_c::conn_c(char const *destaddr, int ctimeout/* = 10*/, int rtimeout/* = 30*/) : m_ctimeout(ctimeout),m_rtimeout(rtimeout),m_conn(NULL)
{
    //检查目的地址
    acl_assert(destaddr && *destaddr); //断言，如果指针和地址为空，进程终止
    // 复制目的地址
    m_destaddr = acl_mystrdup(destaddr); //深拷贝
}
conn_c::~conn_c()
{
    //关闭连接
    close();
    // 释放目的地址
    acl_myfree(m_destaddr);
}
//从跟踪服务器获取存储服务器地址列表
int conn_c::saddrs(char const *appid, char const *userid, char const *fileid, std::string &saddrs)
{
    //构造请求
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|
    // |   8   | 1  | 1  |  16 | 256  | 128  |
    long long bodylen = APPID_SIZE + USERID_SIZE + FILEID_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    if(makerequest(CMD_TRACKER_SADDRS,appid,userid,fileid,request) != OK)
        return ERROR;
    llton(bodylen, request);
    if(!open())
        return SOCKET_ERROR;
    // 发送请求
    if(m_conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    // 接收响应
    char *body = NULL;
    int result = recvbody(&body, bodylen);
    // 成功响应
    //|包体长度|命令|状态|组名|存储服务器地址列表|
    //|   8   | 1  | 1  |16+1|包体长度-（16+1）|
    if(result == OK)
    {
        saddrs = body + STORAGE_GROUPNAME_MAX + 1;
    }
    // 失败响应
    //|包体长度|命令|状态|错误号|错误描述|
    //|   8   | 1  | 1  |  2  | <= 1024|
    else if(result == STATUS_ERROR)
    {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }
    // 释放包体
    if(body)
    {
        free(body);
        body = NULL;
    }
    return result;
}

//从跟踪服务器获取密钥协商服务器地址列表
int conn_c::eaddrs(char const *appid, char const *userid, char const *fileid, std::string &eaddrs)
{
    //构造请求
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|
    // |   8   | 1  | 1  |  16 | 256  | 128  |
    long long bodylen = APPID_SIZE + USERID_SIZE + FILEID_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    if(makerequest(CMD_TRACKER_EADDRS,appid,userid,fileid,request) != OK)
        return ERROR;
    llton(bodylen, request);
    if(!open())
        return SOCKET_ERROR;
    // 发送请求
    if(m_conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    // 接收响应
    char *body = NULL;
    int result = recvbody(&body, bodylen);
    // 成功响应
    //|包体长度|命令|状态|组名|密钥协商服务器地址列表|
    //|   8   | 1  | 1  |16+1|包体长度-（16+1）|
    if(result == OK)
    {
        eaddrs = body + ENCRYPT_GROUPNAME_MAX + 1;
    }
    // 失败响应
    //|包体长度|命令|状态|错误号|错误描述|
    //|   8   | 1  | 1  |  2  | <= 1024|
    else if(result == STATUS_ERROR)
    {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }
    // 释放包体
    if(body)
    {
        free(body);
        body = NULL;
    }
    return result;
}

// 从跟踪服务器获取组列表
int conn_c::groups(std::string &groups)
{
    //构造请求
    // |包体长度|命令|状态|
    // |   8   | 1  | 1  |
    long long bodylen = 0;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    llton(bodylen, request);
    request[BODYLEN_SIZE] = CMD_TRACKER_GROUPS;
    request[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    if (!open())
        return SOCKET_ERROR;
    // 发送请求
    if(m_conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    // 接收响应
    char *body = NULL;
    int result = recvbody(&body, bodylen);
    // 成功响应
    //|包体长度|命令|状态|组列表|
    //|   8   | 1  | 1  |     |
    if(result == OK)
    {
        groups = body;
    }
    // 失败响应
    //|包体长度|命令|状态|错误号|错误描述|
    //|   8   | 1  | 1  |  2  | <= 1024|
    else if(result == STATUS_ERROR)
    {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }
    // 释放包体
    if(body)
    {
        free(body);
        body = NULL;
    }
    return result;
}
// 向存储服务器上传文件 
int conn_c::upload(char const *appid, char const *userid, char const *fileid, char const *filedata, long long filesize)
{
    //构造请求
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|文件大小|文件内容|
    // |   8   | 1  | 1  |  16 | 256  | 128  |  8    |       |
    long long bodylen = APPID_SIZE + USERID_SIZE + FILEID_SIZE + BODYLEN_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    if(makerequest(CMD_STORAGE_UPLOAD,appid,userid,fileid,request) != OK)
        return ERROR;
    llton(filesize, request + HEADLEN + APPID_SIZE + USERID_SIZE + FILEID_SIZE);
    bodylen += filesize;
    llton(bodylen, request);
    if (!open())
        return SOCKET_ERROR;
    // 发送请求
    if(m_conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    //发送数据
    if(m_conn->write(filedata,filesize) < 0)
    {
        logger_error("write fail :%s,filesize:%lld, to:%s", acl::last_serror(), filesize, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail :%s,filesize:%lld, to:%s", acl::last_serror(), filesize, m_conn->get_peer());
        close();  //关闭连接
        return SOCKET_ERROR;
    }
    // 接收响应
    char *body = NULL;
    int result = recvbody(&body, bodylen);
    // 失败响应
    //|包体长度|命令|状态|错误号|错误描述|
    //|   8   | 1  | 1  |  2  | <= 1024|
    if(result == STATUS_ERROR)
    {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }
    // 释放包体
    if(body)
    {
        free(body);
        body = NULL;
    }
    return result;
}
// 向存储服务器上传文件
int conn_c::upload(char const *appid, char const *userid, char const *fileid, char const *filepath)
{
    //构造请求
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|文件大小|文件内容|
    // |   8   | 1  | 1  |  16 | 256  | 128  |  8    |       |
    long long bodylen = APPID_SIZE + USERID_SIZE + FILEID_SIZE + BODYLEN_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    if(makerequest(CMD_STORAGE_UPLOAD,appid,userid,fileid,request) != OK)
        return ERROR;
    acl::ifstream ifs;
    if(!ifs.open_read(filepath))
    {
        logger_error("open file fail, filepath: %s", filepath);
        return ERROR;
    }
    long long filesize = ifs.fsize();
    llton(filesize, request + HEADLEN + APPID_SIZE + USERID_SIZE + FILEID_SIZE);
    bodylen += filesize;
    llton(bodylen, request);
    if (!open())
        return SOCKET_ERROR;
    // 发送请求
    if(m_conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    //发送文件
    long long leftbytes = filesize;  //未发字节数
    off_t offset = 0;                    // 偏移
    while (leftbytes)
    {
        long long bytes = std::min(leftbytes, static_cast<long long>(STORAGE_RCVWD_SIZE));
        long long count = sendfile(m_conn->sock_handle(), ifs.file_handle(), &offset, bytes);
        if(count < 0)
        {
            logger_error("send file fail filesize:%lld, leftbytes:%lld", filesize, leftbytes);
            m_errnumb = -1;
            m_errdesc.format("send file fail filesize:%lld, leftbytes:%lld", filesize, leftbytes);
            close();
            return SOCKET_ERROR;
        }
        leftbytes -= count;
    }
    ifs.close();
    // 接收响应
    char *body = NULL;
    int result = recvbody(&body, bodylen);
    // 失败响应
    //|包体长度|命令|状态|错误号|错误描述|
    //|   8   | 1  | 1  |  2  | <= 1024|
    if(result == STATUS_ERROR)
    {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }
    // 释放包体
    if(body)
    {
        free(body);
        body = NULL;
    }
    return result;
}

// 向存储服务器加密上传文件
int conn_c::enupload(char const *appid, char const *userid, char const *fileid, char const *filedata, long long filesize, const long long &keylen, char const *key)
{
    // 初始化 AES 加密器
    AesCrypto aes(key, keylen);

    // 加密文件数据
    char* encrypted_data = nullptr;
    int encrypted_len = 0;
    if (aes.encrypt(filedata, filesize, encrypted_data, encrypted_len) != OK) {
        logger_error("encrypt data fail");
        m_errnumb = -1;
        m_errdesc = "encrypt data fail";
        return ERROR;
    }

    // 构造请求
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|文件大小|加密后的文件内容|
    // |   8   | 1  | 1  |  16 | 256  | 128  |  8    |       |
    long long bodylen = APPID_SIZE + USERID_SIZE + FILEID_SIZE + BODYLEN_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    if (makerequest(CMD_STORAGE_ENUPLOAD, appid, userid, fileid, request) != OK) {
        delete[] encrypted_data;
        return ERROR;
    }

    // 更新包体长度
    bodylen += encrypted_len;
    llton(bodylen, request);

    // 更新文件大小字段（加密后的文件大小）
    llton(encrypted_len, request + HEADLEN + APPID_SIZE + USERID_SIZE + FILEID_SIZE);

    // 打开连接
    if (!open()) {
        delete[] encrypted_data;
        return SOCKET_ERROR;
    }

    // 发送请求头
    if (m_conn->write(request, requestlen) < 0) {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        delete[] encrypted_data;
        return SOCKET_ERROR;
    }

    // 发送加密后的文件数据
    if (m_conn->write(encrypted_data, encrypted_len) < 0) {
        logger_error("write fail:%s, filesize:%d, to:%s", acl::last_serror(), encrypted_len, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, filesize:%d, to:%s", acl::last_serror(), encrypted_len, m_conn->get_peer());
        close();
        delete[] encrypted_data;
        return SOCKET_ERROR;
    }

    // 释放加密数据
    delete[] encrypted_data;

    // 接收响应
    char* body = nullptr;
    int result = recvbody(&body, bodylen);

    // 处理失败响应
    // |包体长度|命令|状态|错误号|错误描述|
    // |   8   | 1  | 1  |  2  | <= 1024|
    if (result == STATUS_ERROR) {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }

    // 释放包体
    if (body) {
        free(body);
        body = nullptr;
    }

    return result;
}
// 向存储服务器加密上传文件
int conn_c::enupload(char const *appid, char const *userid, char const *fileid, char const *filepath, const long long &keylen, char const *key)
{
    // 初始化 AES 加密器
    AesCrypto aes(key, keylen);

    // 打开文件
    acl::ifstream ifs;
    if (!ifs.open_read(filepath)) {
        logger_error("open file fail, filepath: %s", filepath);
        return ERROR;
    }

    // 获取文件大小
    long long filesize = ifs.fsize();

    // 构造请求
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|文件大小|加密后的文件内容|
    // |   8   | 1  | 1  |  16 | 256  | 128  |  8    |       |
    long long bodylen = APPID_SIZE + USERID_SIZE + FILEID_SIZE + BODYLEN_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    if (makerequest(CMD_STORAGE_ENUPLOAD, appid, userid, fileid, request) != OK) {
        ifs.close();
        return ERROR;
    }

    // 更新包体长度
    bodylen += filesize; // 加密后的文件大小可能会变化，这里先使用原始文件大小
    llton(bodylen, request);

    // 更新文件大小字段（加密后的文件大小）
    llton(filesize, request + HEADLEN + APPID_SIZE + USERID_SIZE + FILEID_SIZE);

    // 打开连接
    if (!open()) {
        ifs.close();
        return SOCKET_ERROR;
    }

    // 发送请求头
    if (m_conn->write(request, requestlen) < 0) {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        ifs.close();
        return SOCKET_ERROR;
    }

    // 分块读取、加密并发送文件数据
    long long leftbytes = filesize; // 未发送字节数
    char readbuf[STORAGE_RCVWD_SIZE]; // 读取缓冲区
    char* encrypted_data = nullptr;   // 加密后的数据
    int encrypted_len = 0;            // 加密后的数据长度

    while (leftbytes > 0) {
        // 读取文件数据
        long long bytes = std::min(leftbytes, static_cast<long long>(sizeof(readbuf)));
        if (ifs.read(readbuf, bytes) != bytes) {
            logger_error("read file fail, filepath: %s", filepath);
            m_errnumb = -1;
            m_errdesc.format("read file fail, filepath: %s", filepath);
            close();
            ifs.close();
            delete[] encrypted_data;
            return ERROR;
        }

        // 加密数据
        if (aes.encrypt(readbuf, bytes, encrypted_data, encrypted_len) != OK) {
            logger_error("encrypt data fail");
            m_errnumb = -1;
            m_errdesc = "encrypt data fail";
            close();
            ifs.close();
            delete[] encrypted_data;
            return ERROR;
        }

        // 发送加密后的数据
        if (m_conn->write(encrypted_data, encrypted_len) < 0) {
            logger_error("write fail:%s, encrypted_len:%d, to:%s", acl::last_serror(), encrypted_len, m_conn->get_peer());
            m_errnumb = -1;
            m_errdesc.format("write fail:%s, encrypted_len:%d, to:%s", acl::last_serror(), encrypted_len, m_conn->get_peer());
            close();
            ifs.close();
            delete[] encrypted_data;
            return SOCKET_ERROR;
        }

        // 释放加密数据缓冲区
        delete[] encrypted_data;
        encrypted_data = nullptr;

        // 更新未发送字节数
        leftbytes -= bytes;
    }

    // 关闭文件
    ifs.close();

    // 接收响应
    char* body = nullptr;
    int result = recvbody(&body, bodylen);

    // 处理失败响应
    // |包体长度|命令|状态|错误号|错误描述|
    // |   8   | 1  | 1  |  2  | <= 1024|
    if (result == STATUS_ERROR) {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }

    // 释放包体
    if (body) {
        free(body);
        body = nullptr;
    }

    return result;
}

// 向存储服务器询问文件大小
int conn_c::filesize(char const *appid, char const *userid, char const *fileid, long long &filesize)
{
    //构造请求
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|
    // |   8   | 1  | 1  |  16 | 256  | 128  |
    long long bodylen = APPID_SIZE + USERID_SIZE + FILEID_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    if(makerequest(CMD_STORAGE_FILESIZE,appid,userid,fileid,request) != OK)
        return ERROR;
    llton(bodylen, request);
    if(!open())
        return SOCKET_ERROR;
    // 发送请求
    if(m_conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    // 接收响应
    char *body = NULL;
    int result = recvbody(&body, bodylen);
    // 成功响应
    //|包体长度|命令|状态|文件大小|
    //|   8   | 1  | 1  |   8   |
    if(result == OK)
    {
        filesize = ntoll(body);
    }
    // 失败响应
    //|包体长度|命令|状态|错误号|错误描述|
    //|   8   | 1  | 1  |  2  | <= 1024|
    else if(result == STATUS_ERROR)
    {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }
    // 释放包体
    if(body)
    {
        free(body);
        body = NULL;
    }
    return result;
}
// 从存储服务器下载文件
int conn_c::download(char const *appid, char const *userid, char const *fileid, long long offset, long long size, char **filedata, long long &filesize)
{
    //构造请求
    // |包体长度|命令|状态|应用ID|用户ID|文件ID| 偏移量 |  大小  |
    // |   8   | 1  | 1  |  16 | 256  | 128  |  8    |   8    |
    long long bodylen = APPID_SIZE + USERID_SIZE + FILEID_SIZE + BODYLEN_SIZE + BODYLEN_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    if(makerequest(CMD_STORAGE_DOWNLOAD,appid,userid,fileid,request) != OK)
        return ERROR;
    llton(offset, request + HEADLEN + APPID_SIZE + USERID_SIZE + FILEID_SIZE);
    llton(size, request + HEADLEN + APPID_SIZE + USERID_SIZE + FILEID_SIZE + BODYLEN_SIZE);
    llton(bodylen, request);
    if (!open())
        return SOCKET_ERROR;
    // 发送请求
    if(m_conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    // 接收响应
    char *body = NULL;
    int result = recvbody(&body, bodylen);
    //成功响应
    //|包体长度|命令|状态|文件内容|
    //|   8   | 1  | 1  |文件大小|
    if(result == OK)
    {
        *filedata = body;
        filesize = bodylen;
        return result;
    }
    // 失败响应
    //|包体长度|命令|状态|错误号|错误描述|
    //|   8   | 1  | 1  |  2  | <= 1024|
    if(result == STATUS_ERROR)
    {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }
    // 释放包体
    if(body)
    {
        free(body);
        body = NULL;
    }
    return result;
}

// 从存储服务器加密下载文件
int conn_c::endownload(char const *appid, char const *userid, char const *fileid, long long offset, long long size, char **filedata, long long &filesize, const long long &keylen, char const *key)
{
    // 初始化 AES 解密器
    AesCrypto aes(key, keylen);

    // 构造请求
    // |包体长度|命令|状态|应用ID|用户ID|文件ID| 偏移量 |  大小  |
    // |   8   | 1  | 1  |  16 | 256  | 128  |  8    |   8    |
    long long bodylen = APPID_SIZE + USERID_SIZE + FILEID_SIZE + BODYLEN_SIZE + BODYLEN_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    if (makerequest(CMD_STORAGE_ENDOWNLOAD, appid, userid, fileid, request) != OK) {
        return ERROR;
    }

    // 设置偏移量和大小
    llton(offset, request + HEADLEN + APPID_SIZE + USERID_SIZE + FILEID_SIZE);
    llton(size, request + HEADLEN + APPID_SIZE + USERID_SIZE + FILEID_SIZE + BODYLEN_SIZE);
    llton(bodylen, request);

    // 打开连接
    if (!open()) {
        return SOCKET_ERROR;
    }

    // 发送请求
    if (m_conn->write(request, requestlen) < 0) {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }

    // 接收响应
    char* body = nullptr;
    int result = recvbody(&body, bodylen);

    // 成功响应
    // |包体长度|命令|状态|文件内容|
    // |   8   | 1  | 1  |文件大小|
    if (result == OK) {
        // 解密数据
        char* decrypted_data = nullptr;
        int decrypted_len = 0;
        if (aes.decrypt(body, bodylen, decrypted_data, decrypted_len) != OK) {
            logger_error("decrypt data fail");
            free(body);
            return ERROR;
        }

        // 返回解密后的数据
        *filedata = decrypted_data;
        filesize = decrypted_len;

        // 释放原始加密数据
        free(body);
        return OK;
    }

    // 失败响应
    // |包体长度|命令|状态|错误号|错误描述|
    // |   8   | 1  | 1  |  2  | <= 1024|
    if (result == STATUS_ERROR) {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }

    // 释放包体
    if (body) {
        free(body);
        body = nullptr;
    }

    return result;
}

// 删除存储服务器上的文件
int conn_c::del(char const *appid, char const *userid, char const *fileid)
{
    //构造请求
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|
    // |   8   | 1  | 1  |  16 | 256  | 128  |
    long long bodylen = APPID_SIZE + USERID_SIZE + FILEID_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    if(makerequest(CMD_STORAGE_DELETE,appid,userid,fileid,request) != OK)
        return ERROR;
    llton(bodylen, request);
    if(!open())
        return SOCKET_ERROR;
    // 发送请求
    if(m_conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    // 接收响应
    char *body = NULL;
    int result = recvbody(&body, bodylen);
    // 失败响应
    //|包体长度|命令|状态|错误号|错误描述|
    //|   8   | 1  | 1  |  2  | <= 1024|
    if(result == STATUS_ERROR)
    {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }
    // 释放包体
    if(body)
    {
        free(body);
        body = NULL;
    }
    return result;
}   

// 向密钥协商服务器发送公钥注册请求
int conn_c::registerPublicKey(char const *appid, char const *userid, const long long &keylen, const char* publicKey, const char* signdata)
{
    logger("enter conn_c::registerPublicKey");
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
    //检查keylen
    if(keylen <= 0)
    {
        logger_error("keylen is invalid:%lld < 0", keylen);
        return -1;
    }
    //检查publicKey
    if(!publicKey)
    {
        logger_error("publicKey is null");
        return -1;
    }
    long publicKeylen = strlen(publicKey);
    if(!publicKeylen)
    {
        logger_error("publicKey is null");
        return -1;
    }
    if(publicKeylen > PUBLICKEY_MAX_SIZE)
    {
        logger_error("publicKey too big:%ld > %d", publicKeylen, PUBLICKEY_MAX_SIZE);
        return -1;
    }
    //检查signdata
    if(!signdata)
    {
        logger_error("signdata is null");
        return -1;
    }
    long signdatalen = strlen(signdata) + 1;
    if(!signdatalen)
    {
        logger_error("signdata is null");
        return -1;
    }
    if(signdatalen > SIGN_DATA_SIZE)
    {
        logger_error("signdata too big:%ld > %d", signdatalen, SIGN_DATA_SIZE);
        return -1;
    }
    logger("enter conn_c::registerPublicKey 1");
    // |包体长度|命令|状态|应用ID| userID |公钥长度|  公钥 + 签名数据  |
    // |   8   | 1  | 1  |  16 |   256  |  8    |    256 + 256     |
    //构造请求
    long long bodylen = APPID_SIZE + USERID_SIZE + PUBLICKEY_SIZE + PUBLICKEY + SIGN_DATA_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    llton(bodylen, request);
    request[BODYLEN_SIZE] = CMD_CLIENT_REGISTER_PUNLICKEY;
    request[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    strcpy(request + HEADLEN, appid);
    strcpy(request + HEADLEN + APPID_SIZE, userid);
    llton(keylen, request + HEADLEN + APPID_SIZE + USERID_SIZE);
    strcpy(request + HEADLEN + APPID_SIZE + USERID_SIZE + PUBLICKEY_SIZE, publicKey);
    strcpy(request + HEADLEN + APPID_SIZE + USERID_SIZE + PUBLICKEY_SIZE + PUBLICKEY, signdata);
    logger("enter conn_c::registerPublicKey 2");
    // 发送请求
    if(m_conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    logger("enter conn_c::registerPublicKey 3");
    // 接收响应
    char *body = NULL;
    int result = recvbody(&body, bodylen);
    // 成功响应
    //|包体长度|命令|状态|
    //|   8   | 1  | 1  |
    // 失败响应
    //|包体长度|命令|状态|错误号|错误描述|
    //|   8   | 1  | 1  |  2  | <= 1024|
    if(result == STATUS_ERROR)
    {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }
    logger("enter conn_c::registerPublicKey 4");
    // 释放包体
    if(body)
    {
        free(body);
        body = NULL;
    }
    delete[] signdata;
    return result;
}
//向密钥协商服务器发送密钥协商请求
int conn_c::getKey(char const *appid, char const *userid, char *&key, long long &keylen)
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
    //    | 包体长度 | 命令 | 状态 | 应用ID | userid |            //
    //    |    8    |  1   |  1   | 16    |  256   |            //
    //                                                          //
    //////////////////////////////////////////////////////////////
    //构造请求
    long long bodylen = APPID_SIZE + USERID_SIZE;
    long long requestlen = HEADLEN + bodylen;
    char request[requestlen] = {};
    llton(bodylen, request);
    request[BODYLEN_SIZE] = CMD_CLIENT_NEGOKEY;
    request[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    strcpy(request + HEADLEN, appid);
    strcpy(request + HEADLEN + APPID_SIZE, userid);
    // 发送请求
    if(m_conn->write(request,requestlen) < 0)
    {
        logger_error("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("write fail:%s, requestlen:%lld, to:%s", acl::last_serror(), requestlen, m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    // 接收响应
    char *body = NULL;
    int result = recvbody(&body, bodylen);
    // 成功响应
    //|包体长度|命令|状态|密钥长度|密钥|
    //|   8   | 1  | 1  |   8   |    |
    if(result == OK)
    {
        keylen = ntoll(body);
        key = new char[keylen];
        strcpy(key, body + KEY_SIZE);
    }
    // 失败响应
    //|包体长度|命令|状态|错误号|错误描述|
    //|   8   | 1  | 1  |  2  | <= 1024|
    else if(result == STATUS_ERROR)
    {
        m_errnumb = ntos(body);
        m_errdesc = bodylen > ERROR_NUMB_SIZE ? body + ERROR_NUMB_SIZE : "";
    }
    // 释放包体
    if(body)
    {
        free(body);
        body = NULL;
    }
    return result;
}

// 获取错误号
short conn_c::errnumb() const
{
    return m_errnumb;
}
// 获取错误描述
char const *conn_c::errdesc() const
{
    return m_errdesc.c_str();
}

//打开连接
bool conn_c::open(void)
{
    if(m_conn)  //连接成功后不能再次连接，直接返回，防止内存泄漏
        return true;
    //创建连接对象
    m_conn = new acl::socket_stream;
    // 连接目的主机
    if(!m_conn->open(m_destaddr,m_ctimeout,m_rtimeout))
    {
        logger_error("open %s fail:%s", m_destaddr, acl_last_serror());
        delete m_conn;
        m_conn = NULL;
        m_errnumb = -1;
        m_errdesc.format("open %s fail:%s", m_destaddr, acl_last_serror());
        return false;
    }
    return true;
}
// 关闭连接
void conn_c::close(void)
{
    if(m_conn)
    {
        delete m_conn;
        m_conn = NULL;
    }
}

//构造请求（部分）（命令，appid，userid，fileid）
int conn_c::makerequest(char command, char const *appid, char const *userid, char const *fileid, char *request)
{
    
    // |包体长度|命令|状态|应用ID|用户ID|文件ID|
    // |   8   | 1  | 1 |  16  | 256  | 128  |
    request[BODYLEN_SIZE] = command;
    request[BODYLEN_SIZE + COMMAND_SIZE] = 0;
    if (strlen(appid) >= APPID_SIZE)
    {
        logger_error("appid size too big:%lu > %d", strlen(appid), APPID_SIZE);
        m_errnumb = -1;
        m_errdesc.format("appid size too big:%lu > %d", strlen(appid), APPID_SIZE);
        return ERROR;
    }
    strcpy(request + HEADLEN, appid);
    if (strlen(userid) >= USERID_SIZE)
    {
        logger_error("userid size too big:%lu > %d", strlen(userid), USERID_SIZE);
        m_errnumb = -1;
        m_errdesc.format("userid size too big:%lu > %d", strlen(userid), USERID_SIZE);
        return ERROR;
    }
    strcpy(request + HEADLEN + APPID_SIZE, userid);
    if (strlen(fileid) >= FILEID_SIZE)
    {
        logger_error("fileid size too big:%lu > %d", strlen(fileid), FILEID_SIZE);
        m_errnumb = -1;
        m_errdesc.format("fileid size too big:%lu > %d", strlen(fileid), FILEID_SIZE);
        return ERROR;
    }
    strcpy(request + HEADLEN + APPID_SIZE + USERID_SIZE, fileid);
    return OK;
}
// 接收包体---->返回包体，和包体长度
int conn_c::recvbody(char **body, long long &bodylen)
{
    //接收包头
    int result = recvhead(bodylen);
    logger("bodylen: %lld", bodylen);
    // 如果不是本地错误也不是套接字错误并且包体长度非空--->分配包体
    if(result != ERROR && result != SOCKET_ERROR && bodylen)
    {
        if(!(*body = (char*)malloc(bodylen)))  //分配失败返回NULL
        {
            logger_error("call malloc fail :%s, bodylen:%lld", strerror(errno), bodylen);
            m_errnumb = -1;
            m_errdesc.format("call malloc fail :%s, bodylen:%lld", strerror(errno), bodylen);
            return ERROR;
        }
        // 接收包体
        if(m_conn->read(*body,bodylen) < 0)
        {
            logger_error("read fail :%s, from:%s", acl::last_serror(), m_conn->get_peer());
            m_errnumb = -1;
            m_errdesc.format("read fail :%s, from:%s", acl::last_serror(), m_conn->get_peer());
            free(*body);
            *body = NULL;
            close();
            return SOCKET_ERROR;
        }
    }   
    
    return result;
}
// 接收包头--->返回包体长度
int conn_c::recvhead(long long &bodylen)
{
    //检查连接是否可用，不可以重新连接
    if(!open())
        return SOCKET_ERROR;
    // 包头缓冲区
    char head[HEADLEN];
    // 接收包头
    if(m_conn->read(head,HEADLEN) < 0)
    {
        logger_error("read fail :%s ,from:%s", acl::last_serror(), m_conn->get_peer());
        m_errnumb = -1;
        m_errdesc.format("read fail :%s ,from:%s", acl::last_serror(), m_conn->get_peer());
        close();
        return SOCKET_ERROR;
    }
    // |包体长度|命令|状态|
    // |  8    | 1  | 1  |
    // 解析包头
    if((bodylen = ntoll(head)) < 0)
    {
        logger_error("invalid body length %lld < 0", bodylen);
        m_errnumb = -1;
        m_errdesc.format("invalid body length %lld < 0", bodylen);
        return ERROR;
    }
    int command = head[BODYLEN_SIZE]; // 命令
    int status = head[BODYLEN_SIZE + COMMAND_SIZE]; //状态
    if(status)
    {
        logger_error("response status %d != 0 , from:%s",status,m_conn->get_peer());
        return STATUS_ERROR;
    }
    logger("bodylen:%lld, command:%d, status:%d", bodylen, command, status);
    return OK;
}
