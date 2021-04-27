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

source "$(dirname "$0")/../../scripts/activate.sh"

cd "$(dirname "$0")"/../..
CHIP_ROOT=$PWD
cd "$CHIP_ROOT"/examples

SUPPORTED_TOOLCHAIN=(GCC_ARM ARM)
SUPPORTED_TARGET_BOARD=(DISCO_L475VG_IOT01A NRF52840_DK CY8CPROTO_062_4343W)
SUPPORTED_APP=(shell lock-app)
SUPPORTED_PROFILES=(release develop debug)

APP=shell
TARGET_BOARD=NRF52840_DK
TOOLCHAIN=GCC_ARM
PROFILE=release

for i in "$@"; do
    case $i in
        -a=* | --app=*)
            APP="${i#*=}"
            shift
            ;;
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
        *)
            # unknown option
            ;;
    esac
done

if [[ ! " ${SUPPORTED_TARGET_BOARD[@]} " =~ " ${TARGET_BOARD} " ]]; then
    echo "ERROR: Target $TARGET_BOARD not supported"
    exit 1
fi

if [[ ! " ${SUPPORTED_APP[@]} " =~ " ${APP} " ]]; then
    echo "ERROR: Application $APP not supported"
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

echo "Build $APP app for $TARGET_BOARD target with $TOOLCHAIN toolchain and $PROFILE profile"
set -x
pwd

# Build and config directory setup
BUILD_DIRECTORY="$APP"/mbed/build-"$TARGET_BOARD"/"$PROFILE"/
MBED_CONFIG_PATH="$APP"/mbed/cmake_build/"$TARGET_BOARD"/develop/"$TOOLCHAIN"/

# Override Mbed OS path to development directory
MBED_OS_PATH="$CHIP_ROOT"/third_party/mbed-os/repo

# Add the Mbed OS driver for the ISM43362 Wi-Fi module
WIFI_ISM43362_PATH="$CHIP_ROOT"/third_party/wifi-ism43362/repo

# Create symlinks to submodules
ln -sfTr "$MBED_OS_PATH" "$APP/mbed/mbed-os"
ln -sfTr "$WIFI_ISM43362_PATH" "$APP/mbed/wifi-ism43362"

# Generate config file for selected target, toolchain and hardware
mbed-tools configure -t "$TOOLCHAIN" -m "$TARGET_BOARD" -p "$APP"/mbed/

# Create output directory and copy config file there.
mkdir -p "$BUILD_DIRECTORY"
cp -f "$MBED_CONFIG_PATH"/mbed_config.cmake "$BUILD_DIRECTORY"/mbed_config.cmake

# Build application
cmake -S "$APP/mbed" -B "$BUILD_DIRECTORY" -GNinja -DCMAKE_BUILD_TYPE="$PROFILE"
cmake --build "$BUILD_DIRECTORY"
