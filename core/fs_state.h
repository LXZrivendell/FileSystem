#pragma once
#include <string>
#include <vector>
#include "fs_types.h"

extern user curuser;
extern dir* curdir;
extern dir* root;
extern std::vector<std::string> records;
extern int reco;

void initDir();
void displayPath();