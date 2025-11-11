#include <iostream>
#include <string>
#include "fs_state.h"
#include "fs_path.h"
#include "fs_auth.h"
#include "fs_ui.h"
#include "fs_util.h"
#include "fs_cmd_file.h"
#include "fs_cmd_dir.h"
#include "fs_cmd_misc.h"
#include "fs_persist.h"
#include "fs_editor.h"
using namespace std;

int main() {
    init();
    login();
    string command;
    while (1) {
        cout << curuser.name << "@" << curuser.name << "-FileSystem:";
        displayPath();
        getline(cin, command);
        string tmp = command;
        if (command.find_first_of(' ') != command.npos) {
            command = command.substr(0, command.find_first_of(' '));
            tmp = tmp.substr(tmp.find_first_of(' ') + 1, tmp.length() - tmp.find_first_of(' '));
        } else tmp = "";
        if (command == "help") help();
        else if (command == "mkdir") mkdir(tmp);
        else if (command == "cd") cd(tmp);
        else if (command == "touch") touch(tmp);
        else if (command == "ls") ls(tmp);
        else if (command == "gedit") gedit(tmp);
        else if (command == "rename") rename(tmp);
        else if (command == "rm") rm(tmp);
        else if (command == "cp") cp(tmp);
        else if (command == "ln") ln(tmp);           // 新增：符号/硬链接
        else if (command == "checklink") checklink(); // 新增：链接完整性检查
        else if (command == "clear") clear();
        else if (command == "su") su();
        else if (command == "exit") {
            exitFS();
            return 0;
        }
        else help2(command);
    }
    return 0;
}