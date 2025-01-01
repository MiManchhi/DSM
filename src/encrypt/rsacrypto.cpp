#include "rsacrypto.h"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <cstring>
#include <stdexcept>

// 构造函数
RsaCrypto::RsaCrypto() : m_keyPair(nullptr) {}

// 析构函数
RsaCrypto::~RsaCrypto() {
    if (m_keyPair) {
        EVP_PKEY_free(m_keyPair);
    }
}

// 解析密钥字符串
void RsaCrypto::parseKeyString(const std::string &key, bool isPublic) {
    BIO *bio = BIO_new_mem_buf(key.data(), key.size());
    if (!bio) {
        throw std::runtime_error("Failed to create BIO");
    }

    if (isPublic) {
        m_keyPair = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    } else {
        m_keyPair = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    }

    BIO_free(bio);

    if (!m_keyPair) {
        throw std::runtime_error("Failed to parse key string");
    }
}

// 生成密钥对
void RsaCrypto::generateRsakey(int bits, const std::string &pubKeyPath, const std::string &priKeyPath) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_PKEY_CTX");
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize key generation");
    }

    if (EVP_PKEY_keygen(ctx, &m_keyPair) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to generate RSA key");
    }

    EVP_PKEY_CTX_free(ctx);

    // 写入公钥
    FILE *pubFile = fopen(pubKeyPath.c_str(), "w");
    if (!pubFile || PEM_write_PUBKEY(pubFile, m_keyPair) <= 0) {
        if (pubFile) fclose(pubFile);
        throw std::runtime_error("Failed to write public key");
    }
    fclose(pubFile);

    // 写入私钥
    FILE *priFile = fopen(priKeyPath.c_str(), "w");
    if (!priFile || PEM_write_PrivateKey(priFile, m_keyPair, nullptr, nullptr, 0, nullptr, nullptr) <= 0) {
        if (priFile) fclose(priFile);
        throw std::runtime_error("Failed to write private key");
    }
    fclose(priFile);
}

// 使用公钥加密
std::string RsaCrypto::rsaPubKeyEncrypt(const std::string &data) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(m_keyPair, nullptr);
    if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0) {
        if (ctx) EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }

    size_t outLen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outLen, reinterpret_cast<const unsigned char *>(data.data()), data.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to determine encrypted size");
    }

    std::string encrypted(outLen, '\0');
    if (EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char *>(&encrypted[0]), &outLen, reinterpret_cast<const unsigned char *>(data.data()), data.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to encrypt data");
    }

    EVP_PKEY_CTX_free(ctx);
    return encrypted;
}

// 使用私钥解密
std::string RsaCrypto::rsaPriKeyDecrypt(const std::string &encrypted) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(m_keyPair, nullptr);
    if (!ctx || EVP_PKEY_decrypt_init(ctx) <= 0) {
        if (ctx) EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }

    size_t outLen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outLen, reinterpret_cast<const unsigned char *>(encrypted.data()), encrypted.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to determine decrypted size");
    }

    std::string decrypted(outLen, '\0');
    if (EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char *>(&decrypted[0]), &outLen, reinterpret_cast<const unsigned char *>(encrypted.data()), encrypted.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to decrypt data");
    }

    EVP_PKEY_CTX_free(ctx);
    return decrypted;
}
