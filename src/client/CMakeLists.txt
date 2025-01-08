cmake_minimum_required(VERSION 3.0)

# 可以选择是否编译为静态库
option(BUILD_STATIC_LIB "Build client as static library" OFF)

# 获取所有源文件
file(GLOB CLIENT_SOURCES
    *.cpp
    ../common/*.cpp  # 引入 common 目录的源文件
)

# 查找 MySQL 库和包含路径
find_library(MYSQL_LIBRARY NAMES mysqlclient PATHS /usr/lib /usr/local/lib)
find_path(MYSQL_INCLUDE_DIR NAMES mysql/mysql.h PATHS /usr/include /usr/local/include)

# 查找 ACL 库
find_library(ACL_LIBRARY NAMES acl_all acl PATHS /usr/lib /usr/local/lib)

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
    set(LIBRARY_OUTPUT_PATH "${PROJECT_ROOT}/lib/release")
    set(ARCHIVE_OUTPUT_PATH "${PROJECT_ROOT}/lib/release")  # 静态库的输出目录
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_ROOT}/bin/debug")
    set(LIBRARY_OUTPUT_PATH "${PROJECT_ROOT}/lib/debug")
    set(ARCHIVE_OUTPUT_PATH "${PROJECT_ROOT}/lib/debug")  # 静态库的输出目录
endif()

# 根据选项，编译为静态库或可执行文件
if(BUILD_STATIC_LIB)
    # 编译为静态库
    add_library(client STATIC ${CLIENT_SOURCES})
else()
    # 编译为可执行文件
    add_executable(client ${CLIENT_SOURCES})
endif()

# 设置包含路径
target_include_directories(client PRIVATE
    ${MYSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../common  # 引入 common 头文件路径
)

# 链接库
target_link_libraries(client PRIVATE
    ${MYSQL_LIBRARY}
    ${ACL_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

# 安装命令
install(TARGETS client
    RUNTIME DESTINATION "${PROJECT_ROOT}/bin/$<CONFIG>"  # 可执行文件安装到 bin/release 或 bin/debug
    LIBRARY DESTINATION "${PROJECT_ROOT}/lib"            # 库文件安装到 lib（无论 Debug 还是 Release）
    ARCHIVE DESTINATION "${PROJECT_ROOT}/lib"            # 静态库文件安装到 lib（无论 Debug 还是 Release）
)

# 定义调试和发布版本的静态库文件名
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(client PROPERTIES OUTPUT_NAME "client")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(client PROPERTIES OUTPUT_NAME "clientd")  # 调试版本名称
endif()