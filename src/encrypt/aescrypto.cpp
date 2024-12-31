#include <cstring>
#include "aescrypto.h"

// 构造函数：初始化 AES 密钥
AesCrypto::AesCrypto(const std::string& key) {
    if (key.size() != 16 && key.size() != 24 && key.size() != 32) {
        throw std::invalid_argument("AES 密钥长度必须是 16、24 或 32 字节。");
    }
    m_key = key;
}

// 析构函数：释放资源
AesCrypto::~AesCrypto() {}

// 加密函数：使用 AES-CBC 模式加密明文
std::string AesCrypto::aesCBCEncrypt(const std::string& plaintext) {
    // 创建加密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("创建 EVP_CIPHER_CTX 失败。");
    }

    // 生成随机 IV（初始化向量）
    unsigned char iv[16];
    if (!RAND_bytes(iv, sizeof(iv))) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("生成随机 IV 失败。");
    }

    // 初始化加密操作
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (unsigned char*)m_key.data(), iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("初始化加密失败。");
    }

    // 分配足够的缓冲区存储加密结果
    std::string ciphertext;
    ciphertext.resize(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int out_len1 = 0, out_len2 = 0;

    // 加密数据
    if (EVP_EncryptUpdate(ctx, (unsigned char*)ciphertext.data(), &out_len1,
                          (unsigned char*)plaintext.data(), plaintext.size()) != 1 ||
        EVP_EncryptFinal_ex(ctx, (unsigned char*)ciphertext.data() + out_len1, &out_len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("加密数据失败。");
    }

    ciphertext.resize(out_len1 + out_len2);

    // 将 IV 和密文拼接
    std::string encryptedData((char*)iv, sizeof(iv));
    encryptedData += ciphertext;

    // 对加密数据进行 Base64 编码
    EVP_CIPHER_CTX_free(ctx);
    return toBase64((unsigned char*)encryptedData.data(), encryptedData.size());
}

// 解密函数：使用 AES-CBC 模式解密密文
std::string AesCrypto::aesCBCDecrypt(const std::string& ciphertext) {
    // Base64 解码
    std::string decodedData = fromBase64(ciphertext);

    // 提取 IV 和密文
    if (decodedData.size() < 16) {
        throw std::invalid_argument("密文长度不足，无法提取 IV。");
    }
    unsigned char iv[16];
    memcpy(iv, decodedData.data(), sizeof(iv));
    std::string encryptedText = decodedData.substr(sizeof(iv));

    // 创建解密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("创建 EVP_CIPHER_CTX 失败。");
    }

    // 初始化解密操作
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (unsigned char*)m_key.data(), iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("初始化解密失败。");
    }

    // 分配足够的缓冲区存储解密结果
    std::string plaintext;
    plaintext.resize(encryptedText.size());
    int out_len1 = 0, out_len2 = 0;

    // 解密数据
    if (EVP_DecryptUpdate(ctx, (unsigned char*)plaintext.data(), &out_len1,
                          (unsigned char*)encryptedText.data(), encryptedText.size()) != 1 ||
        EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext.data() + out_len1, &out_len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("解密数据失败。");
    }

    plaintext.resize(out_len1 + out_len2);
    EVP_CIPHER_CTX_free(ctx);

    return plaintext;
}

// 将二进制数据转换为 Base64 编码字符串
std::string AesCrypto::toBase64(const unsigned char* str, int len) {
    BIO* mem = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_push(b64, mem);

    BIO_write(b64, str, len);
    BIO_flush(b64);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(b64, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length - 1); // 去掉最后的换行符
    BIO_free_all(b64);
    return result;
}

// 将 Base64 编码的字符串解码为二进制数据
std::string AesCrypto::fromBase64(const std::string& str) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new_mem_buf(str.data(), str.size());
    BIO_push(b64, mem);

    std::string result;
    result.resize(str.size());
    int decodedLen = BIO_read(b64, result.data(), result.size());
    if (decodedLen < 0) {
        BIO_free_all(b64);
        throw std::runtime_error("Base64 解码失败。");
    }

    result.resize(decodedLen);
    BIO_free_all(b64);
    return result;
}
