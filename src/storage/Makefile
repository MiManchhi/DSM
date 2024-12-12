# 项目生成路径
PROJ       = ../../bin/storage
# 源文件路径
SRC_DIR    = .
COMMON_DIR = ../common
# 构建目录
BUILD_DIR  = build
# 源文件
SRCS       = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(COMMON_DIR)/*.cpp)
# 目标文件，将源文件路径转化为构建目录的路径
OBJS       = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/src/storage/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRCS))) \
             $(patsubst $(COMMON_DIR)/%.cpp,$(BUILD_DIR)/common/%.o,$(filter $(COMMON_DIR)/%.cpp,$(SRCS)))

# 编译器
CC         = g++
# 连接器
LINK       = g++
# 编译选项
CFLAGS     = -c -Wall -Wextra -I../common
# 库依赖项
LIBS       = -pthread -lacl_all `mysql_config --libs`

RM         = rm -rf

all: $(PROJ)

$(PROJ): $(OBJS)
	$(LINK) $^ $(LIBS) -o $@

# 构建目标文件规则
$(BUILD_DIR)/src/storage/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/common/%.o: $(COMMON_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

# 清理规则
clean:
	$(RM) $(PROJ) $(BUILD_DIR)

rebuild: clean all

.PHONY: all clean rebuild
