#!/bin/bash
pushd ../../../../

output_dir=${d}silabs_examples/unify-matter-bridge/linux/zap-generated/unify-matter-bridge-common/zap-generated

rm -rf $output_dir
mkdir -p $output_dir

./scripts/tools/zap/generate.py \
  -o $output_dir \
  ${d}silabs_examples/unify-matter-bridge/unify-matter-bridge-common/unify-matter-bridge.zap

# This is a hack to make sure that the af-gen-event.h file is always generated
touch $output_dir/af-gen-event.h

find $output_dir -type f -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" -o -name "*.inc" | xargs clang-format -i -style=WebKit

##############################################################################################################################
# Created  Unit test specific zap files , due to intialization failure in general clusters like Time synchronization cluster
# gen_zap_file_for_ut.py script copies the zap file from unify-matter-bridge-common to tests/unify-matter-bridge-ut and 
# disables time synchronization server cluster  and modifies json relative path in zap file.

ut_output_dir=${d}silabs_examples/unify-matter-bridge/linux/src/tests/unify-matter-bridge-ut

rm -rf $ut_output_dir/unify-matter-bridge-ut.zap
rm -rf $ut_output_dir/unify-matter-bridge-ut.matter

./silabs_examples/unify-matter-bridge/scripts/gen_zap_file_for_ut.py

output_dir=${d}silabs_examples/unify-matter-bridge/linux/zap-generated/unify-matter-bridge-ut/zap-generated

rm -rf $output_dir
mkdir -p $output_dir


./scripts/tools/zap/generate.py \
  -o $output_dir \
  ${d}silabs_examples/unify-matter-bridge/linux/src/tests/unify-matter-bridge-ut/unify-matter-bridge-ut.zap

# This is a hack to make sure that the af-gen-event.h file is always generated
touch $output_dir/af-gen-event.h

find $output_dir -type f -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" -o -name "*.inc" | xargs clang-format -i -style=WebKit

popd