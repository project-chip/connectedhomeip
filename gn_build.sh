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
echo 'To compile the generated debug build:'
echo ninja -C "$CHIP_ROOT/out/debug"

echo
echo 'To test the generated debug build (idempotent):'
echo ninja -C "$CHIP_ROOT/out/debug" check

echo
echo 'To compile the generated release build':
echo ninja -C "$CHIP_ROOT/out/release"

echo
echo 'To test the generated release build (idempotent):'
echo ninja -C "$CHIP_ROOT/out/release" check

echo
echo 'To build a custom build (for help run "gn args --list out/debug")'
echo gn args "$CHIP_ROOT/out/custom"
echo ninja -C "$CHIP_ROOT/out/custom"

extra_args=""
user_args=""
ninja_args=()

while getopts :d:j:k:l:nt:vw: opt; do
    case "$opt" in
        [nv])
            ninja_args+=("-$opt")
            ;;
        [djkltw])
            ninja_args+=("-$opt" "$OPTARG")
            ;;
        '?')
            printf '\nError: unknown option -%s\n' "$OPTARG"
            printf 'Usage: %s [ninja-options] [gn-args]\n' "$0"
            exit 1
            ;;
    esac
done
shift $((OPTIND - 1))

for arg; do
    case $arg in
        enable_qpg_builds=true)
            qpg_enabled=1
            ;;
        enable_efr32_builds=true)
            efr32_enabled=1
            ;;
        enable_p6_builds=true)
            p6_builds_enabled=1
            ;;
        psoc6_board=*)
            p6_board_selected=1
            ;;
    esac
    user_args+=" $arg"
done

# Android SDK setup
android_sdk_args=""

if [[ -d "${ANDROID_NDK_HOME}/toolchains" && -d "${ANDROID_HOME}/platforms" ]]; then
    # Android prebuilt JAR setup
    python3 third_party/android_deps/set_up_android_deps.py

    android_sdk_args+="android_sdk_root=\"$ANDROID_HOME\" android_ndk_root=\"$ANDROID_NDK_HOME\""
    extra_args+=" $android_sdk_args enable_android_builds=true"
else
    echo
    echo "Hint: Set \$ANDROID_HOME and \$ANDROID_NDK_HOME to enable building for Android"
    echo "      The required android sdk platform version is 21. It can be obtained from"
    echo "      https://dl.google.com/android/repository/android-21_r02.zip"
fi

echo

# EFR32 SDK setup
if [[ -z "$efr32_enabled" ]]; then
    echo "Hint: Pass enable_efr32_builds=true to enable building for EFR32"
else
    echo 'To build the EFR32 lock sample as a standalone project':
    echo "(cd $CHIP_ROOT/examples/lock-app/efr32; gn gen out/debug; ninja -C out/debug)"
fi

echo

# PSoC6 Build setup
if [[ -z "$p6_builds_enabled" ]]; then
    echo "Hint: Pass enable_p6_builds=true to this script to enable building for PSoC6-43012"
else
    psoc6_sdk_args=""
    if [[ -z "$p6_board_selected" ]]; then
        psoc6_sdk_args="psoc6_board=\"CY8CKIT-062S2-43012\""
    fi
fi

# K32W SDK setup
k32w_sdk_args=""

if [[ -d "$NXP_K32W0_SDK_ROOT" ]]; then
    k32w_sdk_args+="k32w0_sdk_root=\"$NXP_K32W0_SDK_ROOT\""
    extra_args+=" $k32w0_sdk_args enable_k32w_builds=true"
fi

echo
if [[ ! -d "$NXP_K32W0_SDK_ROOT" ]]; then
    echo "Hint: Set \$NXP_K32W0_SDK_ROOT to enable building for K32W061"
else
    echo 'To build the K32W lock sample as a standalone project':
    echo "(cd $CHIP_ROOT/examples/lock-app/nxp/k32w/k32w0; gn gen out/debug --args='$k32w_sdk_args'; ninja -C out/debug)"
fi
echo

if [[ -z "$qpg_enabled" ]]; then
    echo "Hint: Pass enable_qpg_builds=true to this script to enable building for QPG"
else
    echo 'To build the QPG6100 lock sample as a standalone project:'
    echo "(cd $CHIP_ROOT/examples/lock-app/qpg; gn gen out/debug; ninja -C out/debug)"
fi

echo

# TI SimpleLink Build setup
ti_simplelink_sdk_args=""

if [[ -f "${TI_SYSCONFIG_ROOT}/sysconfig_cli.sh" ]]; then
    ti_simplelink_sdk_args+="ti_sysconfig_root=\"$TI_SYSCONFIG_ROOT\""
    extra_args+=" $ti_simplelink_sdk_args enable_ti_simplelink_builds=true"

    echo 'To build the cc13x2x7_26x2x7 lock sample as a standalone project':
    echo "(cd $CHIP_ROOT/examples/lock-app/cc13x2x7_26x2x7; gn gen out/debug --args='$ti_simplelink_sdk_args'; ninja -C out/debug)"
else
    echo "Hint: Set \$TI_SYSCONFIG_ROOT to enable building for cc13x2_26x2"
fi

echo

tizen_sdk_args=""

if [[ -d "${TIZEN_SDK_ROOT}" && -d "${TIZEN_SDK_SYSROOT}" ]]; then
    tizen_sdk_args+="tizen_sdk_root=\"$TIZEN_SDK_ROOT\" tizen_sdk_sysroot=\"$TIZEN_SDK_SYSROOT\""
    extra_args+=" $tizen_sdk_args enable_tizen_builds=true"
else
    echo
    echo "Hint: Set \$TIZEN_SDK_ROOT and \$TIZEN_SDK_SYSROOT to enable building for Tizen"
    echo "      Required Tizen SDK can be obtained from"
    echo "      https://developer.tizen.org/development/tizen-studio/download"
fi

echo

_chip_banner "Build: GN configure"

gn --root="$CHIP_ROOT" gen --check --fail-on-unused-args "$CHIP_ROOT/out/debug" --args='target_os="all"'"$extra_args$user_args"
gn --root="$CHIP_ROOT" gen --check --fail-on-unused-args "$CHIP_ROOT/out/release" --args='target_os="all" is_debug=false'"$extra_args$user_args"

_chip_banner "Build: Ninja build"

time ninja -C "$CHIP_ROOT/out/debug" "${ninja_args[@]}" all check
