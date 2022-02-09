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

# Build script for GN EFT32 examples GitHub workflow.

set -e
source "$(dirname "$0")/../../scripts/activate.sh"

set -x
env
USE_WIFI=false

USAGE="./scripts/examples/gn_efr32_example.sh <AppRootFolder> <outputFolder> <efr32_board_name> [<Build options>]"

if [ "$#" == "0" ]; then
    echo "Build script for EFR32 Matter apps
    Format:
    $USAGE

    <AppRootFolder>
        Root Location of the app e.g: examples/lighting-app/efr32/

    <outputFolder>
        Desired location for the output files

    <efr32_board_name>
        Identifier of the board for which this app is built
        Currently Supported :
            BRD4161A
            BRD4163A
            BRD4164A
            BRD4166A
            BRD4170A
            BRD4186A
            BRD4187A
            BRD4304A

    <Build options> - optional noteworthy build options for EFR32
        chip_build_libshell
            Enable libshell support. (Default false)
        chip_logging
            Current value (Default true)
        chip_openthread_ftd
            Use openthread Full Thread Device, else, use Minimal Thread Device. (Default true)
        efr32_sdk_root
            Location for an alternate or modified efr32 SDK
        enable_heap_monitoring
            Monitor & log memory usage at runtime. (Default false)
        setupDiscriminator
            Discriminatoor value used for BLE connexion. (Default 3840)
        setupPinCode
            PIN code for PASE session establishment. (Default 73141520)
        enable_sleepy_device
            Enable Sleepy end device. (Default false)
            Must also set chip_openthread_ftd=false
        use_rs911x
            Build wifi example with extension board rs911x. (Default false)
        use_wf200
            Build wifi example with extension board wf200. (Default false)
        OTA_periodic_query_timeout

        'import("//with_pw_rpc.gni")'
            Use to build the example with pigweed RPC

        Presets
        --sed
            enable sleepy end device and set thread mtd
        --wifi <wf200 | rs911x>
            build wifi example variant for given exansion board
    "
elif [ "$#" -lt "2" ]; then
    echo "Invalid number of arguments
    Format:
    $USAGE"
else
    ROOT=$1
    OUTDIR=$2

    if [ "$#" -gt "2" ]; then
        EFR32_BOARD=$3
        shift
    fi

    shift
    shift
    while [ $# -gt 0 ]; do
        case $1 in
            --wifi)
                if [ -z "$2" ]; then
                    echo "--wifi requires rs911x or wf200"
                    exit 1
                fi
                if [ "$2" = "rs911x" ]; then
                    optArgs+="use_rs911x=true"
                elif [ "$2" = "wf200" ]; then
                    optArgs+="use_wf200=true"
                else
                    echo "Wifi usage: --wifi rs911x|wf200"
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
            *)
                if [ "$1" =~ *"use_rs911x=true"* ] || [ "$1" =~ *"use_wf200=true"* ]; then
                    USE_WIFI=true
                fi
                if [ "$1" =~ *"OTA_periodic_query_timeout"* ]; then
                    USE_OTA_TIMEOUT=true
                    echo "$OTA_periodic_query_timeout"
                fi
                optArgs+=$1" "
                shift
                ;;
        esac
    done

    if [ -z "$EFR32_BOARD" ]; then
        echo "EFR32_BOARD not defined"
        exit 1
    fi

    BUILD_DIR=$OUTDIR/$EFR32_BOARD
    echo BUILD_DIR="$BUILD_DIR"
    if [ "$USE_WIFI" == true ]; then
        gn gen --check --fail-on-unused-args --root="$ROOT" --dotfile="$ROOT"/build_for_wifi_gnfile.gn --args="efr32_board=\"$EFR32_BOARD\" $optArgs" "$BUILD_DIR"
    else
        # thread build
        #
        if [ -z "$optArgs" ]; then
            gn gen --check --fail-on-unused-args --root="$ROOT" --args="efr32_board=\"$EFR32_BOARD\"" "$BUILD_DIR"
        else
            gn gen --check --fail-on-unused-args --root="$ROOT" --args="efr32_board=\"$EFR32_BOARD\" $optArgs" "$BUILD_DIR"
        fi
    fi
    ninja -v -C "$BUILD_DIR"/
    #print stats
    arm-none-eabi-size -A "$BUILD_DIR"/*.out

fi
