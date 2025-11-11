#pragma once
#include <string>
#include "fs_types.h"

void mkdir(std::string name);
void cd(std::string name);
void ls(std::string path);
void rename(std::string tmp);
void rm(std::string tmp);
dir* cpDir(dir *tmp);
void deletedir(dir *cur);
void cp(std::string tmp);
// 新增：创建符号链接 ln -s TARGET LINKNAME
void ln(std::string tmp);