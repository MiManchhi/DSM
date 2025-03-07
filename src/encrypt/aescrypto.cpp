#include "aescrypto.h"
#include <cstring>
#include <stdexcept>

// 构造函数：初始化 AES 密钥
AesCrypto::AesCrypto(const char* base64Key, int base64KeyLen) {
    // Base64 解码
    unsigned char* decodedKey = nullptr;
    int decodedKeyLen = 0;
    if (fromBase64(base64Key, base64KeyLen, decodedKey, decodedKeyLen) != OK) {
        throw std::invalid_argument("Base64 密钥解码失败。");
    }

    // 检查解码后的密钥长度
    if (decodedKeyLen != 16 && decodedKeyLen != 24 && decodedKeyLen != 32) {
        delete[] decodedKey;
        throw std::invalid_argument("解码后的密钥长度无效。");
    }

    // 保存解码后的二进制密钥
    m_key = new char[decodedKeyLen];
    memcpy(m_key, decodedKey, decodedKeyLen);
    m_keyLen = decodedKeyLen;

    delete[] decodedKey;
}

// 析构函数：释放资源
AesCrypto::~AesCrypto() {
    delete[] m_key;
}

// 加密数据并进行 Base64 编码
int AesCrypto::encrypt(const char* plaintext, int plaintextLen, char*& ciphertext, int& ciphertextLen) {
    // 创建加密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return ERROR;
    }

    // 生成随机 IV（初始化向量）
    unsigned char iv[16];
    if (!RAND_bytes(iv, sizeof(iv))) {
        EVP_CIPHER_CTX_free(ctx);
        return ERROR;
    }

    // 初始化加密操作
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (unsigned char*)m_key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return ERROR;
    }

    // 分配足够的缓冲区存储加密结果
    int outLen1 = 0, outLen2 = 0;
    int cipherLen = plaintextLen + EVP_CIPHER_block_size(EVP_aes_256_cbc());
    unsigned char* encryptedData = new unsigned char[cipherLen];

    // 加密数据
    if (EVP_EncryptUpdate(ctx, encryptedData, &outLen1, (unsigned char*)plaintext, plaintextLen) != 1 ||
        EVP_EncryptFinal_ex(ctx, encryptedData + outLen1, &outLen2) != 1) {
        delete[] encryptedData;
        EVP_CIPHER_CTX_free(ctx);
        return ERROR;
    }

    cipherLen = outLen1 + outLen2;

    // 将 IV 和密文拼接
    unsigned char* combinedData = new unsigned char[cipherLen + 16];
    memcpy(combinedData, iv, 16);
    memcpy(combinedData + 16, encryptedData, cipherLen);
    delete[] encryptedData;

    // 对加密数据进行 Base64 编码
    int base64Len = 0;
    char* base64Text = nullptr;
    if (toBase64(combinedData, cipherLen + 16, base64Text, base64Len) != OK) {
        delete[] combinedData;
        EVP_CIPHER_CTX_free(ctx);
        return ERROR;
    }

    ciphertext = base64Text;
    ciphertextLen = base64Len;

    delete[] combinedData;
    EVP_CIPHER_CTX_free(ctx);
    return OK;
}

// 解密 Base64 编码的数据
int AesCrypto::decrypt(const char* ciphertext, int ciphertextLen, char*& plaintext, int& plaintextLen) {
    // Base64 解码
    unsigned char* combinedData = nullptr;
    int combinedLen = 0;
    if (fromBase64(ciphertext, ciphertextLen, combinedData, combinedLen) != OK) {
        return ERROR;
    }

    // 提取 IV 和密文
    if (combinedLen < 16) {
        delete[] combinedData;
        return ERROR;
    }
    unsigned char iv[16];
    memcpy(iv, combinedData, 16);
    unsigned char* encryptedData = combinedData + 16;
    int encryptedLen = combinedLen - 16;

    // 创建解密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        delete[] combinedData;
        return ERROR;
    }

    // 初始化解密操作
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (unsigned char*)m_key, iv) != 1) {
        delete[] combinedData;
        EVP_CIPHER_CTX_free(ctx);
        return ERROR;
    }

    // 分配足够的缓冲区存储解密结果
    int outLen1 = 0, outLen2 = 0;
    plaintext = new char[encryptedLen];
    if (EVP_DecryptUpdate(ctx, (unsigned char*)plaintext, &outLen1, encryptedData, encryptedLen) != 1 ||
        EVP_DecryptFinal_ex(ctx, (unsigned char*)(plaintext + outLen1), &outLen2) != 1) {
        delete[] combinedData;
        delete[] plaintext;
        EVP_CIPHER_CTX_free(ctx);
        return ERROR;
    }

    plaintextLen = outLen1 + outLen2;
    plaintext[plaintextLen] = '\0'; // 添加字符串结束符

    delete[] combinedData;
    EVP_CIPHER_CTX_free(ctx);
    return OK;
}

// 生成对称加密密钥并输出 Base64 编码的密钥
int AesCrypto::generateKey(int keyLength, char*& base64Key, int& base64KeyLen) {
    if (keyLength != 16 && keyLength != 24 && keyLength != 32) {
        return ERROR;
    }

    // 生成二进制密钥
    char* key = new char[keyLength];
    if (RAND_bytes((unsigned char*)key, keyLength) != 1) {
        delete[] key;
        return ERROR;
    }

    // 将二进制密钥编码为 Base64
    if (toBase64((unsigned char*)key, keyLength, base64Key, base64KeyLen) != OK) {
        delete[] key;
        return ERROR;
    }

    delete[] key;
    return OK;
}

// 将二进制数据转换为 Base64 编码字符串
int AesCrypto::toBase64(const unsigned char* data, int dataLen, char*& base64Text, int& base64Len) {
    BIO* mem = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_push(b64, mem);

    BIO_write(b64, data, dataLen);
    BIO_flush(b64);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(b64, &bufferPtr);

    base64Text = new char[bufferPtr->length];
    memcpy(base64Text, bufferPtr->data, bufferPtr->length - 1); // 去掉最后的换行符
    base64Text[bufferPtr->length - 1] = '\0'; // 添加字符串结束符
    base64Len = bufferPtr->length - 1;

    BIO_free_all(b64);
    return OK;
}

// 将 Base64 编码的字符串解码为二进制数据
int AesCrypto::fromBase64(const char* base64Text, int base64Len, unsigned char*& data, int& dataLen) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new_mem_buf(base64Text, base64Len);
    BIO_push(b64, mem);

    data = new unsigned char[base64Len];
    dataLen = BIO_read(b64, data, base64Len);
    if (dataLen < 0) {
        delete[] data;
        BIO_free_all(b64);
        return ERROR;
    }

    BIO_free_all(b64);
    return OK;
}