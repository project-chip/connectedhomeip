#!/bin/bash

if [[ ! -d $NXP_K32W0_SDK_ROOT ]]; then
    echo "NXP_K32W0_SDK_ROOT is not set"
    exit 1
fi

board=$(ls "$NXP_K32W0_SDK_ROOT"/boards)

convert_to_dos() {

    [[ $(file -b - <$1) != *"CRLF"* ]] && sed -i 's/$/\r/' "$1"
}

SOURCE=${BASH_SOURCE[0]}
SOURCE_DIR=$(cd "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)

convert_to_dos "$NXP_K32W0_SDK_ROOT"/middleware/wireless/framework/OSAbstraction/Source/fsl_os_abstraction_free_rtos.c
convert_to_dos "$NXP_K32W0_SDK_ROOT"/middleware/wireless/framework/OtaSupport/Interface/OtaUtils.h

patch -N --binary -d "$NXP_K32W0_SDK_ROOT"/middleware/wireless/framework/OSAbstraction/Source/ -p1 <"$SOURCE_DIR/fsl_os_abstraction_free_rtos_c.patch"
patch -N --binary -d "$NXP_K32W0_SDK_ROOT"/middleware/wireless/framework/OtaSupport/Interface/ -p1 <"$SOURCE_DIR/OtaUtils_h.patch"

echo "K32W SDK 2.6.7 was patched!"
exit 0
