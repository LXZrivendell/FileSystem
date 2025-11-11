#include "fs_cmd_file.h"
#include "fs_state.h"
#include "fs_util.h"
#include "fs_path.h"
#include "fs_editor.h"
#include <iostream>
#include <fstream>
#include <cstdio>
using namespace std;

void touch(string name) {
    if (name == "") {
        cout << "Require Parameters" << endl;
    } else if (curdir->files.find(name) != curdir->files.end()) {
        cout << "There is a same file.\n";
    } else if (!judgeName(name)) {
        cout << "Name has at least a illegal character.\n";
    } else {
        file *tmp = new file();
        tmp->name = name;
        tmp->owner = curuser;
        tmp->content.push_back("");
        tmp->link_count = 1;  // 新增：初始引用计数
        curdir->files[name] = tmp;
    }
}

void gedit(string name) {
    dir *t = curdir;
    if (name == "") {
        cout << "Require Parameters" << endl;
        return;
    } else if (name.find_last_of('/') != name.npos) {
        string tmp = name;
        if (tmp[0] == '/') tmp = "~" + tmp;
        tmp = tmp.substr(0, tmp.find_last_of('/'));
        name = name.substr(name.find_last_of('/') + 1, name.length() - name.find_last_of('/'));
        curdir = pathTrans(tmp);
        if (curdir == NULL) {
            curdir = t;
            printf("Wrong Path!\n");
            return;
        }
    }
    if (curdir->files.find(name) == curdir->files.end()) {
        cout << "No Such File.\n";
    } else if (curdir->files[name]->owner.name != curuser.name) {
        cout << "This file is not yours!\n";
    } else {
        ofstream out("tmp.dat");
        for (int i = 0; i < (int)curdir->files[name]->content.size(); i++) {
        out << curdir->files[name]->content[i] << endl;
    }
        out.close();
        system("gedit tmp.dat");
        ifstream in("tmp.dat");
        string tline;
        curdir->files[name]->content.clear();
        while (getline(in, tline)) {
            curdir->files[name]->content.push_back(tline);
        }
    }
    curdir = t;
}