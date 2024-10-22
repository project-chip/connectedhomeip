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

cd "$(dirname "$0")/../../examples"

APP="$1"
BOARD="$2"
shift 2

# Disable debug symbols and firmware build time to increase ccache hit ratio in CI
COMMON_CI_FLAGS=(-DCONFIG_CHIP_DEBUG_SYMBOLS=n)

if [[ ! -f "$APP/nrfconnect/CMakeLists.txt" || -z "$BOARD" ]]; then
    echo "Usage: $0 <application> <board>" >&2
    echo "Applications:" >&2
    ls */nrfconnect/CMakeLists.txt | awk -F/ '{print "  "$1}' >&2
    exit 1
fi

set -x

# Activate Matter environment
source "../scripts/activate.sh"

# Activate Zephyr environment
[[ -n $ZEPHYR_BASE ]] && source "$ZEPHYR_BASE/zephyr-env.sh"

# Use Zephyr SDK toolchain
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr

if [[ -z "$ZEPHYR_SDK_INSTALL_DIR" && -n "$NRF5_TOOLS_ROOT" ]]; then
    export ZEPHYR_SDK_INSTALL_DIR="$NRF5_TOOLS_ROOT"/zephyr-sdk-0.16.1
fi

# Set ccache base directory to improve the cache hit ratio
export CCACHE_BASEDIR="$PWD/$APP/nrfconnect"

env
west build -p auto -b "$BOARD" -d "$APP/nrfconnect/build" "$APP/nrfconnect" --sysbuild -- "${COMMON_CI_FLAGS[@]}" "$@"
