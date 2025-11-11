#include "fs_cmd_misc.h"
#include "fs_state.h"
#include "fs_types.h"
#include "fs_path.h"
#include <cstdlib>
#include <iostream>
#include <map>

void clear() {
    system("clear");
}

// 新增：链接完整性检查
void checklink() {
    std::map<file*, int> counts;
    int symlink_total = 0, symlink_broken = 0;

    // DFS 统计
    std::vector<dir*> stack;
    stack.push_back(root);
    while (!stack.empty()) {
        dir* d = stack.back(); stack.pop_back();

        for (auto &kv : d->files) {
            counts[kv.second]++;
        }
        for (auto &kv : d->links) {
            symlink_total++;
            symlink* sl = kv.second;
            bool broken = sl->to_dir ? isBrokenDirLink(sl->target, d)
                                     : isBrokenFileLink(sl->target, d);
            if (broken) symlink_broken++;
        }
        for (auto &kv : d->next) {
            stack.push_back(kv.second);
        }
    }

    int mismatch = 0;
    for (auto &kv : counts) {
        file* fp = kv.first;
        int actual = kv.second;
        if (fp->link_count != actual) mismatch++;
    }

    std::cout << "Hard link check:\n";
    std::cout << "  total files: " << counts.size() << "\n";
    std::cout << "  mismatched link_count: " << mismatch << "\n";
    std::cout << "Symlink check:\n";
    std::cout << "  total symlinks: " << symlink_total << "\n";
    std::cout << "  broken symlinks: " << symlink_broken << "\n";
}