#include "hash.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// 构造函数
Hash::Hash(HashType type) : m_ctx(nullptr), m_md(nullptr)
{
    // 创建哈希上下文
    m_ctx = EVP_MD_CTX_new();
    if (!m_ctx)
    {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    // 根据指定的类型选择哈希算法
    switch (type)
    {
    case HashType::MD5:
        m_md = EVP_md5();
        break;
    case HashType::SHA1:
        m_md = EVP_sha1();
        break;
    case HashType::SHA224:
        m_md = EVP_sha224();
        break;
    case HashType::SHA256:
        m_md = EVP_sha256();
        break;
    case HashType::SHA384:
        m_md = EVP_sha384();
        break;
    case HashType::SHA512:
        m_md = EVP_sha512();
        break;
    default:
        throw std::invalid_argument("Invalid hash type");
    }

    // 初始化哈希算法
    if (EVP_DigestInit_ex(m_ctx, m_md, nullptr) != 1)
    {
        throw std::runtime_error("Failed to initialize digest");
    }
}

// 析构函数
Hash::~Hash()
{
    // 释放哈希上下文
    EVP_MD_CTX_free(m_ctx);
}

// 添加数据到哈希计算中
void Hash::addData(const std::string& data)
{
    if (EVP_DigestUpdate(m_ctx, data.data(), data.size()) != 1)
    {
        throw std::runtime_error("Failed to update digest");
    }
}

// 计算并返回最终的哈希值
std::string Hash::result()
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;

    // 计算最终的哈希值
    if (EVP_DigestFinal_ex(m_ctx, hash, &length) != 1)
    {
        throw std::runtime_error("Failed to finalize digest");
    }

    // 将结果转换为十六进制字符串
    std::ostringstream oss;
    for (unsigned int i = 0; i < length; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return oss.str();
}
