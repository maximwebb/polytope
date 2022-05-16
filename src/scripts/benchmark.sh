#!/bin/bash

SCRIPT_DIR=$(dirname "$0")/..
OPT_PATH=${SCRIPT_DIR}/../llvm-project/llvm/build/bin/opt
cd ${SCRIPT_DIR}/output

clang -emit-llvm -fno-discard-value-names -O0 -Xclang -disable-O0-optnone test.c -S -o test.ll
${OPT_PATH} -S -passes="mem2reg,loop-rotate,simplifycfg,instcombine" test.ll -o test_opt.ll
${OPT_PATH} -S -load-pass-plugin ../cmake-build-debug/libpolytope-pass.so test_opt.ll -o test_poly_opt.ll
clang -O0 test_poly_opt.ll -o poly

clang -O0 test_opt.ll -o control

echo "Poly opt:"
./poly
echo "Control:"
./control