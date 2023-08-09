#!/bin/bash
pushd ../../../../

output_dir=silabs_examples/unify-matter-pc/linux/zap-generated/

rm -rf "$output_dir"
mkdir -p "$output_dir"

./scripts/tools/zap/generate.py -t silabs_examples/unify-matter-pc/linux/templates/mpc-templates.json \
    -o "$output_dir" \
    silabs_examples/unify-matter-pc/unify-matter-pc-common/unify-matter-pc.zap

find "$output_dir" -type f -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" -o -name "*.inc" | xargs clang-format -i -style=WebKit

popd
