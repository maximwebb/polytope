#!/bin/bash

SCRIPT_DIR=$(dirname "$0")/..
OPT_PATH=${SCRIPT_DIR}/../llvm-project/llvm/build/bin/opt
cd ${SCRIPT_DIR}/output

clang -emit-llvm -fno-discard-value-names -O0 -Xclang -disable-O0-optnone test.c -S -o test.ll
${OPT_PATH} -S -passes="mem2reg,loop-rotate,simplifycfg,instcombine" test.ll -o test_opt.ll
echo "foo"
if [ "$1" != 'polynone' ]
then
  ${OPT_PATH} -S -load-pass-plugin ../cmake-build-debug/libpolytope-pass.so test_opt.ll -o test_poly_opt.ll
fi