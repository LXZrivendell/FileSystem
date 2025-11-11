#include "fs_crypto.h"
#include <string>
#include <vector>

std::string xorCrypt(const std::string& data, const std::string& key) {
    if (key.empty()) return data;
    std::string out = data;
    for (size_t i = 0; i < data.size(); ++i) {
        out[i] = data[i] ^ key[i % key.size()];
    }
    return out;
}

// 占位 AES：简单多轮 XOR 与轮偏移，不是真正 AES，仅为演示
std::string aesLikeCrypt(const std::string& data, const std::string& key) {
    if (key.empty()) return data;
    std::string out = data;
    std::string k = key;
    for (int round = 0; round < 3; ++round) {
        for (size_t i = 0; i < out.size(); ++i) {
            out[i] = out[i] ^ k[(i + round) % k.size()];
        }
        // 轮换密钥
        if (!k.empty()) {
            char c = k[0];
            k.erase(k.begin());
            k.push_back(c);
        }
    }
    return out;
}

std::string transform(const std::string& data, const std::string& algo, const std::string& key, bool /*encrypt*/) {
    if (algo == "XOR") return xorCrypt(data, key);
    if (algo == "AES") return aesLikeCrypt(data, key); // 占位
    // 默认：原样返回
    return data;
}

std::vector<std::string> encryptForFile(const std::vector<std::string>& plaintext, const file* f) {
    std::vector<std::string> out;
    out.reserve(plaintext.size());
    for (const auto& line : plaintext) {
        out.push_back(transform(line, f->enc_algo, f->enc_key, true));
    }
    return out;
}

std::vector<std::string> decryptForFile(const std::vector<std::string>& ciphertext, const file* f) {
    std::vector<std::string> out;
    out.reserve(ciphertext.size());
    for (const auto& line : ciphertext) {
        out.push_back(transform(line, f->enc_algo, f->enc_key, false));
    }
    return out;
}

void setEncryptionOn(file* f, const std::string& algo, const std::string& key) {
    f->enc_algo = algo;
    f->enc_key  = key;
    if (!f->encrypted) {
        f->content = encryptForFile(f->content, f);
        f->encrypted = true;
    }
}

void setEncryptionOff(file* f) {
    if (f->encrypted) {
        // 将密文转换为明文再关闭加密
        std::vector<std::string> plain = decryptForFile(f->content, f);
        f->content = plain;
        f->encrypted = false;
        f->enc_algo.clear();
        f->enc_key.clear();
    }
}