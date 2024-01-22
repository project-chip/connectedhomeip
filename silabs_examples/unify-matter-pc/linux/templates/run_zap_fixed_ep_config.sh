#!/bin/bash
pushd ../../../../

output_dir=${d}silabs_examples/unify-matter-pc/linux/zap-generated/unify-matter-pc-common/zap-generated

rm -rf $output_dir
mkdir -p $output_dir

./scripts/tools/zap/generate.py \
  -o $output_dir \
  ${d}silabs_examples/unify-matter-pc/unify-matter-pc-common/unify-matter-pc.zap

# This is a hack to make sure that the af-gen-event.h file is always generated
touch $output_dir/af-gen-event.h

find $output_dir -type f -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" -o -name "*.inc" | xargs clang-format -i -style=WebKit

popd