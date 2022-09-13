d=$(pwd)
mkdir -p ${d}/linux/third_party/connectedhomeip/zzz_generated/unify-matter-bridge/zap-generated
./linux/third_party/connectedhomeip/scripts/tools/zap/generate.py \
  -o ${d}/linux/third_party/connectedhomeip/zzz_generated/unify-matter-bridge/zap-generated \
  ${d}/unify-matter-bridge-common/unify-matter-bridge.zap
