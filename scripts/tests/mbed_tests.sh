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
SUPPORTED_TARGET_BOARD=(DISCO_L475VG_IOT01A NRF52840_DK CY8CPROTO_062_4343W)
SUPPORTED_PROFILES=(release develop debug)
SUPPORTED_COMMAND=(build flash build-flash)

TARGET_BOARD=DISCO_L475VG_IOT01A
TOOLCHAIN=GCC_ARM
PROFILE=develop
COMMAND=build

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
    -c=* | --command=*)
        COMMAND="${i#*=}"
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

if [[ ! " ${SUPPORTED_COMMAND[@]} " =~ " ${COMMAND} " ]]; then
    echo "ERROR: Command $COMMAND not supported"
    exit 1
fi

BUILD_DIRECTORY="build-$TARGET_BOARD/$PROFILE/"

set -e # Exit immediately if a command exits with a non-zero status.
# set -x # Print commands and their arguments as they are executed.

if [[ "$COMMAND" == *"build"* ]]; then
    echo "Build $APP app for $TARGET_BOARD target with $TOOLCHAIN toolchain and $PROFILE profile"

    # Config directory setup
    MBED_CONFIG_PATH=./cmake_build/"$TARGET_BOARD"/develop/"$TOOLCHAIN"/

    # Override Mbed OS path to development directory
    MBED_OS_PATH="$CHIP_ROOT"/third_party/mbed-os/repo

    # Create symlinks to mbed-os submodule
    ln -sfTr $MBED_OS_PATH "mbed-os"

    if [ "$TARGET_BOARD" == "DISCO_L475VG_IOT01A" ]; then
        # Add the Mbed OS driver for the ISM43362 Wi-Fi module
        WIFI_ISM43362_PATH="$CHIP_ROOT"/third_party/wifi-ism43362/repo

        # Create symlinks to WIFI-ISM43362 submodule
        ln -sfTr $WIFI_ISM43362_PATH "wifi-ism43362"
    fi

    # Generate config file for selected target, toolchain and hardware
    mbed-tools configure -t "$TOOLCHAIN" -m "$TARGET_BOARD"

    # Remove old artifacts to force linking
    rm -rf "${BUILD_DIRECTORY}/chip-"*

    # Create output directory and copy config file there.
    mkdir -p "$BUILD_DIRECTORY"
    cp -f "$MBED_CONFIG_PATH"/mbed_config.cmake "$BUILD_DIRECTORY"/mbed_config.cmake

    # Build application
    cmake -S "./" -B "$BUILD_DIRECTORY" -GNinja -DCMAKE_BUILD_TYPE="$PROFILE"
    cmake --build "$BUILD_DIRECTORY"
fi

if [[ "$COMMAND" == *"flash"* ]]; then

    echo "Flash Unit Tests app to $TARGET_BOARD target [$TOOLCHAIN toolchain, $PROFILE profile]"

    # Flash scripts path setup
    MBED_FLASH_SCRIPTS_PATH=$CHIP_ROOT/config/mbed/scripts

    # Flash application
    openocd -f $MBED_FLASH_SCRIPTS_PATH/$TARGET_BOARD.tcl -c "program $BUILD_DIRECTORY/chip-tests verify reset exit"
fi
