# common CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

# 获取源文件
file(GLOB COMMON_SOURCES
    *.cpp
)

# 创建静态库
add_library(common STATIC ${COMMON_SOURCES})

# 设置包含路径
target_include_directories(common PUBLIC
    ${MYSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common  # 引入 common 头文件路径
)

# 链接库
target_link_libraries(common PRIVATE
    ${MYSQL_LIBRARY}
    ${ACL_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)
