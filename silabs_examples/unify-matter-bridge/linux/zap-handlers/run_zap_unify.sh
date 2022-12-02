#!/bin/bash

if [ -z "${UCL_XML_PATH}" ]
then
  echo "Please dirrect the environment variable UCL_XML_PATH to the location of the Unify DotDot XML files."
  exit 1
fi

pushd ../../../../

./scripts/tools/zap/generate.py \
    -z ${UCL_XML_PATH}/library.xml \
    -t silabs_examples/unify-matter-bridge/linux/zap-handlers/unify-templates.json \
    -o silabs_examples/unify-matter-bridge/linux/zap-generated/data_model_translator \
    silabs_examples/unify-matter-bridge/unify-matter-bridge-common/unify-matter-bridge.zap 

popd
