
pushd ../../../../

./scripts/tools/zap/generate.py -t silabs_examples/unify-matter-bridge/linux/zap-handlers/bridge-templates.json \
  -o silabs_examples/unify-matter-bridge/linux/zap-handlers/gen \
  silabs_examples/unify-matter-bridge/unify-matter-bridge-common/unify-matter-bridge.zap 

popd
