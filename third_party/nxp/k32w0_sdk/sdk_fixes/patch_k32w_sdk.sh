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

echo "SDK 2.6.6 doesn't need any patching!"
exit 0
