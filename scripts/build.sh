#!/usr/bin/env bash

set -e

rm -rf build
mkdir -p build/obj build/bin lls artifacts images

cmake -G Ninja                                  \
    -DCMAKE_C_COMPILER=clang-19                 \
    -DCMAKE_CXX_COMPILER=clang++-19             \
    -DLLVM_DIR=/usr/lib/llvm-19/lib/cmake/llvm  \
    -B build                                    \
    && cmake --build build

cd build
ninja
cd ..

clang -c src/Runtime.c -o build/obj/Runtime.o
