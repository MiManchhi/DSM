#pragma once
#include <string>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>
#include "types.h"

/**
 * AesCrypto 类
 * 该类实现了基于 AES-256-CBC 模式的加密和解密功能。
 * - 支持 16 字节、24 字节、32 字节的密钥。
 * - 加密后的数据使用 Base64 编码，方便存储和传输。
 * - 内部使用 OpenSSL 的 EVP 高级加密接口，保证兼容性和安全性。
 */
class AesCrypto {
public:
    /**
     * 构造函数
     * @param key - AES 密钥，必须是 16、24 或 32 字节长。
     */
    AesCrypto(const std::string& key);

    /**
     * 析构函数
     */
    ~AesCrypto();

    /**
     * AES-CBC 加密
     * @param plaintext - 待加密的明文字符串。
     * @return 加密后的密文字符串（Base64 编码）。
     */
    std::string aesCBCEncrypt(const std::string& plaintext);

    /**
     * AES-CBC 解密
     * @param ciphertext - 待解密的密文字符串（Base64 编码）。
     * @return 解密后的明文字符串。
     */
    std::string aesCBCDecrypt(const std::string& ciphertext);

    /**
     * 生成对称加密密钥
     * @param keyLength - 密钥长度 -只能是16，24，32
     * @param key - 返回参数 生成的密钥
     * @return OK 生成成功， ERROR生成失败
     */
    static int generateKey(int keyLength, std::string &key);

private:
    /**
     * 将二进制数据转换为 Base64 编码字符串。
     * @param str - 待编码的数据。
     * @param len - 数据长度。
     * @return Base64 编码后的字符串。
     */
    std::string toBase64(const unsigned char* str, int len);

    /**
     * 将 Base64 编码的字符串解码为二进制数据。
     * @param str - Base64 编码的字符串。
     * @return 解码后的二进制数据。
     */
    std::string fromBase64(const std::string& str);

    std::string m_key; // AES 密钥
};
