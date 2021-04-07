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

cd "$(dirname $0)/../.."
CHIP_ROOT="$PWD"
cd "$CHIP_ROOT/src/test_driver/mbed/"

SUPPORTED_TOOLCHAIN=(GCC_ARM ARM)
SUPPORTED_TARGET_BOARD=(DISCO_L475VG_IOT01A NRF52840_DK)
SUPPORTED_PROFILES=(release develop debug)
SUPPORTED_MODES=(build build_flash flash_only)

TARGET_BOARD=DISCO_L475VG_IOT01A
TOOLCHAIN=GCC_ARM
PROFILE=develop
MODE=build

for i in "$@"; do
    case $i in
    -b=* | --board=*)
        TARGET_BOARD="${i#*=}"
        shift
        ;;
    -t=* | --toolchain=*)
        TOOLCHAIN="${i#*=}"
        shift
        ;;
    -p=* | --profile=*)
        PROFILE="${i#*=}"
        shift
        ;;
    -m=* | --mode=*)
        MODE="${i#*=}"
        shift
        ;;
    *)
        # unknown option
        ;;
    esac
done

if [[ ! " ${SUPPORTED_TARGET_BOARD[@]} " =~ " ${TARGET_BOARD} " ]]; then
    echo "ERROR: Target $TARGET_BOARD not supported"
    exit 1
fi

if [[ ! " ${SUPPORTED_TOOLCHAIN[@]} " =~ " ${TOOLCHAIN} " ]]; then
    echo "ERROR: Toolchain $TOOLCHAIN not supported"
    exit 1
fi

if [[ ! " ${SUPPORTED_PROFILES[@]} " =~ " ${PROFILE} " ]]; then
    echo "ERROR: Profile $PROFILE not supported"
    exit 1
fi

if [[ ! " ${SUPPORTED_MODES[@]} " =~ " ${MODE} " ]]; then
    echo "ERROR: Mode $MODE not supported"
    exit 1
fi

BUILD_DIRECTORY="build-$TARGET_BOARD/$PROFILE/"

set -e # Exit immediately if a command exits with a non-zero status.
# set -x # Print commands and their arguments as they are executed.

if [[ "${MODE}" == *"build"* ]]; then
    # Create symlinks to submodules.
    ln -sfTr "$CHIP_ROOT/third_party/mbed-os/repo" "mbed-os"
    ln -sfTr "$CHIP_ROOT/third_party/wifi-ism43362/repo" "wifi-ism43362"

    # Create a target specific build dir.
    mkdir -p "$BUILD_DIRECTORY"

    # Remove old artifacts to force linking
    rm -rf "${BUILD_DIRECTORY}/chip-tests"*

    # Generate a config file for selected toolchain and target.
    mbed-tools configure -t "$TOOLCHAIN" -m "$TARGET_BOARD"
    cp -f "cmake_build/$TARGET_BOARD/develop/$TOOLCHAIN/mbed_config.cmake" "$BUILD_DIRECTORY"

    # Build.
    cmake -S "./" -B "$BUILD_DIRECTORY" -G Ninja -D CMAKE_BUILD_TYPE="$PROFILE"
    cmake --build "$BUILD_DIRECTORY"
fi

if [[ "${MODE}" == *"flash"* ]]; then
    case $TARGET_BOARD in
    NRF52840_DK)
        PYOCD_TARGET=nrf52840
        ;;
    DISCO_L475VG_IOT01A)
        PYOCD_TARGET=stm32l475xg
        PYOCD_TARGET_ARGS="--frequency 4M"
        ;;
    *)
        # unknown option
        ;;
    esac

    # Flash the target.
    pyocd flash -t "$PYOCD_TARGET" -O connect_mode=under-reset $PYOCD_TARGET_ARGS "$BUILD_DIRECTORY/chip-tests.hex"
fi
