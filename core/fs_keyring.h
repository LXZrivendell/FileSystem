#pragma once
#include <string>

void keyringInit();
void keyringSave();
std::string keyringGet(const std::string& user);
void keyringSet(const std::string& user, const std::string& key);