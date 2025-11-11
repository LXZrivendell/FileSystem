#include "fs_keyring.h"
#include <map>
#include <fstream>
#include "fs_state.h"

static const char* KEY_FILE = "keyring.dat";

std::string keyringGet(const std::string& user) {
    auto it = g_keyring.find(user);
    return it == g_keyring.end() ? "" : it->second;
}

void keyringSet(const std::string& user, const std::string& key) {
    g_keyring[user] = key;
}

void keyringInit() {
    std::ifstream in(KEY_FILE);
    std::string u, k;
    while (in >> u >> k) {
        g_keyring[u] = k;
    }
}

void keyringSave() {
    std::ofstream out(KEY_FILE);
    for (auto& kv : g_keyring) {
        out << kv.first << " " << kv.second << "\n";
    }
}