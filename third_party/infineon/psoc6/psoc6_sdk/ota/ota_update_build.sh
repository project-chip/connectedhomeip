#!/bin/bash

SCRIPT_ROOT_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")/" && pwd)"
# Windows: convert /cygdrive/d/git to D:/git
if [[ ${OS:-} == Windows_NT ]]; then
    SCRIPT_ROOT_DIR="$(cygpath --mixed "$SCRIPT_ROOT_PATH")"
else
    SCRIPT_ROOT_DIR="$SCRIPT_ROOT_PATH"
fi

APP=${1:-"psoc6-lock"}                   # {psoc6-lock, psoc6-light, psoc6-all-clusters}
HEX_NAME=${2:-"chip-psoc6-lock-example"} # {chip-psoc6-lock-example, chip-psoc6-lighting-example, chip-psoc6-clusters-example}

LIBS_DIR="$SCRIPT_ROOT_DIR/../libs"
ROOT_DIR="$SCRIPT_ROOT_DIR/../../../../../"
OUT_DIR="$ROOT_DIR/out/infineon-$APP-ota-updateimage"

source "$ROOT_DIR/scripts/activate.sh"

arm-none-eabi-objcopy -I ihex -O elf32-little "$OUT_DIR/$HEX_NAME".hex "$OUT_DIR/$HEX_NAME".elf

chmod +x "$OUT_DIR/$HEX_NAME".elf

"$LIBS_DIR"/ota-bootloader-abstraction/scripts/mcuboot/sign_script.bash "$OUT_DIR" "$HEX_NAME" python3 arm-none-eabi-objcopy "-O ihex" "elf_first" "$LIBS_DIR"/ota-bootloader-abstraction/makefiles/mcuboot/../../scripts/mcuboot/imgtool imgtool.py sign 0xFF 0x400 3584 1.0.0 0x028000 0x1c0000 arm-none-eabi-objcopy "-k $LIBS_DIR/ota-bootloader-abstraction/scripts/mcuboot/keys/cypress-test-ec-p256.pem"

if [ -f "$OUT_DIR"/"$HEX_NAME".ota ]; then
    rm "$OUT_DIR"/"$HEX_NAME".ota
fi

python3 "$ROOT_DIR"/src/app/ota_image_tool.py create -v 0xFFF1 -p 0x8000 -vn 2 -vs "2.0" -da sha256 "$OUT_DIR"/"$HEX_NAME".bin "$OUT_DIR"/"$HEX_NAME".ota
