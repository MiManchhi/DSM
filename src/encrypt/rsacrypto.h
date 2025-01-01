#pragma once
#include <string>
#include <openssl/evp.h>
#include <openssl/pem.h>

// 签名级别枚举
enum SignLevel
{
    Level1 = NID_md5,
    Level2 = NID_sha1,
    Level3 = NID_sha224,
    Level4 = NID_sha256,
    Level5 = NID_sha384,
    Level6 = NID_sha512
};

class RsaCrypto
{
public:
    RsaCrypto();
    RsaCrypto(std::string fileName, bool isPrivate = true);
    ~RsaCrypto();

    // 通过解析字符串得到密钥
    void parseKeyString(const std::string &keystr, bool isPublic = true);
    // 生成RSA密钥对
    void generateRsakey(int bits, const std::string &pub = "public.pem", const std::string &pri = "private.pem");
    // 公钥加密
    std::string rsaPubKeyEncrypt(const std::string &data);
    // 私钥解密
    std::string rsaPriKeyDecrypt(const std::string &encData);
    // 使用RSA签名
    std::string rsaSign(const std::string &data, SignLevel level = Level3);
    // 使用RSA验证签名
    bool rsaVerify(const std::string &data, const std::string &signData, SignLevel level = Level3);

private:
    // base64编码
    std::string toBase64(const char *str, int len);
    // base64解码
    char *fromBase64(const std::string &str);
    // 初始化公钥
    bool initPublicKey(const std::string &pubfile);
    // 初始化私钥
    bool initPrivateKey(const std::string &prifile);

private:
    EVP_PKEY *m_keyPair; // 密钥对 (用于替代单独的公钥和私钥)
};
