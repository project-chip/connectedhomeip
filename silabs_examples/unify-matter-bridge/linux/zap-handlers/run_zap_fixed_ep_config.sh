#!/bin/bash
pushd ../../../../

mkdir -p ${d}silabs_examples/unify-matter-bridge/linux/zap-generated/unify-matter-bridge-common/zap-generated
./scripts/tools/zap/generate.py \
  -o ${d}silabs_examples/unify-matter-bridge/linux/zap-generated/unify-matter-bridge-common/zap-generated \
  ${d}silabs_examples/unify-matter-bridge/unify-matter-bridge-common/unify-matter-bridge.zap

popd