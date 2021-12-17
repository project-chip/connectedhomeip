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

cd "$(dirname "$0")"/../..
CHIP_ROOT=$PWD
cd "$CHIP_ROOT"/examples

SUPPORTED_TOOLCHAIN=(GCC_ARM ARM)
SUPPORTED_TARGET_BOARD=(CY8CPROTO_062_4343W)
SUPPORTED_APP=(lock-app lighting-app pigweed-app all-clusters-app shell ota-requestor-app)
SUPPORTED_PROFILES=(release develop debug)
SUPPORTED_COMMAND=(build flash build-flash)
SUPPORTED_TYPE=(simple boot upgrade)

COMMAND=build
APP=lock-app
TARGET_BOARD=CY8CPROTO_062_4343W
TOOLCHAIN=GCC_ARM
PROFILE=release
TYPE=simple

TARGET_MEMORY_ALIGN[CY8CPROTO_062_4343W]=8

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
    -c=* | --command=*)
        COMMAND="${i#*=}"
        shift
        ;;
    -T=* | --type=*)
        TYPE="${i#*=}"
        shift
        ;;
    *)
        # unknown option
        ;;
    esac
done

if [[ ! " ${SUPPORTED_COMMAND[@]} " =~ " ${COMMAND} " ]]; then
    echo "ERROR: Command $COMMAND not supported"
    exit 1
fi

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

if [[ ! " ${SUPPORTED_TYPE[@]} " =~ " ${TYPE} " ]]; then
    echo "ERROR: Type $TYPE not supported"
    exit 1
fi

set -e # Exit immediately if a command exits with a non-zero status.

# Activate Matter environment
source "$CHIP_ROOT"/scripts/activate.sh

# Build directory setup
BUILD_DIRECTORY="$APP"/mbed/build-"$TARGET_BOARD"/"$PROFILE"/

# Set bootloader root directory
BOOTLOADER_ROOT_DIRECTORY="$CHIP_ROOT"/examples/platform/mbed/bootloader

# Set bootloader build directory
BOOTLOADER_BUILD_DIRECTORY="$BOOTLOADER_ROOT_DIRECTORY"/build-"$TARGET_BOARD"/"$PROFILE"/

