#!/bin/bash

set -e

# 如果 build 目录不存在则创建
if [ ! -d "build" ]; then
    mkdir build
fi

# 清理 build 目录
rm -rf build/*

# 进入 build 编译
cd build &&
    cmake .. &&
    make -j

# 回到项目根目录
cd ..

# 创建 lib 目录
if [ ! -d "lib" ]; then
    mkdir lib
fi

# 拷贝头文件到 lib 目录下
# 注意：这里 cp -r src/include lib 会生成 lib/include
cp  src/include/* lib/