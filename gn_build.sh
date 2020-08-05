#!/usr/bin/env bash
#
# Copyright (c) 2020 Project CHIP Authors
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

CHIP_ROOT="$(dirname "$0")"

_chip_red() {
    echo -e "\033[0;31m$*\033[0m"
}

_chip_yellow() {
    echo -e "\033[0;33m$*\033[0m"
}

_chip_banner() {
    _chip_yellow '.--------------------------------'
    _chip_yellow "-- $1"
    _chip_yellow "'--------------------------------"
}

_chip_banner "Environment bringup"

git -C "$CHIP_ROOT" submodule update --init

set +e
source "$CHIP_ROOT/scripts/activate.sh"
set -e

_chip_banner "Instructions"

echo
echo 'To activate existing build environment in your shell, run (do this first):'
echo source "$CHIP_ROOT/scripts/activate.sh"

echo
echo 'To re-create the build environment from scratch, run:'
echo source "$CHIP_ROOT/scripts/bootstrap.sh"

echo
echo 'To build a debug build:'
echo gn gen "$CHIP_ROOT/out/debug" --args=\''target_os="all"'"$extra_args"\'
echo ninja -C "$CHIP_ROOT/out/debug"

echo
echo 'To run tests (idempotent):'
echo ninja -C "$CHIP_ROOT/out/debug" check

echo
echo 'To build & test an optimized build:'
echo gn gen "$CHIP_ROOT/out/release" --args=\''target_os="all" is_debug=false'"$extra_args"\'
echo ninja -C "$CHIP_ROOT/out/release" check

echo
echo 'To build a custom build (for help run "gn args --list out/debug")'
echo gn args "$CHIP_ROOT/out/custom"
echo ninja -C "$CHIP_ROOT/out/custom"

# nRF5 SDK setup
nrf5_sdk_args=""
extra_args=""

if [[ -d "$NRF5_SDK_ROOT/components/libraries" ]]; then
    nrf5_sdk_args+="nrf5_sdk_root=\"$NRF5_SDK_ROOT\""
    extra_args+=" $nrf5_sdk_args enable_nrf5_builds=true"
fi

echo
if [[ ! -d "$NRF5_SDK_ROOT/components/libraries" ]]; then
    echo "Hint: Set \$NRF5_SDK_ROOT to enable building for nRF5"
else
    echo 'To build the nRF5 lock sample as a standalone project':
    echo "(cd $CHIP_ROOT/examples/lock-app/nrf5; gn gen out/debug --args='$nrf5_sdk_args'; ninja -C out/debug)"
fi
echo

# EFR32 SDK setup
efr32_sdk_args=""

if [[ -d "$EFR32_SDK_ROOT/protocol/" ]]; then
    efr32_sdk_args+="efr32_sdk_root=\"$EFR32_SDK_ROOT\" efr32_board=\"$EFR32_BOARD\""
    extra_args+=" $efr32_sdk_args enable_efr32_builds=true"
    echo 'To build the EFR32 lock sample as a standalone project':
    echo "(cd $CHIP_ROOT/examples/lock-app/efr32; gn gen out/debug --args='$efr32_sdk_args'; ninja -C out/debug)"
else
    echo "Hint: Set \$EFR32_SDK_ROOT to enable building for EFR32"
fi

echo

_chip_banner "Build: GN configure"

gn --root="$CHIP_ROOT" gen --check "$CHIP_ROOT/out/debug" --args='target_os="all"'"$extra_args"
gn --root="$CHIP_ROOT" gen --check "$CHIP_ROOT/out/release" --args='target_os="all" is_debug=false'"$extra_args"

_chip_banner "Build: Ninja build"

time ninja -C "$CHIP_ROOT/out/debug" all check
