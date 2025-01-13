# 项目生成路径
PROJ            = ../../bin/release/client
DEBUG_PROJ      = ../../bin/debug/client

# 静态库生成路径
RELEASE_STATIC_LIB = ../../lib/libclient.a
DEBUG_STATIC_LIB = ../../lib/libclientd.a

# 源文件路径
SRC_DIR         = .
COMMON_DIR      = ../common

# 构建目录
BUILD_DIR       = build
DEBUG_BUILD_DIR = build_debug

# 源文件
SRCS            = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(COMMON_DIR)/*.cpp)

# 目标文件，将源文件路径转化为构建目录的路径
RELEASE_OBJS    = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/src/client/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRCS))) \
                  $(patsubst $(COMMON_DIR)/%.cpp,$(BUILD_DIR)/common/%.o,$(filter $(COMMON_DIR)/%.cpp,$(SRCS)))

DEBUG_OBJS      = $(patsubst $(SRC_DIR)/%.cpp,$(DEBUG_BUILD_DIR)/src/client/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRCS))) \
                  $(patsubst $(COMMON_DIR)/%.cpp,$(DEBUG_BUILD_DIR)/common/%.o,$(filter $(COMMON_DIR)/%.cpp,$(SRCS)))

# 库源文件
LIB_SRCS        = $(filter-out %main.o, $(RELEASE_OBJS))
LIB_DEBUG_SRCS  = $(filter-out %main.o, $(DEBUG_OBJS))

# 编译器和链接器
CC              = g++
LINK            = g++
AR              = ar rcv

# 编译选项
CFLAGS          = -c -Wall -Wextra -I../common
DEBUG_FLAGS     = -g -O0
RELEASE_FLAGS   = -O2

# 库依赖项
LIBS            = -pthread -lacl_all -lssl -lcrypto

# 清理命令
RM              = rm -rf

all: release debug static_libs

# Debug版本规则
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(DEBUG_PROJ)

# Release版本规则
release: CFLAGS += $(RELEASE_FLAGS)
release: $(PROJ)

# static_libs规则
static_libs: $(RELEASE_STATIC_LIB) $(DEBUG_STATIC_LIB)

# 静态库生成规则(release)
$(RELEASE_STATIC_LIB): $(LIB_SRCS) | $(RELEASE_OBJS)
	@mkdir -p $(dir $@)
	$(AR) $@ $(LIB_SRCS)

# 静态库生成规则(debug)
$(DEBUG_STATIC_LIB): $(LIB_DEBUG_SRCS) | $(DEBUG_OBJS)
	@mkdir -p $(dir $@)
	$(AR) $@ $(LIB_DEBUG_SRCS)

# Release目标
$(PROJ): $(RELEASE_OBJS)
	$(LINK) $(RELEASE_OBJS) $(LIBS) -o $@

# Debug目标
$(DEBUG_PROJ): $(DEBUG_OBJS)
	$(LINK) $(DEBUG_OBJS) $(LIBS) -o $@

# 构建目标文件规则 (Release)
$(BUILD_DIR)/src/client/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $< -o $@

$(BUILD_DIR)/common/%.o: $(COMMON_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $< -o $@

# 构建目标文件规则 (Debug)
$(DEBUG_BUILD_DIR)/src/client/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $< -o $@

$(DEBUG_BUILD_DIR)/common/%.o: $(COMMON_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $< -o $@

# 清理规则
clean:
	$(RM) $(PROJ) $(DEBUG_PROJ) $(BUILD_DIR) $(DEBUG_BUILD_DIR) $(RELEASE_STATIC_LIB) $(DEBUG_STATIC_LIB)

clean_release:
	$(RM) $(PROJ) $(BUILD_DIR)

clean_debug:
	$(RM) $(DEBUG_PROJ) $(DEBUG_BUILD_DIR)

clean_static_libs:
	$(RM) $(RELEASE_STATIC_LIB) $(DEBUG_STATIC_LIB)

rebuild: clean all
rebuild_debug: clean_debug debug
rebuild_release: clean_release release

.PHONY: all clean rebuild clean_debug clean_release rebuild_debug rebuild_release release debug clean_static_libs