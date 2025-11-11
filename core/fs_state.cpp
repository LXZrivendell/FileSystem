#include "fs_state.h"
#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> records;
int reco = 0;
user curuser;
dir* curdir = nullptr;
dir* root = nullptr;

void initDir() {
    curdir = new dir();
    curdir->name = "~";
    curdir->pre = NULL;
    root = curdir;
}

void displayPath() {
    dir* tmp = curdir;
    std::vector<std::string> path;
    while (tmp != NULL) {
        path.push_back(tmp->name);
        tmp = tmp->pre;
    }
    std::cout << "~";
    for (int i = (int)path.size() - 2; i >= 0; i--) {
        std::cout << "/" << path[i];
    }
    std::cout << "$ ";
}