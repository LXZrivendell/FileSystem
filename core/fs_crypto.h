#pragma once
#include <string>
#include <vector>
#include "fs_types.h"

// 基础变换：XOR 与占位 AES
std::string xorCrypt(const std::string& data, const std::string& key);
std::string aesLikeCrypt(const std::string& data, const std::string& key); // 占位实现

// 统一变换接口（按算法选择）
std::string transform(const std::string& data, const std::string& algo, const std::string& key, bool encrypt);

// 针对 file 的行级加/解密
std::vector<std::string> encryptForFile(const std::vector<std::string>& plaintext, const file* f);
std::vector<std::string> decryptForFile(const std::vector<std::string>& ciphertext, const file* f);

// 打开/关闭加密（会转换内容）
void setEncryptionOn(file* f, const std::string& algo, const std::string& key);
void setEncryptionOff(file* f);