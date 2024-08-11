#!/bin/bash
SOURCE=$1
echo "Compiling $1..."
clang -O0 -emit-llvm -target riscv64 -c $SOURCE -o ${SOURCE%.c}.bc
llvm-dis ${SOURCE%.c}.bc
echo "${SOURCE%.c}.bc and ${SOURCE%.c}.ll generated."