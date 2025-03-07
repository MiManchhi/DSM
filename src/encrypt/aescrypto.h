#pragma once
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include "types.h"


/**
 * AesCrypto 类
 * 支持 AES-256-CBC 加密和解密，加密后的数据使用 Base64 编码。
 * 适用于文本、文件和分块数据的加密和解密。
 * 函数参数仅用于标记执行成功（OK 或 ERROR），输入输出使用 char* 或 char**。
 */
class AesCrypto {
public:
    /**
     * 构造函数
     * @param key - AES 密钥，必须是 16、24 或 32 字节长。
     * @param keyLen - 密钥长度。
     */
    AesCrypto(const char *base64Key, int base64KeyLen);

    /**
     * 析构函数
     */
    ~AesCrypto();

    /**
     * 加密数据并进行 Base64 编码
     * @param plaintext - 待加密的数据。
     * @param plaintextLen - 待加密数据的长度。
     * @param ciphertext - 输出参数，加密后的数据（Base64 编码）。
     * @param ciphertextLen - 输出参数，加密后数据的长度。
     * @return OK 加密成功，ERROR 加密失败。
     */
    int encrypt(const char* plaintext, int plaintextLen, char*& ciphertext, int& ciphertextLen);

    /**
     * 解密 Base64 编码的数据
     * @param ciphertext - Base64 编码的加密数据。
     * @param ciphertextLen - 加密数据的长度。
     * @param plaintext - 输出参数，解密后的数据。
     * @param plaintextLen - 输出参数，解密后数据的长度。
     * @return OK 解密成功，ERROR 解密失败。
     */
    int decrypt(const char* ciphertext, int ciphertextLen, char*& plaintext, int& plaintextLen);

    /**
     * 生成对称加密密钥
     * @param keyLength - 密钥长度（16、24 或 32）。
     * @param base64key - 输出参数，生成的密钥。
     * @param base64keyLen - 输出参数，密钥的长度。
     * @return OK 生成成功，ERROR 生成失败。
     */
    static int generateKey(int keyLength, char*& base64Key, int& base64KeyLen);

private:
    /**
     * 将二进制数据转换为 Base64 编码字符串。
     * @param data - 待编码的数据。
     * @param dataLen - 数据长度。
     * @param base64Text - 输出参数，Base64 编码后的字符串。
     * @param base64Len - 输出参数，Base64 编码后的长度。
     * @return OK 编码成功，ERROR 编码失败。
     */
    static int toBase64(const unsigned char* data, int dataLen, char*& base64Text, int& base64Len);

    /**
     * 将 Base64 编码的字符串解码为二进制数据。
     * @param base64Text - Base64 编码的字符串。
     * @param base64Len - Base64 编码的长度。
     * @param data - 输出参数，解码后的二进制数据。
     * @param dataLen - 输出参数，解码后的数据长度。
     * @return OK 解码成功，ERROR 解码失败。
     */
    static int fromBase64(const char* base64Text, int base64Len, unsigned char*& data, int& dataLen);

    char* m_key; // AES 密钥
    int m_keyLen; // 密钥长度
};