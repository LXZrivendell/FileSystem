#!/bin/bash
echo "编译文件系统项目..."
g++ -std=c++11 -Wall -Wextra -o filesystem core/*.cpp
if [ $? -eq 0 ]; then
    echo "编译成功！"
    echo "运行: ./filesystem"
else
    echo "编译失败！"
fi