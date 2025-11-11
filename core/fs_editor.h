#pragma once
#include <string>

// 打开指定路径的文件进行编辑；返回是否成功保存
bool openEditor(const std::string& path);