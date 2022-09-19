OUT_DIR=$1
HEX_NAME=$2
ANYCLOUD_DIR=third_party/infineon/psoc6/psoc6_sdk/libs/anycloud-ota/

mv "$OUT_DIR"/"$HEX_NAME".hex "$OUT_DIR"/"$HEX_NAME".unsigned.hex

./"$ANYCLOUD_DIR"/scripts/sign_script.bash "$OUT_DIR"/ "$HEX_NAME" python3 arm-none-eabi-objcopy "-O ihex" "" "$ANYCLOUD_DIR"/source/COMPONENT_OTA_MCUBOOT_PSOC/mcuboot/scripts/ imgtool_v1.7.0/imgtool.py sign 0xFF 0x400 3584 1.0.0 0x00018000 0x001C0000 arm-none-eabi-objcopy "-k $ANYCLOUD_DIR/source/COMPONENT_OTA_MCUBOOT_PSOC/mcuboot/keys/cypress-test-ec-p256.pem"

if [ -f "$OUT_DIR"/"$HEX_NAME".ota ]; then
    rm "$OUT_DIR"/"$HEX_NAME".ota
fi

src/app/ota_image_tool.py create -v 0xFFF1 -p 0x8000 -vn 2 -vs "2.0" -da sha256 "$OUT_DIR"/"$HEX_NAME".bin "$OUT_DIR"/"$HEX_NAME".ota
