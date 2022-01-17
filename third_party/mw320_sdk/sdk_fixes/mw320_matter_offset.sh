"$MW320_SDK_ROOT"/tools/mw_img_conv/src/mw_img_conv mcufw "$MW320_LIGHTING"/out/mw320/chip-mw320-light-example.bin "$MW320_LIGHTING"/out/mw320/chip-mw320-light-example.mcufw.bin 0x1F000100
cp "$MW320_LIGHTING"/out/mw320/chip-mw320-light-example.mcufw.bin "$MW320_SDK_ROOT"/mw320_matter_flash/Matter/chip-mw320-light-example.mcufw.bin

echo "Please flash $MW320_SDK_ROOT/mw320_matter_flash/Matter/chip-mw320-light-example.mcufw.bin to NXP MW320 platfom"
exit 0
