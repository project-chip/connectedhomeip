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

BUILD_BANK="$1"
BUILD_TARGET="$2"
RT_PLATFORM="$3"

if [ -z "$OT_SRCDIR" ]; then
    echo "OT_SRCDIR not set!"
    exit 1
fi

if [ -z "$BUILD_BANK" ] || [ -z "$BUILD_TARGET" ] || [ -z "$RT_PLATFORM" ]; then
    echo "Usage: $0 <bank0|bank1> <build_target> <rt_platform>"
    exit 1
fi

if [ "$BUILD_BANK" = "bank0" ]; then
    BANK_NUM=0
elif [ "$BUILD_BANK" = "bank1" ]; then
    BANK_NUM=1
else
    echo "Error: Invalid BUILD_BANK value '$BUILD_BANK'. Must be 'bank0' or 'bank1'." >&2
    exit 1
fi

SRC="$OT_SRCDIR/vendor/$RT_PLATFORM/$BUILD_TARGET/app.ld"
OUT="$OT_SRCDIR/vendor/$RT_PLATFORM/$BUILD_TARGET/app.ld.gen"
arm-none-eabi-gcc -D BUILD_BANK="$BANK_NUM" -E -P -x c "$SRC" -o "$OUT"
