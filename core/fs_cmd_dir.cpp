#include "fs_cmd_dir.h"
#include "fs_state.h"
#include "fs_path.h"
#include "fs_util.h"
#include <iostream>
#include <iomanip>
#include <cstdio>
using namespace std;

void mkdir(string name) {
    if (name == "") {
        cout << "Require Parameters" << endl;
    } else if (curdir->next.find(name) != curdir->next.end()) {
        cout << "There is a directory having same name.\n";
    } else if (!judgeName(name)) {
        cout << "Name has at least a illegal character.\n";
    } else {
        dir *tmp = new dir();
        tmp->name = name;
        tmp->pre = curdir;
        curdir->next[name] = tmp;
    }
}

void cd(string name) {
    if (name == "") {
        dir* tmp = curdir;
        vector<string> path;
        while (tmp != NULL) {
            path.push_back(tmp->name);
            tmp = tmp->pre;
        }
        cout << "~";
        for (int i = (int)path.size() - 2; i >= 0; i--) {
            cout << "/" << path[i];
        }
        cout << endl;
    } else {
        dir* tmp = pathTrans(name);
        if (tmp == NULL) {
            cout << "No Such Directory.\n";
        } else {
            curdir = tmp;
        }
    }
}

void ls(string path) {
    dir *tmp = curdir;
    if (path != "") {
        curdir = pathTrans(path);
        if (curdir == NULL) {
            curdir = tmp;
            printf("Wrong Path!\n");
            return;
        }
    }
    for (auto it = curdir->files.begin(); it != curdir->files.end(); it++) {
        cout << it->first << setw(25 - (int)it->first.length()) << "<FILE>"
             << "  owner:" << it->second->owner.name;
        if (it->second->link_count > 1) {
            cout << "  links:" << it->second->link_count;  // 新增：显示硬链接计数
        }
        cout << endl;
    }
    for (auto it = curdir->next.begin(); it != curdir->next.end(); it++) {
        cout << it->first << setw(26 - (int)it->first.length()) << "<DIR>  " << endl;
    }
    // 新增：显示符号链接（用 -> 目标），并标注断链
    int linkCount = 0, brokenCount = 0;
    for (auto it = curdir->links.begin(); it != curdir->links.end(); ++it) {
        symlink* sl = it->second;
        bool broken = sl->to_dir ? isBrokenDirLink(sl->target, curdir)
                                 : isBrokenFileLink(sl->target, curdir);
        string kind = sl->to_dir ? "<LINK->DIR>" : "<LINK->FILE>";
        cout << it->first << " -> " << sl->target << " " << kind;
        if (broken) cout << " [broken]";
        cout << endl;
        linkCount++;
        brokenCount += broken ? 1 : 0;
    }
    if (linkCount > 0) {
        cout << "Symlinks: " << linkCount << " (broken " << brokenCount << ")" << endl;
    }
    curdir = tmp;
}

void rename(string tmp) {
    dir *t = curdir;
    string option = "";
    if (tmp.find_first_of('-') != tmp.npos && tmp.length() >= 2) {
        option = tmp.substr(tmp.find_first_of('-'), 2);
    } else {
        cout << "Wrong Option!\n";
        return;
    }
    string old = "", newname = "";
    old = tmp.substr(tmp.find_first_of('-') + 3, tmp.find_last_of(' ') - tmp.find_first_of('-') - 3);
    newname = tmp.substr(tmp.find_last_of(' ') + 1, tmp.length() - tmp.find_last_of(' '));
    if (!judgeName(newname)) {
        cout << "Newname has at least a illegal character.\n";
        return;
    }
    if (newname == "" || old == "" || newname == old) {
        cout << "Require Parameters" << endl;
        return;
    }
    if (old.find_last_of('/') != old.npos) {
        string tpath = old;
        if (tpath[0] == '/') tpath = "~" + tpath;
        tpath = tpath.substr(0, tpath.find_last_of('/'));
        old = old.substr(old.find_last_of('/') + 1, old.length() - old.find_last_of('/'));
        curdir = pathTrans(tpath);
        if (curdir == NULL) {
            curdir = t;
            printf("Wrong Path!\n");
            return;
        }
    }
    if (option == "-d") {
        if (curdir->next.find(old) == curdir->next.end()) {
            cout << "No Such Directory.\n";
        } else if (curdir->next.find(newname) != curdir->next.end()) {
            cout << "There is a directory having same name.\n";
        } else {
            dir *tmpd = curdir->next[old];
            curdir->next.erase(old);
            tmpd->name = newname;
            curdir->next[newname] = tmpd;
        }
    } else if (option == "-f") {
        if (curdir->files.find(old) == curdir->files.end()) {
            cout << "No Such File.\n";
        } else if (curdir->files.find(newname) != curdir->files.end()) {
            cout << "There is a same file.\n";
        } else if (curdir->files[old]->owner.name != curuser.name) {
            cout << "This file is not yours!\n";
        } else {
            file *tmpf = curdir->files[old];
            curdir->files.erase(old);
            tmpf->name = newname;
            curdir->files[newname] = tmpf;
        }
    } else {
        cout << "Wrong Option!\n";
    }
    curdir = t;
}

