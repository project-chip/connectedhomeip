#!/usr/bin/env bash

#
#    Copyright (c) 2023 Project CHIP Authors
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

# Build script for GN STM32 examples GitHub workflow.

set -e

echo_green() {
    echo -e "\033[0;32m$*\033[0m"
}

echo_blue() {
    echo -e "\033[1;34m$*\033[0m"
}

if [[ -z "${MATTER_ROOT}" ]]; then
    echo "Using default path for Matter root"
    CHIP_ROOT="$(dirname "$0")/../.."
else
    echo "Using ENV path for Matter root"
    CHIP_ROOT="$MATTER_ROOT"
fi

source "$CHIP_ROOT/scripts/activate.sh"

set -x
env
USE_WIFI=false

#ST_THREAD_TARGET=\""../st:ot-stm32-cert"\"
USAGE="./scripts/examples/gn_stm32_example.sh <AppRootFolder> <outputFolder> <stm32_board_name> [<Build options>]"

if [ "$#" == "0" ]; then
    echo "Build script for stm32 Matter apps
    Format:
    $USAGE

    <AppRootFolder>
        Root Location of the app e.g: examples/lighting-app/stm32/

    <outputFolder>
        Desired location for the output files

    <stm32_board_name>
        Identifier of the board for which this app is built
        Currently Supported :
            STM32WB5MM-DK

    <Build options> - optional noteworthy build options for stm32
        chip_build_libshell
            Enable libshell support. (Default false)
        chip_logging
            Current value (Default true)
        chip_openthread_ftd
            Use openthread Full Thread Device, else, use Minimal Thread Device. (Default true)
        stm32_sdk_root
            Location for an alternate or modified stm32 SDK
        enable_heap_monitoring
            Monitor & log memory usage at runtime. (Default false)
        enable_openthread_cli
            Enables openthread cli without matter shell. (Default true)
        kvs_max_entries
            Set the maxium Kvs entries that can be store in NVM (Default 75)
            Thresholds: 30 <= kvs_max_entries <= 255
        show_qr_code
            Enables QR code on LCD for devices with an LCD
        setupDiscriminator
            Discriminatoor value used for BLE connexion. (Default 3840)
        setupPinCode
            PIN code for PASE session establishment. (Default 20202021)
        enable_sleepy_device
            Enable Sleepy end device. (Default false)
            Must also set chip_openthread_ftd=false
        use_mxchip
            Build wifi example with MXCHIP extension board. (Default false)
        'import("//with_pw_rpc.gni")'
            Use to build the example with pigweed RPC
        OTA_periodic_query_timeout
            Periodic query timeout variable for OTA in seconds
        wifi_wpa3_only
            Support for WPA3 only mode
        Presets
        --sed
            enable sleepy end device, set thread mtd
            For minimum consumption, disable openthread cli and qr code
        --wifi <mxchip>
            build wifi example variant for given extension board
        --additional_data_advertising
            enable Addition data advertissing and rotating device ID
        --use_ot_lib
            use the STMicroelectronics openthread library
    "
elif [ "$#" -lt "2" ]; then
    echo "Invalid number of arguments
    Format:
    $USAGE"
else
    ROOT=$1
    OUTDIR=$2

    if [ "$#" -gt "2" ]; then
        STM32_BOARD=$3
        shift
    fi

    shift
    shift
    while [ $# -gt 0 ]; do
        case $1 in
            --wifi)
                if [ -z "$2" ]; then
                    echo "--wifi requires mxchip"
                    exit 1
                fi
                if [ "$2" = "mxchip" ]; then
                    optArgs+="use_mxchip=true"
                else
                    echo "Wifi usage: --wifi mxchip"
                    exit 1
                fi
                USE_WIFI=true
                shift
                shift
                ;;
            --sed)
                optArgs+="enable_sleepy_device=true chip_openthread_ftd=false "
                shift
                ;;
            --chip_enable_wifi_ipv4)
                optArgs+="chip_enable_wifi_ipv4=true "
                shift
                ;;
            --additional_data_advertising)
                optArgs+="chip_enable_additional_data_advertising=true chip_enable_rotating_device_id=true "
                shift
                ;;
            --use_ot_lib)
                optArgs+="use_st_thread_lib=true chip_openthread_target=$ST_THREAD_TARGET openthread_external_platform=\"""\" "
                shift
                ;;
            --use_ot_coap_lib)
                optArgs+="use_st_thread_lib=true chip_openthread_target=$ST_THREAD_TARGET openthread_external_platform=\"""\" use_thread_coap_lib=true "
                shift
                ;;
            *)
                if [ "$1" =~ *"use_mxchip=true"* ]; then
                    USE_WIFI=true
                fi

                optArgs+=$1" "
                shift
                ;;
        esac
    done

    if [ -z "$STM32_BOARD" ]; then
        echo "STM32_BOARD not defined"
        exit 1
    fi

    BUILD_DIR=$OUTDIR/$STM32_BOARD
    echo BUILD_DIR="$BUILD_DIR"
    if [ "$USE_WIFI" == true ]; then
        gn gen --check --fail-on-unused-args --export-compile-commands --root="$ROOT" --dotfile="$ROOT"/build_for_wifi_gnfile.gn --args="stm32_board=\"$STM32_BOARD\" $optArgs" "$BUILD_DIR"
    else
        # thread build
        #
        if [ -z "$optArgs" ]; then
            gn gen --check --fail-on-unused-args --export-compile-commands --root="$ROOT" --args="stm32_board=\"$STM32_BOARD\" treat_warnings_as_errors=false" --ide=json "$BUILD_DIR"
        else
            gn gen --check --fail-on-unused-args --export-compile-commands --root="$ROOT" --args="stm32_board=\"$STM32_BOARD\" $optArgs treat_warnings_as_errors=false" --ide=json "$BUILD_DIR"
        fi
    fi
    ninja -v -C "$BUILD_DIR"/

    #print stats
    arm-none-eabi-size "$BUILD_DIR"/*.elf

    arm-none-eabi-objcopy -O binary "$BUILD_DIR"/*.elf """$BUILD_DIR/*.elf.bin"

    set +x
    echo_green "Finished building target: ""$BUILD_DIR/*.elf"
fi
