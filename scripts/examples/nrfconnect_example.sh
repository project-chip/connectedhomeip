#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
    export ZEPHYR_SDK_INSTALL_DIR="$NRF5_TOOLS_ROOT"/zephyr-sdk-0.16.0
fi

# Set ccache base directory to improve the cache hit ratio
export CCACHE_BASEDIR="$PWD/$APP/nrfconnect"

env
west build -p auto -b "$BOARD" -d "$APP/nrfconnect/build" "$APP/nrfconnect" -- "${COMMON_CI_FLAGS[@]}" "$@"
