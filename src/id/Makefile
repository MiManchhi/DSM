# 项目生成路径
PROJ            = ../../bin/release/id
DEBUG_PROJ      = ../../bin/debug/id

# 源文件路径
SRC_DIR         = .
COMMON_DIR      = ../common

# 构建目录
BUILD_DIR       = build
DEBUG_BUILD_DIR = build_debug

# 源文件
SRCS            = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(COMMON_DIR)/*.cpp)

# 目标文件，将源文件路径转化为构建目录的路径
RELEASE_OBJS    = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/src/id/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRCS))) \
                  $(patsubst $(COMMON_DIR)/%.cpp,$(BUILD_DIR)/common/%.o,$(filter $(COMMON_DIR)/%.cpp,$(SRCS)))

DEBUG_OBJS      = $(patsubst $(SRC_DIR)/%.cpp,$(DEBUG_BUILD_DIR)/src/id/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRCS))) \
                  $(patsubst $(COMMON_DIR)/%.cpp,$(DEBUG_BUILD_DIR)/common/%.o,$(filter $(COMMON_DIR)/%.cpp,$(SRCS)))

# 编译器和链接器
CC              = g++
LINK            = g++

# 编译选项
CFLAGS          = -c -Wall -Wextra -I../common
DEBUG_FLAGS     = -g -O0
RELEASE_FLAGS   = -O2

# 库依赖项
LIBS            = -pthread -lacl_all `mysql_config --libs`

# 清理命令
RM              = rm -rf

all: release debug

# Debug版本规则
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(DEBUG_PROJ)

# Release版本规则
release: CFLAGS += $(RELEASE_FLAGS)
release: $(PROJ)

# Release目标
$(PROJ): $(RELEASE_OBJS)
	$(LINK) $(RELEASE_OBJS) $(LIBS) -o $@

# Debug目标
$(DEBUG_PROJ): $(DEBUG_OBJS)
	$(LINK) $(DEBUG_OBJS) $(LIBS) -o $@

# 构建目标文件规则 (Release)
$(BUILD_DIR)/src/id/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $< -o $@

$(BUILD_DIR)/common/%.o: $(COMMON_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $< -o $@

# 构建目标文件规则 (Debug)
$(DEBUG_BUILD_DIR)/src/id/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $< -o $@

$(DEBUG_BUILD_DIR)/common/%.o: $(COMMON_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $< -o $@

# 清理规则
clean:
	$(RM) $(PROJ) $(DEBUG_PROJ) $(BUILD_DIR) $(DEBUG_BUILD_DIR)

clean_release:
	$(RM) $(PROJ) $(BUILD_DIR)

clean_debug:
	$(RM) $(DEBUG_PROJ) $(DEBUG_BUILD_DIR)

rebuild: clean all
rebuild_debug: clean_debug debug
rebuild_release: clean_release release

.PHONY: all clean rebuild clean_debug clean_release rebuild_debug rebuild_release release debug