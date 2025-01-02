#include "rsacrypto.h"
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <cstring>

RsaCrypto::RsaCrypto() : m_keyPair(nullptr) {}

RsaCrypto::RsaCrypto(std::string fileName, bool isPrivate) : m_keyPair(nullptr)
{
    if (isPrivate)
    {
        initPrivateKey(fileName);
    }
    else
    {
        initPublicKey(fileName);
    }
}

RsaCrypto::~RsaCrypto()
{
    if (m_keyPair)
    {
        EVP_PKEY_free(m_keyPair);
    }
}

int RsaCrypto::parseKeyString(const std::string &keystr, bool isPublic)
{
    BIO *bio = BIO_new_mem_buf(keystr.data(), keystr.size());
    if (isPublic)
    {
        m_keyPair = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    }
    else
    {
        m_keyPair = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    }
    BIO_free(bio);

    return m_keyPair ? OK : ERROR;
}

int RsaCrypto::generateRsakey(int bits, const std::string &pub, const std::string &pri) {
    // 创建上下文对象
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        return ERROR; // 创建上下文失败
    }

    // 初始化密钥生成
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return ERROR; // 初始化失败
    }

    // 设置密钥长度
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return ERROR; // 设置密钥长度失败
    }

    // 生成密钥对
    EVP_PKEY *pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return ERROR; // 密钥生成失败
    }
    EVP_PKEY_CTX_free(ctx);

    // 将公钥写入文件
    FILE *pubFile = fopen(pub.c_str(), "wb");
    if (!pubFile) {
        EVP_PKEY_free(pkey);
        return ERROR; // 打开公钥文件失败
    }
    if (PEM_write_PUBKEY(pubFile, pkey) <= 0) {
        fclose(pubFile);
        EVP_PKEY_free(pkey);
        return ERROR; // 写入公钥失败
    }
    fclose(pubFile);

    // 将私钥写入文件
    FILE *priFile = fopen(pri.c_str(), "wb");
    if (!priFile) {
        EVP_PKEY_free(pkey);
        return ERROR; // 打开私钥文件失败
    }
    if (PEM_write_PrivateKey(priFile, pkey, nullptr, nullptr, 0, nullptr, nullptr) <= 0) {
        fclose(priFile);
        EVP_PKEY_free(pkey);
        return ERROR; // 写入私钥失败
    }
    fclose(priFile);

    EVP_PKEY_free(pkey);
    return OK; // 密钥生成成功
}


int RsaCrypto::rsaPubKeyEncrypt(const std::string &data, std::string &encData)
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(m_keyPair, nullptr);
    if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0)
    {
        return ERROR;
    }

    size_t outLen;
    EVP_PKEY_encrypt(ctx, nullptr, &outLen, reinterpret_cast<const unsigned char *>(data.data()), data.size());

    std::vector<unsigned char> outBuf(outLen);
    if (EVP_PKEY_encrypt(ctx, outBuf.data(), &outLen, reinterpret_cast<const unsigned char *>(data.data()), data.size()) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        return ERROR;
    }

    encData.assign(outBuf.begin(), outBuf.end());
    EVP_PKEY_CTX_free(ctx);
    return OK;
}

int RsaCrypto::rsaPriKeyDecrypt(const std::string &encData, std::string &data)
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(m_keyPair, nullptr);
    if (!ctx || EVP_PKEY_decrypt_init(ctx) <= 0)
    {
        return ERROR;
    }

    size_t outLen;
    EVP_PKEY_decrypt(ctx, nullptr, &outLen, reinterpret_cast<const unsigned char *>(encData.data()), encData.size());

    std::vector<unsigned char> outBuf(outLen);
    if (EVP_PKEY_decrypt(ctx, outBuf.data(), &outLen, reinterpret_cast<const unsigned char *>(encData.data()), encData.size()) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        return ERROR;
    }

    data.assign(outBuf.begin(), outBuf.end());
    EVP_PKEY_CTX_free(ctx);
    return OK;
}

int RsaCrypto::rsaSign(const std::string &data, std::string &signData, SignLevel level)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx || EVP_DigestSignInit(ctx, nullptr, EVP_get_digestbynid(level), nullptr, m_keyPair) <= 0)
    {
        return ERROR;
    }

    size_t sigLen;
    EVP_DigestSign(ctx, nullptr, &sigLen, reinterpret_cast<const unsigned char *>(data.data()), data.size());

    std::vector<unsigned char> sigBuf(sigLen);
    if (EVP_DigestSign(ctx, sigBuf.data(), &sigLen, reinterpret_cast<const unsigned char *>(data.data()), data.size()) <= 0)
    {
        EVP_MD_CTX_free(ctx);
        return ERROR;
    }

    EVP_MD_CTX_free(ctx);
    return toBase64(reinterpret_cast<const char *>(sigBuf.data()), sigLen, signData);
}

int RsaCrypto::rsaVerify(const std::string &data, const std::string &signData, bool &isValid, SignLevel level)
{
    std::string decodedSign;
    if (fromBase64(signData, decodedSign) != OK)
    {
        return ERROR;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx || EVP_DigestVerifyInit(ctx, nullptr, EVP_get_digestbynid(level), nullptr, m_keyPair) <= 0)
    {
        EVP_MD_CTX_free(ctx);
        return ERROR;
    }

    int ret = EVP_DigestVerify(ctx,
                               reinterpret_cast<const unsigned char *>(decodedSign.data()),
                               decodedSign.size(),
                               reinterpret_cast<const unsigned char *>(data.data()),
                               data.size());
    EVP_MD_CTX_free(ctx);
    isValid = (ret == 1);
    return ret == 1 ? OK : ERROR;
}

int RsaCrypto::toBase64(const char *str, int len, std::string &encoded)
{
    BIO *mem = BIO_new(BIO_s_mem());
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_push(b64, mem);

    if (BIO_write(b64, str, len) <= 0 || BIO_flush(b64) <= 0)
    {
        BIO_free_all(b64);
        return ERROR;
    }

    BUF_MEM *bufferPtr;
    BIO_get_mem_ptr(b64, &bufferPtr);

    encoded.assign(bufferPtr->data, bufferPtr->length - 1); // 去掉换行符
    BIO_free_all(b64);
    return OK;
}

int RsaCrypto::fromBase64(const std::string &str, std::string &decoded)
{
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *mem = BIO_new_mem_buf(str.data(), str.size());
    BIO_push(b64, mem);

    std::vector<char> buf(str.size());
    int decodedLen = BIO_read(b64, buf.data(), buf.size());

    if (decodedLen < 0)
    {
        BIO_free_all(b64);
        return ERROR;
    }

    decoded.assign(buf.data(), decodedLen);
    BIO_free_all(b64);
    return OK;
}

int RsaCrypto::initPublicKey(const std::string &pubfile)
{
    FILE *file = fopen(pubfile.c_str(), "r");
    if (!file)
    {
        return ERROR;
    }

    m_keyPair = PEM_read_PUBKEY(file, nullptr, nullptr, nullptr);
    fclose(file);
    return m_keyPair ? OK : ERROR;
}

int RsaCrypto::initPrivateKey(const std::string &prifile)
{
    FILE *file = fopen(prifile.c_str(), "r");
    if (!file)
    {
        return ERROR;
    }

    m_keyPair = PEM_read_PrivateKey(file, nullptr, nullptr, nullptr);
    fclose(file);
    return m_keyPair ? OK : ERROR;
}
