//存储服务器主函数
#include "globals.h"
#include "server.h"

int main(void)
{
    //初始化ACL库
    acl::acl_cpp_init();
    acl::log::stdout_open(true);  //日志打印到终端
    // 建立并运行服务器
    server_c &server = acl::singleton2<server_c>::get_instance();  //单例
    server.set_cfg_str(cfg_str);  //字符串配置表
    server.set_cfg_int(cfg_int);  //整形配置表
    server.run_alone("127.0.0.1:23000", "../etc/storage.cfg");
    return 0;
}