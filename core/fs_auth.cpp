#include "fs_auth.h"
#include "fs_state.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <map>
using namespace std;

void login() {
    bool flag = 1;
    map<string, string> users;
    system("clear");
    cout << "\n\n\n";
    cout << "\t\t                Linux文件系统模拟器" << endl << endl;
    cout << "\t\t            ==========================" << endl << endl;
    cout << "\t\t             版本：Ubuntu 24.04 兼容版" << endl;
    cout << "\t\t             作者：Ciallo～(∠・ω< )⌒☆" << endl << endl << endl;
    ifstream in("user.dat");
    if (!in) {
        cout << "打开文件出错！即将退出！" << endl;
        cout << "按回车键继续...";
        cin.get();
        exit(1);
    }
    string tname, tpass;
    while (in >> tname >> tpass) {
        users[tname] = tpass;
    }
    printf("username:");
    cin >> tname;
    while (flag) {
        printf("password:");
        cin >> tpass;
        if (users.find(tname) == users.end()) {
            flag = 0;
        } else {
            if (users[tname] == tpass) {
                system("clear");
                curuser.name = tname;
                curuser.password = tpass;
                break;
            } else {
                printf("password is incorrect!\n");
            }
        }
    }
    if (!flag) {
        printf("This user is not exist.\nDo you want to creat a new user?(y/n):");
        char choice;
        cin >> choice;
        if (choice == 'Y' || choice == 'y') {
            users[tname] = tpass;
            curuser.name = tname;
            curuser.password = tpass;
            system("clear");
        } else {
            login();
        }
    }
    ofstream out("user.dat");
    for (auto it = users.begin(); it != users.end(); it++) {
        out << it->first << " " << it->second << endl;
    }
    getchar();
    return;
}

void su() {
    login();
}