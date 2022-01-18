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

cd "$(dirname "$0")"/../../..
CHIP_ROOT=$PWD
cd "$CHIP_ROOT"/src/test_driver/mbed/unit_tests/

SUPPORTED_TOOLCHAIN=(GCC_ARM ARM)
SUPPORTED_TARGET_BOARD=(CY8CPROTO_062_4343W)
SUPPORTED_PROFILES=(release develop debug)
SUPPORTED_COMMAND=(build flash build-flash)

COMMAND=build
TARGET_BOARD=CY8CPROTO_062_4343W
TOOLCHAIN=GCC_ARM
PROFILE=release

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

set -e # Exit immediately if a command exits with a non-zero status.

# Activate Matter environment
source "$CHIP_ROOT"/scripts/activate.sh

# Build directory setup
BUILD_DIRECTORY=build-"$TARGET_BOARD"/"$PROFILE"/

if [[ "$COMMAND" == *"build"* ]]; then
    echo "Build unit tests app for $TARGET_BOARD target with $TOOLCHAIN toolchain and $PROFILE profile"

    # Set Mbed OS path
    MBED_OS_PATH="$CHIP_ROOT"/third_party/mbed-os/repo

    # Set Mbed OS posix socket submodule path
    MBED_OS_POSIX_SOCKET_PATH="$CHIP_ROOT"/third_party/mbed-os-posix-socket/repo

    # Generate config file for selected target, toolchain and hardware
    mbed-tools configure -t "$TOOLCHAIN" -m "$TARGET_BOARD" -o "$BUILD_DIRECTORY" --mbed-os-path "$MBED_OS_PATH"

    # Remove old artifacts to force linking
    rm -rf "$BUILD_DIRECTORY/chip-"*

    # Build application
    cmake -S "./" -B "$BUILD_DIRECTORY" -GNinja -DCMAKE_BUILD_TYPE="$PROFILE" -DMBED_OS_PATH="$MBED_OS_PATH" -DMBED_OS_POSIX_SOCKET_PATH="$MBED_OS_POSIX_SOCKET_PATH"
    cmake --build "$BUILD_DIRECTORY"
fi

if [[ "$COMMAND" == *"flash"* ]]; then

    echo "Flash unit tests app to $TARGET_BOARD target [$TOOLCHAIN toolchain, $PROFILE profile]"

    # Flash scripts path setup
    MBED_FLASH_SCRIPTS_PATH=$CHIP_ROOT/config/mbed/scripts

    # Flash application
    "$OPENOCD_PATH"/bin/openocd -f "$MBED_FLASH_SCRIPTS_PATH/$TARGET_BOARD".tcl -c "program $BUILD_DIRECTORY/chip-mbed-unit-tests.elf verify reset exit"
fi
