#!/usr/bin/env bash

#
#    Copyright (c) 2021 Project CHIP Authors
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

# Run bootstrap and activate to set up

APP="$1"
BOARD="$2"

source "$(dirname "$0")/../../scripts/activate.sh"
cd "$(dirname "$0")/../../examples"
shift 2

if [[ ! -f "$APP/telink/CMakeLists.txt" ]]; then
    echo "Usage: $0 <application>" >&2
    echo "Applications:" >&2
    ls */telink/CMakeLists.txt | awk -F/ '{print "  "$1}' >&2
    exit 1
fi

if [ -z "$BOARD" ]; then
    echo "No mandatory BOARD argument supplied!"
    exit 1
fi

set -x
[[ -n $ZEPHYR_BASE ]] && source "$ZEPHYR_BASE/zephyr-env.sh"

export ZEPHYR_TOOLCHAIN_VARIANT=cross-compile
export CROSS_COMPILE="$TELINK_TOOLCHAIN_PATH"/riscv32-elf-

env

west build -b "$BOARD" -d "$APP/telink/build/$BOARD" "$APP/telink" -- "$@"
