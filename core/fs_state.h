#pragma once
#include <string>
#include <vector>
#include "fs_types.h"
#include <map>

extern user curuser;
extern dir* curdir;
extern dir* root;
extern std::vector<std::string> records;
extern int reco;
extern std::map<std::string, std::string> g_keyring; // 新增：用户密钥环

void initDir();
void displayPath();