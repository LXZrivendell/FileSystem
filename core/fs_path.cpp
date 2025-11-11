#include "fs_path.h"
#include "fs_state.h"
#include "fs_types.h"
#include <string>
#include <vector>

std::vector<std::string> split(std::string str) {
    std::vector<std::string> ans;
    std::string tmp = "";
    for (int i = 0; i < (int)str.length(); i++) {
        if (str[i] != '/') {
            tmp = tmp + str[i];
        } else {
            ans.push_back(tmp);
            tmp = "";
        }
    }
    if (tmp != "") ans.push_back(tmp);
    return ans;
}

static dir* traverseWithLinks(dir* start, const std::vector<std::string>& parts, int depth_limit = 16) {
    dir* cur = start;
    int depth = 0;
    for (int i = 0; i < (int)parts.size(); i++) {
        const std::string& seg = parts[i];
        if (seg.empty() || seg == ".") continue;
        if (seg == "..") {
            if (cur == root) return NULL;
            cur = cur->pre;
            continue;
        }
        // 目录直接进入
        if (cur->next.find(seg) != cur->next.end()) {
            cur = cur->next[seg];
            continue;
        }
        // 链接：仅当链接目标为目录时可作为路径段进入
        auto lit = cur->links.find(seg);
        if (lit != cur->links.end()) {
            symlink* sl = lit->second;
            if (!sl->to_dir) return NULL;
            if (++depth > depth_limit) return NULL; // 防循环
            dir* resolved = resolveDirTarget(sl->target, cur);
            if (!resolved) return NULL;
            cur = resolved;
            continue;
        }
        return NULL;
    }
    return cur;
}

dir* pathTrans(std::string path) {
    std::string tmp = path;
    if (path[0] == '~' || path[0] == '/') {
        dir *start = root;
        if (path[0] == '/') path = "~" + path;
        std::vector<std::string> parts = split(path);
        // 绝对路径去掉 "~"
        std::vector<std::string> tail(parts.begin() + 1, parts.end());
        return traverseWithLinks(start, tail);
    } else {
        std::vector<std::string> parts = split(path);
        return traverseWithLinks(curdir, parts);
    }
}

dir* pathTransFrom(dir* base, std::string path) {
    if (path.empty()) return base;
    if (path[0] == '~' || path[0] == '/') {
        if (path[0] == '/') path = "~" + path;
        std::vector<std::string> parts = split(path);
        std::vector<std::string> tail(parts.begin() + 1, parts.end());
        return traverseWithLinks(root, tail);
    } else {
        std::vector<std::string> parts = split(path);
        return traverseWithLinks(base, parts);
    }
}

dir* resolveDirTarget(std::string target, dir* base) {
    return pathTransFrom(base, target);
}

bool isBrokenDirLink(const std::string& target, dir* base) {
    return resolveDirTarget(target, base) == NULL;
}

bool isBrokenFileLink(const std::string& target, dir* base) {
    std::string t = target;
    std::string fname;
    dir* parent = NULL;

    size_t pos = t.find_last_of('/');
    if (!t.empty() && (t[0] == '/' || t[0] == '~')) {
        if (t[0] == '/') t = "~" + t;
        if (pos == std::string::npos) {
            // "~name" 视为根下文件
            parent = root;
            fname = t.substr(1);
        } else {
            std::string dpart = t.substr(0, pos);
            fname = t.substr(pos + 1);
            parent = resolveDirTarget(dpart, root);
        }
    } else {
        if (pos == std::string::npos) {
            parent = base;
            fname = t;
        } else {
            std::string dpart = t.substr(0, pos);
            fname = t.substr(pos + 1);
            parent = resolveDirTarget(dpart, base);
        }
    }
    if (!parent) return true;
    return parent->files.find(fname) == parent->files.end();
}

// 新增：解析目标文件指针
file* resolveFileTarget(std::string target, dir* base) {
    std::string t = target;
    std::string fname;
    dir* parent = NULL;

    size_t pos = t.find_last_of('/');
    if (!t.empty() && (t[0] == '/' || t[0] == '~')) {
        if (t[0] == '/') t = "~" + t;
        if (pos == std::string::npos) {
            parent = root;
            fname = t.substr(1);
        } else {
            std::string dpart = t.substr(0, pos);
            fname = t.substr(pos + 1);
            parent = resolveDirTarget(dpart, root);
        }
    } else {
        if (pos == std::string::npos) {
            parent = base;
            fname = t;
        } else {
            std::string dpart = t.substr(0, pos);
            fname = t.substr(pos + 1);
            parent = resolveDirTarget(dpart, base);
        }
    }
    if (!parent) return nullptr;
    auto it = parent->files.find(fname);
    if (it == parent->files.end()) return nullptr;
    return it->second;
}