void deletedir(dir *cur) {
    for (auto it = cur->files.begin(); it != cur->files.end(); it++) {
        delete(it->second);
    }
    cur->files.clear();
    for (auto it = cur->next.begin(); it != cur->next.end(); it++) {
        deletedir(it->second);
    }
    cur->next.clear();
    delete(cur);
}

void rm(string tmp) {
    string option = "";
    dir *t = curdir;
    if (tmp.find_first_of('-') != tmp.npos && tmp.length() >= 2) {
        option = tmp.substr(tmp.find_first_of('-'), 2);
    } else {
        cout << "Wrong Option!\n";
        return;
    }
    string name = "";
    name = tmp.substr(tmp.find_first_of('-') + 3, tmp.length() - tmp.find_first_of('-') - 3);
    if (name.find_last_of('/') != name.npos) {
        string tpath = name;
        if (tpath[0] == '/') tpath = "~" + tpath;
        tpath = tpath.substr(0, tpath.find_last_of('/'));
        name = name.substr(name.find_last_of('/') + 1, name.length() - name.find_last_of('/'));
        curdir = pathTrans(tpath);
        if (curdir == NULL) {
            curdir = t;
            printf("Wrong Path!\n");
            return;
        }
    }
    if (name == "") {
        cout << "Require Parameters" << endl;
    } else if (option == "-d") {
        if (curdir->next.find(name) == curdir->next.end()) {
            cout << "No Such Directory.\n";
        } else {
            deletedir(curdir->next[name]);
            curdir->next.erase(name);
        }
    } else if (option == "-f") {
        if (curdir->files.find(name) == curdir->files.end()) {
            cout << "No Such File.\n";
        } else if (curdir->files[name]->owner.name != curuser.name) {
            cout << "This file is not yours!\n";
        } else {
            // 新增：硬链接删除逻辑（引用计数为0时真正删除）
            file* fp = curdir->files[name];
            curdir->files.erase(name);
            fp->link_count--;
            if (fp->link_count <= 0) {
                delete fp;
            }
        }
    } else if (option == "-l") {
        // 新增：删除符号链接
        auto lit = curdir->links.find(name);
        if (lit == curdir->links.end()) {
            cout << "No Such Link.\n";
        } else {
            delete(lit->second);
            curdir->links.erase(name);
        }
    } else {
        cout << "Wrong Option!\n";
    }
}

dir* cpDir(dir *tmp) {
    dir *goal = new dir(*tmp);
    goal->next.clear();
    goal->files.clear();
    for (auto it = tmp->files.begin(); it != tmp->files.end(); it++) {
        file *f = new file(*(it->second));
        goal->files[it->first] = f;
    }
    for (auto it = tmp->next.begin(); it != tmp->next.end(); it++) {
        dir *d = cpDir(it->second);
        d->pre = goal;
        goal->next[it->first] = d;
    }
    return goal;
}

