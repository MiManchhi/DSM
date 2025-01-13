#include "rsacrypto.h"
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <cstring>

RsaCrypto::RsaCrypto() : m_keyPair(nullptr) {}

RsaCrypto::RsaCrypto(const char* key, bool isPrivate, bool isFile) : m_keyPair(nullptr)
{
    if (isFile)
    {
        if (isPrivate)
        {
            initPrivateKey(key);
        }
        else
        {
            initPublicKey(key);
        }
    }
    else
    {
        parseKeyString(key, !isPrivate);
    }
}

RsaCrypto::~RsaCrypto()
{
    if (m_keyPair)
    {
        EVP_PKEY_free(m_keyPair);
    }
}

int RsaCrypto::parseKeyString(const char* keystr, bool isPublic)
{
    BIO *bio = BIO_new_mem_buf(keystr, -1);
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

int RsaCrypto::generateRsakey(int bits, const char* pub, const char* pri) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        return ERROR;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return ERROR;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return ERROR;
    }

    EVP_PKEY *pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return ERROR;
    }
    EVP_PKEY_CTX_free(ctx);

    FILE *pubFile = fopen(pub, "wb");
    if (!pubFile) {
        EVP_PKEY_free(pkey);
        return ERROR;
    }
    if (PEM_write_PUBKEY(pubFile, pkey) <= 0) {
        fclose(pubFile);
        EVP_PKEY_free(pkey);
        return ERROR;
    }
    fclose(pubFile);

    FILE *priFile = fopen(pri, "wb");
    if (!priFile) {
        EVP_PKEY_free(pkey);
        return ERROR;
    }
    if (PEM_write_PrivateKey(priFile, pkey, nullptr, nullptr, 0, nullptr, nullptr) <= 0) {
        fclose(priFile);
        EVP_PKEY_free(pkey);
        return ERROR;
    }
    fclose(priFile);

    EVP_PKEY_free(pkey);
    return OK;
}

int RsaCrypto::rsaPubKeyEncrypt(const char* data, int dataLen, char** encData)
{
    if (!data || dataLen <= 0 || !encData) {
        return ERROR;
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(m_keyPair, nullptr);
    if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0) {
        if (ctx) EVP_PKEY_CTX_free(ctx);
        return ERROR;
    }

    size_t outLen;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outLen, reinterpret_cast<const unsigned char *>(data), dataLen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return ERROR;
    }

    std::vector<unsigned char> outBuf(outLen);
    if (EVP_PKEY_encrypt(ctx, outBuf.data(), &outLen, reinterpret_cast<const unsigned char *>(data), dataLen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return ERROR;
    }

    EVP_PKEY_CTX_free(ctx);

    // 对加密后的数据进行 Base64 编码
    if (toBase64(reinterpret_cast<const char *>(outBuf.data()), outLen, encData) != OK) {
        return ERROR;
    }

    return OK;
}

int RsaCrypto::rsaPriKeyDecrypt(const char* encData, char** data, int& dataLen)
{
    if (!encData || !data) {
        return ERROR;
    }

    // 对加密数据进行 Base64 解码
    char* decodedEncData = nullptr;
    int decodedLen = 0;
    if (fromBase64(encData, &decodedEncData, decodedLen) != OK) {
        return ERROR;
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(m_keyPair, nullptr);
    if (!ctx || EVP_PKEY_decrypt_init(ctx) <= 0) {
        delete[] decodedEncData;
        return ERROR;
    }

    size_t outLen;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outLen, reinterpret_cast<const unsigned char *>(decodedEncData), decodedLen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        delete[] decodedEncData;
        return ERROR;
    }

    *data = new char[outLen + 1]; // +1 用于字符串结束符
    if (EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char *>(*data), &outLen, reinterpret_cast<const unsigned char *>(decodedEncData), decodedLen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        delete[] decodedEncData;
        delete[] *data;
        *data = nullptr;
        return ERROR;
    }

    (*data)[outLen] = '\0'; // 添加字符串结束符
    dataLen = static_cast<int>(outLen);
    EVP_PKEY_CTX_free(ctx);
    delete[] decodedEncData;
    return OK;
}

