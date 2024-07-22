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
MATTER_SCRIPTS_DIR="$SCRIPT_ROOT_DIR/../../../../../scripts"
OUT_DIR="$SCRIPT_ROOT_DIR/../../../../../out/infineon-$APP-ota"

source "$MATTER_SCRIPTS_DIR/activate.sh"

arm-none-eabi-objcopy -I ihex -O elf32-little "$OUT_DIR/$HEX_NAME".hex "$OUT_DIR/$HEX_NAME".elf
chmod +x "$OUT_DIR/$HEX_NAME".elf
"$LIBS_DIR"/ota-bootloader-abstraction/scripts/mcuboot/sign_script.bash "$OUT_DIR" "$HEX_NAME" python3 arm-none-eabi-objcopy "-O ihex" "elf_first" "$LIBS_DIR"/ota-bootloader-abstraction/makefiles/mcuboot/../../scripts/mcuboot/imgtool imgtool.py create 0xFF 0x400 3584 1.0.0 0x028000 0x1c0000 arm-none-eabi-objcopy "-k $LIBS_DIR/ota-bootloader-abstraction/scripts/mcuboot/keys/cypress-test-ec-p256.pem"
