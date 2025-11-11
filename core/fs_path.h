#pragma once
#include <string>
#include <vector>
#include "fs_types.h"

std::vector<std::string> split(std::string str);
dir* pathTrans(std::string path);
// 从指定基目录解析路径（相对/绝对均可），支持链接跟随
dir* pathTransFrom(dir* base, std::string path);
// 解析链接目标（目录）
dir* resolveDirTarget(std::string target, dir* base);
// 断链检测：目录链接是否断开
bool isBrokenDirLink(const std::string& target, dir* base);
// 断链检测：文件链接是否断开
bool isBrokenFileLink(const std::string& target, dir* base);

// 新增：解析目标为文件，成功返回文件指针，否则返回 nullptr
file* resolveFileTarget(std::string target, dir* base);