#pragma once
#include <openssl/evp.h>
#include <string>
#include <vector>

// 定义哈希算法类型
enum class HashType
{
    MD5,
    SHA1,
    SHA224,
    SHA256,
    SHA384,
    SHA512
};

// 哈希类，提供对多种哈希算法的支持
class Hash
{
public:
    // 构造函数，指定哈希算法类型
    Hash(HashType type);
    // 析构函数
    ~Hash();

    // 添加数据到哈希计算中
    void addData(const std::string& data);
    // 计算并返回最终的哈希值（十六进制字符串形式）
    std::string result();

private:
    HashType m_type;                  // 哈希算法类型
    EVP_MD_CTX* m_ctx;                // 哈希上下文
    const EVP_MD* m_md;               // 哈希算法指针
};
