#!/bin/bash
#
# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]; then
    echo "Usage: $0 <bank0|bank1> <CMAKE_TARGET> <RT_PLATFORM>"
    exit 1
fi

BUILD_BANK="$1"
CMAKE_TARGET="$2"
RT_PLATFORM="$3"

# disassembly
arm-none-eabi-objdump -S -C -l "$OUT_FOLDER/bin/$CMAKE_TARGET" >"$OUT_FOLDER/bin/$CMAKE_TARGET".asm

if [ "$BUILD_BANK" = "bank1" ]; then
    BIN_FILE="$OUT_FOLDER/bin/${CMAKE_TARGET}_bank1.bin"
    MP_FILE="$OUT_FOLDER/bin/${CMAKE_TARGET}_bank1_MP.bin"
    TRACE_FILE="$OUT_FOLDER/bin/${CMAKE_TARGET}_bank1.trace"
else
    BIN_FILE="$OUT_FOLDER/bin/${CMAKE_TARGET}_bank0.bin"
    MP_FILE="$OUT_FOLDER/bin/${CMAKE_TARGET}_bank0_MP.bin"
    TRACE_FILE="$OUT_FOLDER/bin/${CMAKE_TARGET}_bank0.trace"
fi

# generate bin and trace
arm-none-eabi-objcopy -O binary -S "$OUT_FOLDER/bin/$CMAKE_TARGET" "$BIN_FILE" --remove-section=App.trace
arm-none-eabi-objcopy -O binary -S "$OUT_FOLDER/bin/$CMAKE_TARGET" "$TRACE_FILE" --only-section=App.trace

if [ "$(uname -s)" = "Darwin" ]; then
    PREPEND_HEADER="$REALTEK_SDK_PATH/tools/prepend_header/prepend_header.macOS"
    MD5_TOOL="$REALTEK_SDK_PATH/tools/md5/MD5.macOS"
elif [ "$(uname -s)" = "Linux" ]; then
    PREPEND_HEADER="$REALTEK_SDK_PATH/tools/prepend_header/prepend_header"
    MD5_TOOL="$REALTEK_SDK_PATH/tools/md5/MD5"
fi

chmod +x "$PREPEND_HEADER"
chmod +x "$MD5_TOOL"

"$PREPEND_HEADER" -t app_code -b 15 -p "$BIN_FILE" -m 1 \
    -i "$OT_SRCDIR/vendor/$RT_PLATFORM/common/mp.ini" \
    -r "$REALTEK_SDK_PATH/tools/keys/rtk_ecdsa.pem" \
    -a "$REALTEK_SDK_PATH/tools/keys/key.json"

"$MD5_TOOL" "$MP_FILE"
