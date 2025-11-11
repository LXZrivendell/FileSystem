#pragma once
#include <string>
#include <vector>
#include <map>

struct user {
    std::string name;
    std::string password;
};

struct file {
    std::string name;
    std::vector<std::string> content;
    user owner;
    int link_count;              // 硬链接引用计数
    bool encrypted;              // 是否加密
    std::string enc_algo;        // 加密算法：XOR / AES（占位）
    std::string enc_key;         // 加密密钥（简单字符串）
};

struct symlink {
    std::string name;      // 链接名
    std::string target;    // 目标路径（支持绝对/相对）
    bool to_dir;           // 目标是目录（true）还是文件（false）
};

struct dir {
    std::string name;
    dir* pre;
    std::map<std::string, file*> files;
    std::map<std::string, dir*> next;
    std::map<std::string, symlink*> links; // 新增：符号链接集合
};