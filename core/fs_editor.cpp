#include "fs_editor.h"
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

static int run(const std::string& cmd) {
    return std::system(cmd.c_str());
}

bool openEditor(const std::string& path) {
    // 1) 优先使用 VISUAL/EDITOR
    if (const char* visual = std::getenv("VISUAL")) {
        if (run(std::string(visual) + " " + path) == 0) return true;
    }
    if (const char* editor = std::getenv("EDITOR")) {
        if (run(std::string(editor) + " " + path) == 0) return true;
    }

    // 2) 有桌面且安装了 gedit，则尝试 gedit，并静默掉告警输出
    if (std::getenv("DISPLAY")) {
        if (run("command -v gedit >/dev/null 2>&1") == 0) {
            if (run("gedit " + path + " 2>/dev/null") == 0) return true;
        }
    }

    // 3) 终端编辑器候选
    const char* cliEditors[] = {"nano", "vi", "vim"};
    for (const char* ed : cliEditors) {
        if (run(std::string("command -v ") + ed + " >/dev/null 2>&1") == 0) {
            if (run(std::string(ed) + " " + path) == 0) return true;
        }
    }

    // 4) 兜底：内置简单行编辑器（无外部依赖）
    std::cout << "进入内置编辑器：逐行输入文本。\n";
    std::cout << "输入 ':wq' 保存退出，输入 ':q' 放弃修改。\n";
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == ":wq") break;
        if (line == ":q") {
            return false; // 放弃编辑
        }
        lines.push_back(line);
    }
    std::ofstream out(path);
    for (const auto& l : lines) out << l << "\n";
    return true;
}