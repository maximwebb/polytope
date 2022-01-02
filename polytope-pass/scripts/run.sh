#!/bin/bash

SCRIPT_DIR=$(dirname "$0")/..
OPT_PATH=${SCRIPT_DIR}/../llvm-project/llvm/build/bin/opt
cd ${SCRIPT_DIR}/output

clang -emit-llvm -fno-discard-value-names -O0 -Xclang -disable-O0-optnone test.c -S -o test.ll
#OPT_PATH -S -passes="mem2reg,indvars,simplifycfg,loop-rotate" test.ll -o ${SCRIPT_DIR}/test1_pre_opt.ll
#OPT_PATH -S -load-pass-plugin ./cmake-build-debug/libPolyLoop.so test1_pre_opt.ll -o test1_opt.ll