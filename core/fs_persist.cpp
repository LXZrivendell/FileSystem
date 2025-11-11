#include "fs_persist.h"
#include "fs_state.h"
#include "fs_types.h"
#include "fs_path.h"   // 新增：用于路径解析
#include "fs_keyring.h"    // 新增
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
    // 新增：ENCFILES 段（记录目录下加密文件的算法与密钥）
    int encCount = 0;
    for (auto &kv : tmp->files) { if (kv.second->encrypted) encCount++; }
    records.push_back("ENCFILES");
    records.push_back(to_string(encCount));
    for (auto &kv : tmp->files) {
        file* f = kv.second;
        if (!f->encrypted) continue;
        records.push_back(kv.first);        // 文件名（目录内入口名）
        records.push_back(f->enc_algo);     // 算法
        records.push_back(f->enc_key);      // 密钥
    }

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

    string t = records[reco++];
    for (int i = 0; i < stoi(t); i++) {
        file *tfile = new file();
        tfile->name = records[reco++];
        while (1) {
            string ts = records[reco++];
            if (ts != "content") tfile->content.push_back(ts);
            else break;
        }
        user a;
        a.name = records[reco++];
        a.password = records[reco++];
        tfile->owner = a;
        tfile->link_count = 1;
        tfile->encrypted = false;  // 默认未加密
        tmp->files[tfile->name] = tfile;
    }

    // 可选：HLINKS 段
    t = records[reco++];
    if (t == "HLINKS") {
        string hc = records[reco++];
        for (int i = 0; i < stoi(hc); i++) {
            PendingHardLink ph;
            ph.base = tmp;
            ph.linkname = records[reco++];
            ph.target = records[reco++];
            pendingHL.push_back(ph);
        }
        t = records[reco++];
    }

    // 可选：LINKS 段
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
        t = records[reco++];
    }

    // 可选：ENCFILES 段
    if (t == "ENCFILES") {
        string ec = records[reco++];
        for (int i = 0; i < stoi(ec); i++) {
            string fname = records[reco++];
            string algo  = records[reco++];
            string key   = records[reco++];
            auto it = tmp->files.find(fname);
            if (it != tmp->files.end()) {
                it->second->enc_algo = algo;
                it->second->enc_key  = key;
                it->second->encrypted = true;
            }
        }
        t = records[reco++];
    }

    for (int i = 0; i < stoi(t); i++) {
        string name;
        name = records[reco++];
        tmp->next[name] = creat(tmp);
    }
    return tmp;
}

// 新增：恢复硬链接并重建引用计数
static void resolveHardLinksAndRecount() {
    // 恢复目录内挂起的硬链接条目
    for (auto &ph : pendingHL) {
        std::string target = ph.target;
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
    std::vector<dir*> stack;
    stack.push_back(root);
    while (!stack.empty()) {
        dir* d = stack.back();
        stack.pop_back();
        for (auto &kv : d->files) {
            counts[kv.second]++;
        }
        for (auto &kv : d->next) {
            stack.push_back(kv.second);
        }
    }
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
    while (inr >> tmp) records.push_back(tmp);
    if (records.size() >= 1) {
        root = curdir = creat(NULL);
        resolveHardLinksAndRecount();        // 现在有定义
    } else {
        initDir();
    }
    keyringInit();  // 新增：加载密钥环
}

void exitFS() {
    records.clear();
    inodePath.clear();
    save(root);
    ofstream outr("record.dat");
    for (int i = 0; i < (int)records.size(); i++) {
        outr << records[i] << endl;
    }
    keyringSave();  // 新增：保存密钥环
}