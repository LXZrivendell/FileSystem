#include "fs_cmd_file.h"
#include "fs_state.h"
#include "fs_util.h"
#include "fs_path.h"
#include "fs_editor.h"
#include "fs_crypto.h"   // 新增
#include "fs_keyring.h"  // 新增
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
        tmp->link_count = 1;     // 新增
        tmp->encrypted = false;  // 新增
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
        // 新增：透明解密后写到临时文件
        vector<string> lines;
        if (curdir->files[name]->encrypted) {
            lines = decryptForFile(curdir->files[name]->content, curdir->files[name]);
        } else {
            lines = curdir->files[name]->content;
        }
        for (int i = 0; i < (int)lines.size(); i++) {
            out << lines[i] << endl;
        }
        out.close();

        // 使用通用编辑器（更稳健）
        if (!openEditor("tmp.dat")) {
            cout << "Edit cancelled.\n";
            curdir = t;
            return;
        }

        ifstream in("tmp.dat");
        string tline;
        vector<string> edited;
        while (getline(in, tline)) {
            edited.push_back(tline);
        }
        // 新增：保存为密文或明文
        if (curdir->files[name]->encrypted) {
            curdir->files[name]->content = encryptForFile(edited, curdir->files[name]);
        } else {
            curdir->files[name]->content = edited;
        }
    }
    curdir = t;
}

// 新增：enc 命令（enc -on name ALG [KEY] | enc -off name）
void enc(string tmp) {
    if (tmp.empty()) { cout << "Require Parameters\n"; return; }

    string option = "";
    if (tmp.find_first_of('-') != tmp.npos && tmp.length() >= 3) {
        option = tmp.substr(tmp.find_first_of('-'),
                            tmp.find_first_of(' ', tmp.find_first_of('-')) - tmp.find_first_of('-'));
    } else { cout << "Wrong Option!\n"; return; }

    string name = tmp.substr(tmp.find_first_of(' ', tmp.find_first_of('-')) + 1);
    // 提取 ALG 和 KEY（-on 模式）
    string alg = "", key = "";
    if (option == "-on") {
        size_t p1 = name.find(' ');
        if (p1 == string::npos) { cout << "Require ALG\n"; return; }
        string fname = name.substr(0, p1);
        string rest  = name.substr(p1 + 1);
        size_t p2 = rest.find(' ');
        alg = (p2 == string::npos) ? rest : rest.substr(0, p2);
        key = (p2 == string::npos) ? ""   : rest.substr(p2 + 1);

        // 支持路径
        dir *t = curdir;
        if (fname.find_last_of('/') != fname.npos) {
            string tmpPath = fname;
            if (tmpPath[0] == '/') tmpPath = "~" + tmpPath;
            string dpart = tmpPath.substr(0, tmpPath.find_last_of('/'));
            fname = tmpPath.substr(tmpPath.find_last_of('/') + 1);
            curdir = pathTrans(dpart);
            if (curdir == NULL) { curdir = t; printf("Wrong Path!\n"); return; }
        }

        if (curdir->files.find(fname) == curdir->files.end()) { cout << "No Such File.\n"; curdir = t; return; }
        file* f = curdir->files[fname];
        if (f->owner.name != curuser.name) { cout << "Permission denied.\n"; curdir = t; return; }

        if (key.empty()) {
            key = keyringGet(curuser.name);
            if (key.empty()) key = curuser.password; // 回退到用户密码
        }
        if (alg != "XOR" && alg != "AES") { cout << "Unsupported ALG.\n"; curdir = t; return; }
        setEncryptionOn(f, alg, key);
        cout << "Encryption ON: " << fname << " [" << alg << "]\n";
        curdir = t;

    } else if (option == "-off") {
        string fname = name;
        dir *t = curdir;
        if (fname.find_last_of('/') != fname.npos) {
            string tmpPath = fname;
            if (tmpPath[0] == '/') tmpPath = "~" + tmpPath;
            string dpart = tmpPath.substr(0, tmpPath.find_last_of('/'));
            fname = tmpPath.substr(tmpPath.find_last_of('/') + 1);
            curdir = pathTrans(dpart);
            if (curdir == NULL) { curdir = t; printf("Wrong Path!\n"); return; }
        }
        if (curdir->files.find(fname) == curdir->files.end()) { cout << "No Such File.\n"; curdir = t; return; }
        file* f = curdir->files[fname];
        if (f->owner.name != curuser.name) { cout << "Permission denied.\n"; curdir = t; return; }

        setEncryptionOff(f);
        cout << "Encryption OFF: " << fname << "\n";
        curdir = t;
    } else {
        cout << "Wrong Option!\n";
    }
}