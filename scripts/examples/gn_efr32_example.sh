#!/usr/bin/env bash

#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# Build script for GN EFT32 examples GitHub workflow.

CHIP_ROOT="$(dirname "$0")/../.."

source "$CHIP_ROOT/scripts/activate.sh"

set -e
set -x
env

if [ -z "$3" ]; then
    gn gen --root="$1" --args="efr32_sdk_root=\"$EFR32_SDK_ROOT\"" "$2"/"$EFR32_BOARD"/
    ninja -v -C "$2"/"$EFR32_BOARD"/
else
    gn gen --root="$1" --args="efr32_sdk_root=\"$EFR32_SDK_ROOT\" efr32_board=\"$3\"" "$2/$3"
    ninja -v -C "$2/$3"
fi

# Post build step since Pigweed doesn't seems to support objcopy yet.
if [ -z "$3" ]; then
    arm-none-eabi-objcopy -O srec ./"$2"/"$EFR32_BOARD"/chip-efr32-lock-example.out ./"$2"/"$EFR32_BOARD"/chip-efr32-lock-example.s37
    arm-none-eabi-size ./"$2"/"$EFR32_BOARD"/chip-efr32-lock-example.s37
else
    arm-none-eabi-objcopy -O srec ./"$2"/"$3"/chip-efr32-lock-example.out ./"$2"/"$3"/chip-efr32-lock-example.s37
    arm-none-eabi-size ./"$2"/"$3"/chip-efr32-lock-example.s37
fi
