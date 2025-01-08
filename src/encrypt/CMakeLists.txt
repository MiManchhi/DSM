cmake_minimum_required(VERSION 3.0)

# 获取所有源文件
file(GLOB ENCRYPT_SOURCES
    *.cpp
    ../common/*.cpp  # 引入 common 目录的源文件
)

# 查找 MySQL 库和包含路径
find_library(MYSQL_LIBRARY NAMES mysqlclient PATHS /usr/lib /usr/local/lib)
find_path(MYSQL_INCLUDE_DIR NAMES mysql/mysql.h PATHS /usr/include /usr/local/include)

# 查找 ACL 库
find_library(ACL_LIBRARY NAMES acl_all acl PATHS /usr/lib /usr/local/lib)

# 查找 OpenSSL 库
find_package(OpenSSL REQUIRED)

# 查找 PThreads 库
find_package(Threads REQUIRED)

# 设置项目根目录路径
set(PROJECT_ROOT $ENV{HOME}/DSM)

# 默认构建类型为 Release，如果未显式指定
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE release)
endif()

# 根据构建类型选择构建目标路径
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_ROOT}/bin/release")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_ROOT}/bin/debug")
endif()

# 编译为可执行文件
add_executable(encrypt ${ENCRYPT_SOURCES})

# 设置包含路径
target_include_directories(encrypt PRIVATE
    ${MYSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../common  # 引入 common 头文件路径
    ${OPENSSL_INCLUDE_DIR}  # 引入 OpenSSL 头文件路径
)

# 链接库
target_link_libraries(encrypt PRIVATE
    ${MYSQL_LIBRARY}
    ${ACL_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    OpenSSL::Crypto  # 链接 OpenSSL 的 Crypto 库
)

# 安装命令
install(TARGETS encrypt
    RUNTIME DESTINATION "${PROJECT_ROOT}/bin/$<CONFIG>"  # 可执行文件安装到 bin/release 或 bin/debug
)
