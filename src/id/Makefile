#项目生成路径
PROJ       = ../../bin/id
#源文件路径
SRC_DIR    = .
COMMON_DIR = ../common
#构建路径
BUILD_DIR  = build
#源文件
SRCS       = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(COMMON_DIR)/*.cpp)
#目标文件（中间文件）
OBJS       = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/src/id/%.o, $(filter $(SRC_DIR)/%.cpp, $(SRCS))) \
	         $(patsubst $(COMMON_DIR)/%.cpp, $(BUILD_DIR)/common/%.o, $(filter $(COMMON_DIR)/%.cpp, $(SRCS)))
#编译器
CC         = g++
#连接器
LINK       = g++
#依赖库
LIBS       = -pthread -lacl_all `mysql_config --libs`
#编译选项
CFLAGS     = -c -Wall -I../common

RM         = rm -rf

all:$(PROJ)

$(PROJ):$(OBJS)
	$(LINK) $^ $(LIBS) -o $@

$(BUILD_DIR)/common/%.o:$(COMMON_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/src/id/%.o:$(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJS) $(PROJ)

rebuild:clean all

.PHONY: all clean rebuild
