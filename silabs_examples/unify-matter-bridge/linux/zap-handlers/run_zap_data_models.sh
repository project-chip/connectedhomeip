#!/bin/bash

if [ -z "${UCL_XML_PATH}" ]
then
  echo "Please dirrect the environment variable UCL_XML_PATH to the location of the Unify DotDot XML files."
  exit 1
fi

pushd ../../../../

echo "Generating Unify Data Model Templates"

output_dir=silabs_examples/unify-matter-bridge/linux/zap-generated/data-models

rm -rf $output_dir
mkdir -p $output_dir

./scripts/tools/zap/generate.py \
    -z ${UCL_XML_PATH}/library.xml \
    -t silabs_examples/unify-matter-bridge/linux/zap-handlers/data_models/unify_data_model.json \
    -o $output_dir \
    silabs_examples/unify-matter-bridge/unify-matter-bridge-common/unify-matter-bridge.zap

./scripts/tools/zap/generate.py \
    -t silabs_examples/unify-matter-bridge/linux/zap-handlers/data_models/matter_data_model.json \
    -o $output_dir \
    silabs_examples/unify-matter-bridge/unify-matter-bridge-common/unify-matter-bridge.zap

find $output_dir -type f -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" -o -name "*.inc" | xargs clang-format -i -style=WebKit



popd
