#!/bin/bash
pushd ../../../../

# TODO Replace "-z third_party/zap/repo/zcl-builtin/dotdot/library.xml" with "-z ${UCL_XML_PATH}/library.xml" when UCL zap files have been updated so Zap can parse them without error

./scripts/tools/zap/generate.py \
    -z third_party/zap/repo/zcl-builtin/dotdot/library.xml \
    -t silabs_examples/unify-matter-bridge/linux/zap-handlers/unify-templates.json \
    -o silabs_examples/unify-matter-bridge/linux/zap-generated/data_model_translator \
    silabs_examples/unify-matter-bridge/unify-matter-bridge-common/unify-matter-bridge.zap 

popd
