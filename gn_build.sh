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

# TODO: Fix pigweed to bootstrap if necessary in activate.sh.
echo
echo "NB: If this fails run \"source scripts/bootstrap.sh\""

source "$CHIP_ROOT/scripts/activate.sh"

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

extra_args=""

for arg; do
    case $arg in
        enable_qpg6100_builds=true)
            qpg6100_enabled=1
            ;;
    esac
    extra_args+=" $arg"
done

# Android SDK setup
android_sdk_args=""

if [[ -d "${ANDROID_NDK_HOME}/toolchains" && -d "${ANDROID_HOME}/platforms" ]]; then
    android_sdk_args+="android_sdk_root=\"$ANDROID_HOME\" android_ndk_root=\"$ANDROID_NDK_HOME\""
    extra_args+=" $android_sdk_args enable_android_builds=true"
else
    echo
    echo "Hint: Set \$ANDROID_HOME and \$ANDROID_NDK_HOME to enable building for Android"
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

# K32W SDK setup
k32w_sdk_args=""

if [[ -d "$K32W061_SDK_ROOT" ]]; then
    k32w_sdk_args+="k32w_sdk_root=\"$K32W061_SDK_ROOT\""
    extra_args+=" $k32w_sdk_args enable_k32w_builds=true"
fi

echo
if [[ ! -d "$K32W061_SDK_ROOT" ]]; then
    echo "Hint: Set \$K32W061_SDK_ROOT to enable building for K32W061"
else
    echo 'To build the K32W lock sample as a standalone project':
    echo "(cd $CHIP_ROOT/examples/lock-app/k32w; gn gen out/debug --args='$k32w_sdk_args'; ninja -C out/debug)"
fi
echo

if [[ -z "$qpg6100_enabled" ]]; then
    echo "Hint: Pass enable_qpg6100_builds=true to this script to enable building for QPG6100"
else
    echo 'To build the QPG6100 lock sample as a standalone project:'
    echo "(cd $CHIP_ROOT/examples/lock-app/qpg6100; gn gen out/debug; ninja -C out/debug)"
fi

echo

_chip_banner "Build: GN configure"

gn --root="$CHIP_ROOT" gen --check --fail-on-unused-args "$CHIP_ROOT/out/debug" --args='target_os="all"'"$extra_args"
gn --root="$CHIP_ROOT" gen --check --fail-on-unused-args "$CHIP_ROOT/out/release" --args='target_os="all" is_debug=false'"$extra_args"

_chip_banner "Build: Ninja build"

time ninja -C "$CHIP_ROOT/out/debug" all check
