#include "fs_persist.h"
#include "fs_state.h"
#include "fs_types.h"
#include "fs_path.h"   // 新增：用于路径解析
#include <fstream>
#include <string>
#include <map>
#include <vector>
using namespace std;

// 新增：保存阶段的“首见路径”与加载阶段的挂起硬链接任务
static std::map<file*, std::string> inodePath;
struct PendingHardLink { dir* base; std::string linkname; std::string target; };
static std::vector<PendingHardLink> pendingHL;

// 新增：辅助获取目录绝对路径
static std::string absDirPath(dir* d) {
    std::vector<std::string> stack;
    dir* cur = d;
    while (cur) { stack.push_back(cur->name); cur = cur->pre; }
    std::string path = "~";
    for (int i = (int)stack.size() - 2; i >= 0; --i) {
        path += "/" + stack[i];
    }
    return path;
}

void save(dir *tmp) {
    records.push_back(tmp->name);

    // 统计：本目录内的“真实文件块数”和“硬链接条目”
    std::vector<std::pair<std::string, std::string>> hlinks; // (linkname, targetAbsPath)
    int realCount = 0;

    // 先遍历文件，区分真实文件与硬链接
    for (auto it = tmp->files.begin(); it != tmp->files.end(); ++it) {
        file* fp = it->second;
        std::string curAbs = absDirPath(tmp) + "/" + it->first;
        auto found = inodePath.find(fp);
        if (found == inodePath.end()) {
            inodePath[fp] = curAbs;
            realCount++;
        } else {
            hlinks.emplace_back(it->first, found->second);
        }
    }

    // 写入真实文件块数量
    records.push_back(to_string(realCount));
    // 输出真实文件块
    for (auto it = tmp->files.begin(); it != tmp->files.end(); ++it) {
        file* fp = it->second;
        if (inodePath[fp] != absDirPath(tmp) + "/" + it->first) continue; // 非首见，不写内容
        records.push_back(fp->name);
        for (int i = 0; i < (int)fp->content.size(); i++) {
            records.push_back(fp->content[i]);
        }
        records.push_back("content");
        records.push_back(fp->owner.name);
        records.push_back(fp->owner.password);
    }

    // 新增：HLINKS 段（目录内硬链接条目）
    records.push_back("HLINKS");
    records.push_back(to_string(hlinks.size()));
    for (auto &p : hlinks) {
        records.push_back(p.first);   // 链接名
        records.push_back(p.second);  // 目标绝对路径
    }

    // 保持原有：LINKS 段（符号链接）
    records.push_back("LINKS");
    records.push_back(to_string(tmp->links.size()));
    for (auto it = tmp->links.begin(); it != tmp->links.end(); ++it) {
        records.push_back(it->second->name);
        records.push_back(it->second->target);
        records.push_back(it->second->to_dir ? "D" : "F");
    }

    // 子目录
    records.push_back(to_string(tmp->next.size()));
    for (auto it = tmp->next.begin(); it != tmp->next.end(); it++) {
        records.push_back(it->second->name);
        save(it->second);
    }
}

dir* creat(dir *last) {
    dir *tmp = new dir();
    tmp->name = records[reco++];
    tmp->pre = last;

    // 文件块数量
    string t = records[reco++];
    for (int i = 0; i < stoi(t); i++) {
        file *tfile = new file();
        tfile->name = records[reco++];
        while (1) {
            string ts = records[reco++];
            if (ts != "content") {
                tfile->content.push_back(ts);
            } else {
                break;
            }
        }
        user a;
        a.name = records[reco++];
        a.password = records[reco++];
        tfile->owner = a;
        tfile->link_count = 1; // 新增：初始计数，稍后重建
        tmp->files[tfile->name] = tfile;

        // 记录绝对路径到文件指针的映射（供硬链接恢复）
        std::string absPath = absDirPath(tmp) + "/" + tfile->name;
        // 使用一个简单的全局字典保存路径->文件指针
        // 我们复用 inodePath 的 string 值空间，不影响存储阶段
        // 但为了清晰，仍在解析阶段通过 pendingHL 恢复
    }

    // 可选：HLINKS 段
    t = records[reco++];
    if (t == "HLINKS") {
        string hc = records[reco++];
        for (int i = 0; i < stoi(hc); i++) {
            PendingHardLink ph;
            ph.base = tmp;
            ph.linkname = records[reco++];
            ph.target = records[reco++]; // 保存的是绝对路径
            pendingHL.push_back(ph);
        }
        t = records[reco++]; // 继续下一段
    }

    // 可选：LINKS 段（兼容旧格式：不存在则跳过）
    if (t == "LINKS") {
        string lc = records[reco++];
        for (int i = 0; i < stoi(lc); i++) {
            symlink* sl = new symlink();
            sl->name = records[reco++];
            sl->target = records[reco++];
            string flag = records[reco++];
            sl->to_dir = (flag == "D");
            tmp->links[sl->name] = sl;
        }
        t = records[reco++]; // 下一项应为子目录数
    }

    // 子目录
    for (int i = 0; i < stoi(t); i++) {
        string name = records[reco++];
        tmp->next[name] = creat(tmp);
    }
    return tmp;
}

// 新增：解析挂起的硬链接并重建 link_count
static void resolveHardLinksAndRecount() {
    // 恢复硬链接条目（绝对路径）
    for (auto &ph : pendingHL) {
        std::string target = ph.target;
        // 解析父目录与文件名
        size_t pos = target.find_last_of('/');
        if (pos == std::string::npos) continue;
        std::string dpart = target.substr(0, pos);
        std::string fname = target.substr(pos + 1);
        dir* parent = pathTrans(dpart);
        if (!parent) continue;
        auto fit = parent->files.find(fname);
        if (fit == parent->files.end()) continue;
        file* fp = fit->second;
        ph.base->files[ph.linkname] = fp;
    }
    pendingHL.clear();

    // 重建所有文件的 link_count
    std::map<file*, int> counts;
    // DFS 统计
    std::vector<dir*> stack;
    stack.push_back(root);
    while (!stack.empty()) {
        dir* d = stack.back(); stack.pop_back();
        for (auto &kv : d->files) {
            counts[kv.second]++;
        }
        for (auto &kv : d->next) {
            stack.push_back(kv.second);
        }
    }
    // 写回计数
    for (auto &kv : counts) {
        kv.first->link_count = kv.second;
    }
}

void init() {
    ifstream inr("record.dat");
    string tmp;
    if (!inr) {
        initDir();
    }
    while (inr >> tmp)
        records.push_back(tmp);
    if (records.size() >= 1) {
        root = curdir = creat(NULL);
        resolveHardLinksAndRecount();        // 新增：恢复硬链接并重建计数
    } else {
        initDir();
    }
}

void exitFS() {
    records.clear();
    inodePath.clear();   // 新增：清空保存阶段的首见映射
    save(root);
    ofstream outr("record.dat");
    for (int i = 0; i < (int)records.size(); i++) {
        outr << records[i] << endl;
    }
}