if [[ "$COMMAND" == *"build"* ]]; then
    echo "Build $TYPE $APP app for $TARGET_BOARD target with $TOOLCHAIN toolchain and $PROFILE profile"

    # Set Mbed OS path
    MBED_OS_PATH="$CHIP_ROOT"/third_party/mbed-os/repo

    # Set Mbed OS posix socket submodule path
    MBED_OS_POSIX_SOCKET_PATH="$CHIP_ROOT"/third_party/mbed-os-posix-socket/repo

    # Set Mbed MCU boot path
    MBED_MCU_BOOT_PATH="$CHIP_ROOT"/third_party/mbed-mcu-boot/repo

    if [[ "$TYPE" == "boot" || "$TYPE" == "upgrade" ]]; then

        cd "$BOOTLOADER_ROOT_DIRECTORY"

        # Install mcuboot requirements (silently)
        pip install -q -r "$MBED_MCU_BOOT_PATH"/scripts/requirements.txt

        # Run mcuboot setup script
        python "$MBED_MCU_BOOT_PATH"/scripts/setup.py install

        # Create the signing keys is not exist
        if [ ! -f signing-keys.pem ] || [ ! -f signing_keys.c ]; then
            "$MBED_MCU_BOOT_PATH"/scripts/imgtool.py keygen -k signing-keys.pem -t rsa-2048 &&
                "$MBED_MCU_BOOT_PATH"/scripts/imgtool.py getpub -k signing-keys.pem >signing_keys.c
        fi

        ln -sfTr "$MBED_MCU_BOOT_PATH"/boot/mbed mcuboot

        # Generate config file for selected target, toolchain and hardware
        mbed-tools configure -t "$TOOLCHAIN" -m "$TARGET_BOARD" -o "$BOOTLOADER_BUILD_DIRECTORY" --mbed-os-path "$MBED_OS_PATH"

        # Remove old artifacts to force linking
        rm -rf "$BOOTLOADER_BUILD_DIRECTORY/chip-"*

        # Build application
        cmake -S . -B "$BOOTLOADER_BUILD_DIRECTORY" -GNinja -DCMAKE_BUILD_TYPE="$PROFILE" -DMBED_OS_PATH="$MBED_OS_PATH" -DMBED_MCU_BOOT_PATH="$MBED_MCU_BOOT_PATH"
        cmake --build "$BOOTLOADER_BUILD_DIRECTORY"

        cd "$CHIP_ROOT"/examples
    fi

    # Set Mbed OS posix socket submodule path
    MBED_OS_POSIX_SOCKET_PATH="$CHIP_ROOT"/third_party/mbed-os-posix-socket/repo

    if [[ "$TYPE" == "boot" || "$TYPE" == "upgrade" ]]; then
        ln -sfTr "$MBED_MCU_BOOT_PATH"/boot/mbed "$APP"/mbed/mcuboot
    fi

    # Generate config file for selected target, toolchain and hardware
    mbed-tools configure -t "$TOOLCHAIN" -m "$TARGET_BOARD" -p "$APP"/mbed/ -o "$BUILD_DIRECTORY" --mbed-os-path "$MBED_OS_PATH"

    # Remove old artifacts to force linking
    rm -rf "$BUILD_DIRECTORY/chip-"*

    # Build application
    cmake -S "$APP/mbed" -B "$BUILD_DIRECTORY" -GNinja -DCMAKE_BUILD_TYPE="$PROFILE" -DMBED_OS_PATH="$MBED_OS_PATH" -DMBED_OS_POSIX_SOCKET_PATH="$MBED_OS_POSIX_SOCKET_PATH" -DMBED_MCU_BOOT_PATH="$MBED_MCU_BOOT_PATH" -DMBED_APP_TYPE="$TYPE"
    cmake --build "$BUILD_DIRECTORY"

    if [[ "$TYPE" == "boot" || "$TYPE" == "upgrade" ]]; then
        APP_VERSION=$(jq '.config."version-number-str".value' $APP/mbed/mbed_app.json | tr -d '\\"')
        HEADER_SIZE=$(jq '.target_overrides.'\"${TARGET_BOARD}\"'."mcuboot.header-size"' $APP/mbed/mbed_app.json | tr -d \")
        SLOT_SIZE=$(jq '.target_overrides.'\"${TARGET_BOARD}\"'."mcuboot.slot-size"' $APP/mbed/mbed_app.json | tr -d \")
        # Signed the primary application
        "$MBED_MCU_BOOT_PATH"/scripts/imgtool.py sign -k "$BOOTLOADER_ROOT_DIRECTORY"/signing-keys.pem \
            --align ${TARGET_MEMORY_ALIGN[$TARGET_BOARD]} -v $APP_VERSION --header-size $(($HEADER_SIZE)) --pad-header -S $SLOT_SIZE \
            "$BUILD_DIRECTORY"/chip-mbed-"$APP"-example.hex "$BUILD_DIRECTORY"/chip-mbed-"$APP"-example-signed.hex

        # Create the factory firmware (bootlaoder + signed primary application)
        hexmerge.py -o "$BUILD_DIRECTORY"/chip-mbed-"$APP"-example.hex --no-start-addr "$BOOTLOADER_BUILD_DIRECTORY"/chip-mbed-bootloader.hex "$BUILD_DIRECTORY"/chip-mbed-"$APP"-example-signed.hex
    fi
fi

if [[ "$COMMAND" == *"flash"* ]]; then

    echo "Flash $APP app to $TARGET_BOARD target [$TOOLCHAIN toolchain, $PROFILE profile]"

    # Flash scripts path setup
    MBED_FLASH_SCRIPTS_PATH=$CHIP_ROOT/config/mbed/scripts

    if [[ "$APP" == "bootloader" ]]; then
        APP_PATH="$BOOTLOADER_BUILD_DIRECTORY"/chip-mbed-bootloader.elf
    else
        if [[ $BOOTLOADER ]]; then
            APP_PATH="$BUILD_DIRECTORY"/chip-mbed-"$APP"-example-boot.elf
        else
            APP_PATH="$BUILD_DIRECTORY"/chip-mbed-"$APP"-example.elf
        fi
    fi

    # Flash application
    "$OPENOCD_PATH"/bin/openocd -f "$MBED_FLASH_SCRIPTS_PATH/$TARGET_BOARD".tcl -c "program $APP_PATH verify reset exit"
fi
