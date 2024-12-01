//公共模块
//声明几个实用函数
#pragma once

#include "types.h"
#include "proto.h"

#include <string>
#include <vector>

//long long类型整数主机序转网络序
void llton(long long ll,char* n);
//网络序转long long类型主机序
long long ntoll(char const *n);
//long类型整数主机序转网络序
void lton(long l,char* n);
//网络序转long类型主机序
long ntol(char const *n);
//short类型整数主机序转网络序
void ston(short s,char* n);
//网络序转short类型主机序
short ntos(char const *n);
//判断字符串是否合法（A-Z，a-z,0-9）
int valid(char const* str);
//拆分字符串
int splitstring(char const*str,std::vector<std::string>& substrs);