void cp(string tmp) {
    string option = "";
    if (tmp.find_first_of('-') != tmp.npos && tmp.length() >= 3) {
        option = tmp.substr(tmp.find_first_of('-'), tmp.find_first_of(' ', tmp.find_first_of('-')) - tmp.find_first_of('-'));
    } else {
        cout << "Wrong Option!\n";
        return;
    }
    string source = "", dest = "", name = "";
    source = tmp.substr(tmp.find_first_of(' ', tmp.find_first_of('-')) + 1, tmp.find_last_of(' ') - tmp.find_first_of(' ', tmp.find_first_of('-')) - 1);
    dest = tmp.substr(tmp.find_last_of(' ') + 1, tmp.length() - tmp.find_last_of(' '));
    dir *sou = NULL, *den = NULL;
    if (source.find_first_of("/") == source.npos) {
        sou = curdir;
        name = source;
    } else {
        if (source[0] == '/') source = "~" + source;
        name = source.substr(source.find_last_of('/') + 1, source.length() - source.find_last_of('/'));
        source = source.substr(0, source.find_last_of('/'));
        sou = pathTrans(source);
    }
    den = pathTrans(dest);
    if (den == NULL) {
        printf("Wrong Path!\n");
    } else if (option == "-f") {
        if (sou == NULL) {
            printf("No Such File.\n");
            return;
        } else {
            if (den->files.find(name) != den->files.end()) {
                cout << "Destination has a same file.\n";
                return;
            } else if (sou->files.find(name) == sou->files.end()) {
                printf("No Such File.\n");
                return;
            } else if (curdir->files[name]->owner.name != curuser.name) {
                cout << "This file is not yours!\n";
            } else {
                file *tmpf = new file(*(sou->files[name]));
                tmpf->link_count = 1;           // 新增：拷贝为新文件，链接计数重置为1
                den->files[name] = tmpf;
            }
        }
    } else if (option == "-d") {
        if (sou == NULL) {
            printf("No Such Directory.\n");
        } else {
            if (den->next.find(name) != den->next.end()) {
                cout << "Destination has a same directory.\n";
                return;
            } else if (sou->next.find(name) == sou->next.end()) {
                printf("No Such Directory.\n");
                return;
            } else {
                dir *tmpd = cpDir(sou->next[name]);
                tmpd->pre = den;
                den->next[name] = tmpd;
            }
        }
    } else if (option == "-cf") {
        if (sou == NULL) {
            printf("No Such File.\n");
            return;
        } else {
            if (den->files.find(name) != den->files.end()) {
                cout << "Destination has a same file.\n";
                return;
            } else if (sou->files.find(name) == sou->files.end()) {
                printf("No Such File.\n");
                return;
            } else if (curdir->files[name]->owner.name != curuser.name) {
                cout << "This file is not yours!\n";
            } else {
                den->files[name] = sou->files[name];
                sou->files.erase(name);
            }
        }
    } else if (option == "-cd") {
        if (sou == NULL) {
            printf("No Such Directory.\n");
        } else {
            if (den->next.find(name) != den->next.end()) {
                cout << "Destination has a same directory.\n";
                return;
            } else if (sou->next.find(name) == sou->next.end()) {
                printf("No Such Directory.\n");
                return;
            } else {
                den->next[name] = sou->next[name];
                sou->next.erase(name);
            }
        }
    } else {
        cout << "Wrong Option!\n";
    }
}

// 修改：ln 支持 -s（符号链接）与 -h（硬链接）
void ln(string tmp) {
    string option = "";
    if (tmp.find_first_of('-') != tmp.npos && tmp.length() >= 3) {
        option = tmp.substr(tmp.find_first_of('-'),
                            tmp.find_first_of(' ', tmp.find_first_of('-')) - tmp.find_first_of('-'));
    } else {
        cout << "Wrong Option!\n";
        return;
    }
    string target = "", linkname = "";
    target = tmp.substr(tmp.find_first_of(' ', tmp.find_first_of('-')) + 1,
                        tmp.find_last_of(' ') - tmp.find_first_of(' ', tmp.find_first_of('-')) - 1);
    linkname = tmp.substr(tmp.find_last_of(' ') + 1, tmp.length() - tmp.find_last_of(' '));

    if (linkname == "" || !judgeName(linkname)) {
        cout << "Require Parameters" << endl;
        return;
    }
    if (curdir->files.find(linkname) != curdir->files.end()
        || curdir->next.find(linkname) != curdir->next.end()
        || curdir->links.find(linkname) != curdir->links.end()) {
        cout << "There is a same name entry.\n";
        return;
    }

    if (option == "-s") {
        // 目标类型判定（尽力判断，不存在也允许创建断链）
        bool toDir = false;
        std::string t = target;
        std::string last;
        dir* parent = NULL;
        size_t pos = t.find_last_of('/');
        if (!t.empty() && (t[0] == '/' || t[0] == '~')) {
            if (t[0] == '/') t = "~" + t;
            if (pos == std::string::npos) {
                parent = root;
                last = t.substr(1);
            } else {
                std::string dpart = t.substr(0, pos);
                last = t.substr(pos + 1);
                parent = resolveDirTarget(dpart, root);
            }
        } else {
            if (pos == std::string::npos) {
                parent = curdir;
                last = t;
            } else {
                std::string dpart = t.substr(0, pos);
                last = t.substr(pos + 1);
                parent = resolveDirTarget(dpart, curdir);
            }
        }
        if (parent && parent->next.find(last) != parent->next.end()) toDir = true;

        symlink* sl = new symlink();
        sl->name = linkname;
        sl->target = target;
        sl->to_dir = toDir;
        curdir->links[linkname] = sl;
        cout << "Create symlink: " << linkname << " -> " << target << endl;
    } else if (option == "-h") {
        // 新增：创建硬链接，仅允许指向文件
        file* fp = resolveFileTarget(target, curdir);
        if (!fp) {
            cout << "No Such File.\n";
            return;
        }
        // 禁止对目录建立硬链接
        // 若目标路径恰好命中目录名而非文件名，resolveFileTarget 会返回 nullptr
        curdir->files[linkname] = fp;
        fp->link_count++;
        cout << "Create hard link: " << linkname << " => " << target << endl;
    } else {
        cout << "Wrong Option!\n";
    }
}