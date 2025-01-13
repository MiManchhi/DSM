#pragma once
#include <string>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include "types.h"

// 签名级别枚举，用于指定不同的摘要算法
enum SignLevel
{
    Level1 = NID_md5,      // MD5 摘要
    Level2 = NID_sha1,     // SHA-1 摘要
    Level3 = NID_sha224,   // SHA-224 摘要
    Level4 = NID_sha256,   // SHA-256 摘要
    Level5 = NID_sha384,   // SHA-384 摘要
    Level6 = NID_sha512    // SHA-512 摘要
};

class RsaCrypto
{
public:
    RsaCrypto();
    /**
     * @brief 构造函数，通过文件路径或密钥字符串初始化公钥或私钥
     * @param key 文件路径或密钥字符串
     * @param isPrivate 是否为私钥，true 为私钥，false 为公钥
     * @param isFile 是否为文件路径，true 表示 key 是文件路径，false 表示 key 是密钥字符串
     */
    RsaCrypto(const char* key, bool isPrivate = true, bool isFile = true);

    /**
     * @brief 析构函数，释放密钥对资源
     */
    ~RsaCrypto();

    /**
     * @brief 通过解析字符串得到密钥
     * @param keystr 密钥字符串
     * @param isPublic 是否为公钥，true 为公钥，false 为私钥
     * @return 返回值 OK 表示成功，ERROR 表示失败
     */
    int parseKeyString(const char* keystr, bool isPublic = true);

    /**
     * @brief 生成 RSA 密钥对，并保存到文件
     * @param bits 密钥位数（如 2048）
     * @param pub 公钥保存文件路径
     * @param pri 私钥保存文件路径
     * @return 返回值 OK 表示成功，ERROR 表示失败
     */
    static int generateRsakey(int bits, const char* pub = "public.pem", const char* pri = "private.pem");

    /**
     * @brief 使用公钥加密数据，并对加密结果进行 Base64 编码
     * @param data 待加密的数据
     * @param dataLen 数据长度
     * @param encData 输出参数，返回加密后的数据（Base64 编码）。调用者需负责释放内存(delete)。
     * @return 返回值 OK 表示成功，ERROR 表示失败
     */
    int rsaPubKeyEncrypt(const char* data, int dataLen, char** encData);

    /**
     * @brief 使用私钥解密数据，解密前会对数据进行 Base64 解码
     * @param encData 加密后的数据（Base64 编码）
     * @param data 输出参数，返回解密后的数据。调用者需负责释放内存(delete)。
     * @param dataLen 输出参数，返回解密后的数据长度。
     * @return 返回值 OK 表示成功，ERROR 表示失败
     */
    int rsaPriKeyDecrypt(const char* encData, char** data, int& dataLen);

    /**
     * @brief 使用 RSA 私钥对数据签名，并对签名结果进行 Base64 编码
     * @param data 待签名的数据
     * @param dataLen 数据长度
     * @param signData 输出参数，返回签名数据（Base64 编码）。调用者需负责释放内存。
     * @param level 签名摘要算法级别（默认为 Level4，即 SHA-256）
     * @return 返回值 OK 表示成功，ERROR 表示失败
     */
    int rsaSign(const char* data, int dataLen, char** signData, SignLevel level = Level4);

    /**
     * @brief 使用 RSA 公钥验证签名，验证前会对签名数据进行 Base64 解码
     * @param data 原始数据
     * @param dataLen 数据长度
     * @param signData 签名数据（Base64 编码）
     * @param isValid 输出参数，返回签名验证结果，true 表示验证成功，false 表示验证失败
     * @param level 签名摘要算法级别（默认为 Level4，即 SHA-256）
     * @return 返回值 OK 表示验证成功，ERROR 表示验证失败
     */
    int rsaVerify(const char* data, int dataLen, const char* signData, bool &isValid, SignLevel level = Level4);

private:
    /**
     * @brief 使用 Base64 对数据进行编码
     * @param str 原始数据
     * @param len 数据长度
     * @param encoded 输出参数，返回编码后的字符串。调用者需负责释放内存。
     * @return 返回值 OK 表示成功，ERROR 表示失败
     */
    int toBase64(const char *str, int len, char** encoded);

    /**
     * @brief 使用 Base64 对数据进行解码
     * @param str 编码后的字符串
     * @param decoded 输出参数，返回解码后的数据。调用者需负责释放内存。
     * @param decodedLen 输出参数，返回解码后的数据长度。
     * @return 返回值 OK 表示成功，ERROR 表示失败
     */
    int fromBase64(const char* str, char** decoded, int& decodedLen);

    /**
     * @brief 初始化公钥
     * @param pubfile 公钥文件路径
     * @return 返回值 OK 表示成功，ERROR 表示失败
     */
    int initPublicKey(const char* pubfile);

    /**
     * @brief 初始化私钥
     * @param prifile 私钥文件路径
     * @return 返回值 OK 表示成功，ERROR 表示失败
     */
    int initPrivateKey(const char* prifile);

private:
    EVP_PKEY *m_keyPair; // 密钥对 (用于替代单独的公钥和私钥)
};