int RsaCrypto::rsaSign(const char* data, int dataLen, char** signData, SignLevel level)
{
    if (!data || dataLen <= 0 || !signData) {
        return ERROR;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx || EVP_DigestSignInit(ctx, nullptr, EVP_get_digestbynid(level), nullptr, m_keyPair) <= 0) {
        if (ctx) EVP_MD_CTX_free(ctx);
        return ERROR;
    }

    size_t sigLen;
    if (EVP_DigestSign(ctx, nullptr, &sigLen, reinterpret_cast<const unsigned char *>(data), dataLen) <= 0) {
        EVP_MD_CTX_free(ctx);
        return ERROR;
    }

    std::vector<unsigned char> sigBuf(sigLen);
    if (EVP_DigestSign(ctx, sigBuf.data(), &sigLen, reinterpret_cast<const unsigned char *>(data), dataLen) <= 0) {
        EVP_MD_CTX_free(ctx);
        return ERROR;
    }

    EVP_MD_CTX_free(ctx);

    // 对签名后的数据进行 Base64 编码
    if (toBase64(reinterpret_cast<const char *>(sigBuf.data()), sigLen, signData) != OK) {
        return ERROR;
    }

    return OK;
}

int RsaCrypto::rsaVerify(const char* data, int dataLen, const char* signData, bool &isValid, SignLevel level)
{
    if (!data || dataLen <= 0 || !signData) {
        return ERROR;
    }

    // 对签名数据进行 Base64 解码
    char* decodedSign = nullptr;
    int decodedLen = 0;
    if (fromBase64(signData, &decodedSign, decodedLen) != OK) {
        return ERROR;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx || EVP_DigestVerifyInit(ctx, nullptr, EVP_get_digestbynid(level), nullptr, m_keyPair) <= 0) {
        EVP_MD_CTX_free(ctx);
        delete[] decodedSign;
        return ERROR;
    }

    int ret = EVP_DigestVerify(ctx,
                               reinterpret_cast<const unsigned char *>(decodedSign),
                               decodedLen,
                               reinterpret_cast<const unsigned char *>(data),
                               dataLen);
    EVP_MD_CTX_free(ctx);
    delete[] decodedSign;
    isValid = (ret == 1);
    return ret == 1 ? OK : ERROR;
}

int RsaCrypto::toBase64(const char *str, int len, char** encoded)
{
    if (!str || len <= 0 || !encoded) {
        return ERROR;
    }

    BIO *mem = BIO_new(BIO_s_mem());
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_push(b64, mem);

    // 禁用 Base64 编码的换行符
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    if (BIO_write(b64, str, len) <= 0 || BIO_flush(b64) <= 0) {
        BIO_free_all(b64);
        return ERROR;
    }

    BUF_MEM *bufferPtr;
    BIO_get_mem_ptr(b64, &bufferPtr);

    *encoded = new char[bufferPtr->length + 1]; // +1 用于字符串结束符
    memcpy(*encoded, bufferPtr->data, bufferPtr->length);
    (*encoded)[bufferPtr->length] = '\0'; // 添加字符串结束符
    BIO_free_all(b64);
    return OK;
}

int RsaCrypto::fromBase64(const char* str, char** decoded, int& decodedLen)
{
    if (!str || !decoded) {
        return ERROR;
    }

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *mem = BIO_new_mem_buf(str, -1);
    BIO_push(b64, mem);

    // 禁用 Base64 解码的换行符
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    *decoded = new char[strlen(str)];
    decodedLen = BIO_read(b64, *decoded, strlen(str));

    if (decodedLen < 0) {
        BIO_free_all(b64);
        delete[] *decoded;
        *decoded = nullptr;
        return ERROR;
    }

    (*decoded)[decodedLen] = '\0'; // 添加字符串结束符
    BIO_free_all(b64);
    return OK;
}

int RsaCrypto::initPublicKey(const char* pubfile)
{
    if (!pubfile) {
        return ERROR;
    }

    FILE *file = fopen(pubfile, "r");
    if (!file) {
        return ERROR;
    }

    m_keyPair = PEM_read_PUBKEY(file, nullptr, nullptr, nullptr);
    fclose(file);
    return m_keyPair ? OK : ERROR;
}

int RsaCrypto::initPrivateKey(const char* prifile)
{
    if (!prifile) {
        return ERROR;
    }

    FILE *file = fopen(prifile, "r");
    if (!file) {
        return ERROR;
    }

    m_keyPair = PEM_read_PrivateKey(file, nullptr, nullptr, nullptr);
    fclose(file);
    return m_keyPair ? OK : ERROR;
}