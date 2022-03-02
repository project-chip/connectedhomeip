OUT_DIR=out/ota_requestor_debug/
ANYCLOUD_DIR=third_party/p6/p6_sdk/libs/anycloud-ota/

./scripts/examples/gn_p6_example.sh examples/ota-requestor-app/p6/ "$OUT_DIR"

mv "$OUT_DIR"/chip-p6-ota-requestor-example.hex "$OUT_DIR"/chip-p6-ota-requestor-example.unsigned.hex

./"$ANYCLOUD_DIR"/scripts/sign_script.bash "$OUT_DIR"/ chip-p6-ota-requestor-example python3 arm-none-eabi-objcopy "-O ihex" "" "$ANYCLOUD_DIR"/source/mcuboot/scripts/ imgtool_v1.7.0/imgtool.py create 0xFF 0x400 3584 1.0.0 0x00018000 0x001C0000 arm-none-eabi-objcopy ""
