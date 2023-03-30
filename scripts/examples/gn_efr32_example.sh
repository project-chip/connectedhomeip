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

# Build script for GN EFR32 examples GitHub workflow.

set -e

if [[ -z "${MATTER_ROOT}" ]]; then
    echo "Using default path for Matter root"
    CHIP_ROOT="$(dirname "$0")/../.."
else
    echo "Using ENV path for Matter root"
    CHIP_ROOT="$MATTER_ROOT"
fi

set -x
env
USE_WIFI=false
USE_DOCKER=false
USE_GIT_SHA_FOR_VERSION=true
USE_SLC=false
GN_PATH=gn
GN_PATH_PROVIDED=false

SILABS_THREAD_TARGET=\""../silabs:ot-efr32-cert"\"
USAGE="./scripts/examples/gn_efr32_example.sh <AppRootFolder> <outputFolder> <silabs_board_name> [<Build options>]"

if [ "$#" == "0" ]; then
    echo "Build script for EFR32 Matter apps
    Format:
    $USAGE

    <AppRootFolder>
        Root Location of the app e.g: examples/lighting-app/efr32/

    <outputFolder>
        Desired location for the output files

    <silabs_board_name>
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
        enable_openthread_cli
            Enables openthread cli without matter shell. (Default true)
        kvs_max_entries
            Set the maxium Kvs entries that can be store in NVM (Default 75)
            Thresholds: 30 <= kvs_max_entries <= 255
        show_qr_code
            Enables QR code on LCD for devices with an LCD
        enable_sleepy_device
            Enable Sleepy end device. (Default false)
            Must also set chip_openthread_ftd=false
        use_rs9116
            Build wifi example with extension board rs9116. (Default false)
        use_SiWx917
            Build wifi example with extension board SiWx917. (Default false)
        use_wf200
            Build wifi example with extension board wf200. (Default false)
        'import("//with_pw_rpc.gni")'
            Use to build the example with pigweed RPC
        ota_periodic_query_timeout_sec
            Periodic query timeout variable for OTA in seconds
        rs91x_wpa3_only
            Support for WPA3 only mode on RS91x
        sl_matter_version
            Use provided software version at build time
        sl_matter_version_str
            Set a Matter sotfware version string for the Silabs examples
            Used and formatted by default in this script.
        sl_hardware_version
            Use provided hardware version at build time
        siwx917_commissionable_data
            Build with the commissionable data given in DeviceConfig.h (only for SiWx917)
        Presets
        --sed
            enable sleepy end device, set thread mtd
            For minimum consumption, add --low-power
        --low-power
            disables all power consuming features for the most power efficient build
            This flag is to be used with --sed
        --wifi <wf200 | rs9116>
            build wifi example variant for given exansion board
        --additional_data_advertising
            enable Addition data advertissing and rotating device ID
        --use_ot_lib
            use the silabs openthread library
        --use_chip_lwip_lib
            use the chip lwip library
        --release
            Remove all logs and debugs features (including the LCD). Yield the smallest image size possible
        --docker
            Change GSDK root for docker builds
        --uart_log
            Forward Logs to Uart instead of RTT
        --slc_generate
            Generate files with SLC for current board and options Requires an SLC-CLI installation or running in Docker.
        --slc_reuse_files
            Use generated files without running slc again.

    "
elif [ "$#" -lt "2" ]; then
    echo "Invalid number of arguments
    Format:
    $USAGE"
