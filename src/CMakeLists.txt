cmake_minimum_required(VERSION 3.0)

# 设置项目根目录路径
set(PROJECT_ROOT $ENV{HOME}/DSM)

# 默认构建类型为 Release，如果未显式指定
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE release)
endif()

# 指定输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_ROOT}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_ROOT}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_ROOT}/lib)

# 包含每个模块的子目录
add_subdirectory(tracker)
add_subdirectory(http)
add_subdirectory(id)
# add_subdirectory(encrypt)
add_subdirectory(storage)

# 对于 client 模块，确保它能够选择编译为静态库
add_subdirectory(client)

# 安装操作
# 安装所有生成的可执行文件到 bin 目录
install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
        DESTINATION ${PROJECT_ROOT}/bin/${CMAKE_BUILD_TYPE}
        FILES_MATCHING PATTERN "*.exe" PATTERN "*.out")

# 安装所有生成的库文件到 lib 目录
install(DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        DESTINATION ${PROJECT_ROOT}/lib
        FILES_MATCHING PATTERN "*.a" PATTERN "*.so" PATTERN "*.dll" PATTERN "*.lib")

install(DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/
        DESTINATION ${PROJECT_ROOT}/lib
        FILES_MATCHING PATTERN "*.a" PATTERN "*.so" PATTERN "*.dll" PATTERN "*.lib")

# 添加卸载目标
if(EXISTS "${CMAKE_SOURCE_DIR}/Uninstall.cmake")
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/Uninstall.cmake
        COMMENT "Uninstalling the project"
    )
endif()