OUT_DIR=$1
HEX_NAME=$2
ANYCLOUD_DIR=third_party/infineon/psoc6/psoc6_sdk/libs/anycloud-ota/

mv "$OUT_DIR"/"$HEX_NAME".hex "$OUT_DIR"/"$HEX_NAME".unsigned.hex

./"$ANYCLOUD_DIR"/scripts/sign_script.bash "$OUT_DIR"/ "$HEX_NAME" python3 arm-none-eabi-objcopy "-O ihex" "" "$ANYCLOUD_DIR"/source/COMPONENT_OTA_MCUBOOT_PSOC/mcuboot/scripts/ imgtool_v1.7.0/imgtool.py create 0xFF 0x400 3584 1.0.0 0x00018000 0x001C0000 arm-none-eabi-objcopy ""
