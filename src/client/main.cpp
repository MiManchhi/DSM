//客户机主函数
#include <unistd.h>
#include <unordered_map>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/acl/lib_acl.h>
#include "types.h"
#include "client.h"

//打印命令行用法
void usage(char const* cmd)
{
    /*
        ./client 127.0.0.1:21000 groups
        ./client 127.0.0.1:21000 upload <appid> <userid> <filepath>
        ./client 127.0.0.1:21000 filesize <appid> <userid> <fileid>
        ./client 127.0.0.1:21000 download <appid> <userid> <fileid> <offset> <size>
        ./client 127.0.0.1:21000 delete <appid> <userid> <fileid>
    */
    fprintf(stderr, "Groups : %s <taddrs> groups\n", cmd);
    fprintf(stderr, "Upload : %s <taddrs> upload <appid> <userid> <filepath>\n", cmd);
    fprintf(stderr, "Filesize : %s <taddrs> filesize <appid> <userid> <fileid>\n", cmd);
    fprintf(stderr, "download : %s <taddrs> download <appid> <userid> <fileid> <offset> <size>\n", cmd);
    fprintf(stderr, "delete : %s <taddrs> delete <appid> <userid> <fileid>\n", cmd);
}

//根据用户ID生成文件ID
/*
(uerid + pid + threadid + rand())--->md5摘要(32)---->去掉前后8字节---->string(16)
---->秒+微秒+string(16)+计数+随机数---->fileid
*/
std::string genfileid(char const *userid)
{
    srand(time(NULL));
    //获取当前时间的细节（包括微秒）
    struct timeval now;
    gettimeofday(&now, NULL);
    acl::string str;
    //uerid + pid + threadid + rand()
    str.format("%s@%d@%lX@%d", userid, getpid(), acl_pthread_self(), rand());
    //md5摘要
    acl::md5 md5;
    md5.update(str.c_str(), str.size());
    md5.finish();
    char buf[33] = {};
    strncpy(buf, md5.get_string(), 32);
    memmove(buf, buf + 8, 16);  //从buf的前8位开始复制16个字节到buf  实现去掉前8字节
    memset(buf + 16, 0, 16);    //将buf+16字节后的内容置空  实现去除后8字节
    static int count = 0;
    if(count >= 8000)
        count = 0;
    acl::string fileid;
    fileid.format("%08lx%06lx%s%04d%02d", now.tv_sec, now.tv_usec, buf, ++count, rand() % 100);
    return fileid.c_str();
}

std::unordered_map<std::string, int> cmd_index =
{
    {"groups",0},
    {"upload",1},
    {"filesize",2},
    {"download",3},
    {"delete",4}
};

//从跟踪服务器获取组列表
int client_groups(client_c &client)
{
    std::string groups;
    if(client.groups(groups) != OK)
    {
        client_c::deinit();
        return -1;
    }
    printf("%s\n", groups.c_str());
    return 0;
}

// 向存储服务器上传文件
int client_upload(int argc, char *argv[], client_c &client)
{
    char const *cmd = argv[0];
    if(argc < 6)
    {
        client_c::deinit();
        usage(cmd);
        return -1;
    }
    char const *appid = argv[3];
    char const *userid = argv[4];
    char const *filepath = argv[5];
    std::string fileid = genfileid(userid);
    if(client.upload(appid,userid,fileid.c_str(),filepath) != OK)
    {
        client_c::deinit();
        return -1;
    }
    printf("Upload success :%s\n", fileid.c_str());
    return 0;
}

// 向存储服务器询问文件大小
int client_filesize(int argc, char *argv[], client_c &client)
{
    char const *cmd = argv[0];
    if(argc < 6)
    {
        client_c::deinit();
        usage(cmd);
        return -1;
    }
    char const *appid = argv[3];
    char const *userid = argv[4];
    char const *fileid = argv[5];
    long long filesize = 0;
    if (client.filesize(appid, userid, fileid,filesize) != OK)
    {
        client_c::deinit();
        return -1;
    }
    printf("Get filesize success :%lld\n", filesize);
    return 0;
}

// 从存储服务器下载文件
int client_download(int argc, char *argv[], client_c &client)
{
    char const *cmd = argv[0];
    if(argc < 8)
    {
        client_c::deinit();
        usage(cmd);
        return -1;
    }
    char const *appid = argv[3];
    char const *userid = argv[4];
    char const *fileid = argv[5];
    long long offset = atoll(argv[6]);
    long long size = atoll(argv[7]);
    char *filedata = NULL;
    long long filesize = 0;
    if (client.download(appid,userid,fileid,offset,size,&filedata,filesize) != OK)
    {
        client_c::deinit();
        return -1;
    }
    printf("Download file success :%lld\n", filesize);
    //这里只是测试，实际需要使用下载到的数据，如写入文件，使用后需要释放
    free(filedata);
    return 0;
}

// 删除存储服务器上的文件
int client_delete(int argc, char *argv[], client_c &client)
{
    char const *cmd = argv[0];
    if(argc < 6)
    {
        client_c::deinit();
        usage(cmd);
        return -1;
    }
    char const *appid = argv[3];
    char const *userid = argv[4];
    char const *fileid = argv[5];
    if(client.del(appid,userid,fileid) != OK)
    {
        client_c::deinit();
        return -1;
    }
    printf("Delete file success :%s\n", fileid);
    return 0;
}

int main(int argc, char *argv[])
{
    char const *cmd = argv[0];
    if(argc < 3)
    {
        usage(cmd);
        return -1;
    }
    char const *taddrs = argv[1];
    char const *subcmd = argv[2];
    // 初始化ACL库
    acl::acl_cpp_init();
    // 日志打印到标准输出
    acl::log::stdout_open(true);
    // 初始化客户机
    if(client_c::init(taddrs) != OK)
        return -1;
    // 客户机对象
    client_c client;
    const auto &iter = cmd_index.find(std::string(subcmd));
    int index = -1;
    if (iter != cmd_index.cend())
        index = iter->second;
    switch (index)
    {
    case 0 :
        // 从跟踪服务器获取组列表
        client_groups(client);
        break;
    case 1 :
        // 向存储服务器上传文件
        client_upload(argc, argv, client);
        break;
    case 2 :
        // 向存储服务器询问文件大小
        client_filesize(argc, argv, client);
        break;
    case 3 :
        // 从存储服务器下载文件
        client_download(argc, argv, client);
        break;
    case 4 :
        // 删除存储服务器上的文件
        client_delete(argc, argv, client);
        break;
    default:
        client_c::deinit();
        usage(cmd);
        return -1;
    }
    // 终结化客户机
    client_c::deinit();
    return 0;
}