else
    ROOT=$1
    OUTDIR=$2

    if [ "$#" -gt "2" ]; then
        SILABS_BOARD=$3
        shift
    fi

    shift
    shift
    while [ $# -gt 0 ]; do
        case $1 in
            --clean)
                DIR_CLEAN=true
                shift
                ;;
            --wifi)
                if [ -z "$2" ]; then
                    echo "--wifi requires rs9116 or SiWx917 or wf200"
                    exit 1
                fi
                if [ "$2" = "rs9116" ]; then
                    optArgs+="use_rs9116=true "
                elif [ "$2" = "SiWx917" ]; then
                    optArgs+="use_SiWx917=true "
                elif [ "$2" = "wf200" ]; then
                    optArgs+="use_wf200=true "
                else
                    echo "Wifi usage: --wifi rs9116|SiWx917|wf200"
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
            --low-power)
                optArgs+="chip_build_libshell=false enable_openthread_cli=false show_qr_code=false disable_lcd=true "
                shift
                ;;
            --chip_enable_wifi_ipv4)
                ipArgs="chip_enable_wifi_ipv4=true chip_inet_config_enable_ipv4=true "
                shift
                ;;
            --additional_data_advertising)
                optArgs+="chip_enable_additional_data_advertising=true chip_enable_rotating_device_id=true "
                shift
                ;;
            --use_ot_lib)
                optArgs+="use_silabs_thread_lib=true chip_openthread_target=$SILABS_THREAD_TARGET openthread_external_platform=\"""\" "
                shift
                ;;
            --use_ot_coap_lib)
                optArgs+="use_silabs_thread_lib=true chip_openthread_target=$SILABS_THREAD_TARGET openthread_external_platform=\"""\" use_thread_coap_lib=true "
                shift
                ;;
            --use_chip_lwip_lib)
                optArgs+="lwip_root=\""//third_party/connectedhomeip/third_party/lwip"\" "
                shift
                ;;
            # Option not to be used until ot-efr32 github is updated
            # --use_ot_github_sources)
            #   optArgs+="openthread_root=\"//third_party/connectedhomeip/third_party/openthread/ot-efr32/openthread\" openthread_efr32_root=\"//third_party/connectedhomeip/third_party/openthread/ot-efr32/src/src\""
            #    shift
            #    ;;
            --release)
                optArgs+="is_debug=false disable_lcd=true chip_build_libshell=false enable_openthread_cli=false use_external_flash=false chip_logging=false silabs_log_enabled=false "
                shift
                ;;
            --docker)
                optArgs+="efr32_sdk_root=\"$GSDK_ROOT\" "
                USE_DOCKER=true
                shift
                ;;
            --uart_log)
                optArgs+="sl_uart_log_output=true "
                shift
                ;;

            --slc_generate)
                optArgs+="slc_generate=true "
                USE_SLC=true
                shift
                ;;
            --slc_reuse_files)
                optArgs+="slc_reuse_files=true "
                USE_SLC=true
                shift
                ;;
            --gn_path)
                if [ -z "$2" ]; then
                    echo "--gn_path requires a path to GN"
                    exit 1
                else
                    GN_PATH="$2"
                fi
                GN_PATH_PROVIDED=true
                shift
                shift
                ;;
            *"sl_matter_version_str="*)
                optArgs+="$1 "
                USE_GIT_SHA_FOR_VERSION=false
                shift
                ;;
            *)
                if [ "$1" =~ *"use_rs9116=true"* ] || [ "$1" =~ *"use_SiWx917=true"* ] || [ "$1" =~ *"use_wf200=true"* ]; then
                    USE_WIFI=true
                fi
                optArgs+=$1" "
                shift
                ;;
        esac
    done

    if [ -z "$SILABS_BOARD" ]; then
        echo "SILABS_BOARD not defined"
        exit 1
    fi

    if [ "$USE_GIT_SHA_FOR_VERSION" == true ]; then
        {
            ShortCommitSha=$(git describe --always --dirty --exclude '*')
            branchName=$(git rev-parse --abbrev-ref HEAD)
            optArgs+="sl_matter_version_str=\"v1.0-$branchName-$ShortCommitSha\" "
        } &>/dev/null
    fi

    if [ "$USE_SLC" == true ] && [ "$GN_PATH_PROVIDED" == false ]; then
        GN_PATH=./.environment/cipd/packages/pigweed/gn
    fi

    BUILD_DIR=$OUTDIR/$SILABS_BOARD
    echo BUILD_DIR="$BUILD_DIR"

    if [ "$DIR_CLEAN" == true ]; then
        rm -rf "$BUILD_DIR"
    fi

    if [ "$USE_WIFI" == true ]; then
        # wifi build
        # NCP mode EFR32 + wifi module
        optArgs+="$ipArgs"
        "$GN_PATH" gen --check --fail-on-unused-args --export-compile-commands --root="$ROOT" --dotfile="$ROOT"/build_for_wifi_gnfile.gn --args="silabs_board=\"$SILABS_BOARD\" $optArgs" "$BUILD_DIR"
    else
        # OpenThread/SoC build
        #
        if [ "$USE_DOCKER" == true ]; then
            optArgs+="openthread_root=\"$GSDK_ROOT/util/third_party/openthread\" "
        fi

        if [ -z "$optArgs" ]; then
            "$GN_PATH" gen --check --script-executable="/usr/bin/python3" --root="$ROOT" --args="silabs_board=\"$SILABS_BOARD\"" "$BUILD_DIR"
        else
            "$GN_PATH" gen --check --script-executable="/usr/bin/python3" --root="$ROOT" --args="silabs_board=\"$SILABS_BOARD\" $optArgs" "$BUILD_DIR"
        fi
    fi

    # Activation needs to be after SLC generation which is done in gn gen.
    # Zap generation requires activation and is done in the build phase
    source "$CHIP_ROOT/scripts/activate.sh"

    ninja -v -C "$BUILD_DIR"/
    #print stats
    arm-none-eabi-size -A "$BUILD_DIR"/*.out

fi
