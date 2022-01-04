#!/bin/bash

SCRIPT_DIR=$(dirname "$0")/..

cd "${SCRIPT_DIR}"/diagrams || exit

# Give user option to pass in custom file
if [ "$1" == "" ]
then
  ../scripts/run.sh
  opt -dot-cfg ../output/test_poly_opt.ll -enable-new-pm=0 -disable-output
else
  opt -dot-cfg ../output/"$1" -enable-new-pm=0 -disable-output
fi

shopt -s dotglob
for i in ./*.dot; do
  name=$(echo $(basename ${i}) | cut -d '.' -f 2)
  dot -Tsvg ${i} > ${name}.svg
  echo Creating ${name}.svg
done