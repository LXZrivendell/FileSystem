#include "fs_util.h"

bool judgeName(std::string name) {
    if (name.find("\\") != name.npos || name.find("/") != name.npos || name.find(":") != name.npos || name.find("*") != name.npos || name.find("?") != name.npos
        || name.find("<") != name.npos || name.find(">") != name.npos || name.find("|") != name.npos || name.find("\"") != name.npos || name.find("..") != name.npos || name.find(" ") != name.npos) return false;
    return true;
}