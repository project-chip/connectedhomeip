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

if [[ -z "${PW_ENVIRONMENT_ROOT}" ]]; then
    echo "Using the bootstrapped pigweed ENV in Matter root"
    PW_PATH="$CHIP_ROOT/.environment/cipd/packages/pigweed"
else
    echo "Using provided $PW_ENVIRONMENT_ROOT as Pigweed ENV root"
    PW_PATH="$PW_ENVIRONMENT_ROOT/cipd/packages/pigweed"
fi

set -x
env
USE_WIFI=false
USE_DOCKER=false
USE_GIT_SHA_FOR_VERSION=true
USE_SLC=false
GN_PATH="$PW_PATH/gn"
USE_BOOTLOADER=false
DOTFILE=".gn"

SILABS_THREAD_TARGET=\""../silabs:ot-efr32-cert"\"
USAGE="./scripts/examples/gn_silabs_example.sh <AppRootFolder> <outputFolder> <silabs_board_name> [<Build options>]"

PROTOCOL_DIR_SUFFIX="thread"
NCP_DIR_SUFFIX=""

if [ "$#" == "0" ]; then
    echo "Build script for EFR32 Matter apps
    Format:
    $USAGE

    <AppRootFolder>
        Root Location of the app e.g: examples/lighting-app/silabs/

    <outputFolder>
        Desired location for the output files

    <silabs_board_name>
        Identifier of the board for which this app is built
        Currently Supported :
            BRD4186A
            BRD4187A
            BRD4186C
            BRD4187C
            BRD2601B
            BRD2703A
            BRD2704A
            BRD4316A
            BRD4317A
            BRD4318A
            BRD4319A


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
        chip_enable_icd_server
            Configure has a Intermitently connected device. (Default false)
            Must also set chip_openthread_ftd=false
        enable_synchronized_sed
            Enable Synchronized Sleepy end device. (Default false)
            Must also set chip_enable_icd_server=true chip_openthread_ftd=false
            --icd can be used to configure both arguments
        use_rs9116
            Build wifi example with extension board rs9116. (Default false)
        use_SiWx917
            Build wifi example with extension board SiWx917. (Default false)
        use_wf200
            Build wifi example with extension board wf200. (Default false)
        use_pw_rpc
            Use to build the example with pigweed RPC
        ota_periodic_query_timeout_sec
            Periodic query timeout variable for OTA in seconds
        rs91x_wpa3_transition
            Support for WPA3 transition mode on RS91x
        slc_gen_path
            Allow users to define a path where slc generates board files. (requires --slc_generate or --slc_reuse_files)
            (default: /third_party/silabs/slc_gen/<board>/)
        sl_pre_gen_path
            Allow users to define a path to pre-generated board files
            (default: third_party/silabs/matter_support/board-support/<family>/<board>/)
        sl_matter_version
            Use provided software version at build time
        sl_matter_version_str
            Set a Matter sotfware version string for the Silabs examples
            Used and formatted by default in this script.
        sl_hardware_version
            Use provided hardware version at build time
        siwx917_commissionable_data
            Build with the commissionable data given in DeviceConfig.h (only for SiWx917)
        si91x_alarm_based_wakeup
            Enable the Alarm Based Wakeup for 917 SoC when sleep is enabled (Default false)
        si91x_alarm_periodic_time
            Periodic time at which the 917 SoC should wakeup (Default: 30sec)
        Presets
        --icd
            enable ICD features, set thread mtd
            For minimum consumption, add --low-power
        --low-power
            disables all power consuming features for the most power efficient build
            This flag is to be used with --icd
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
        --bootloader
            Add bootloader to the generated image.


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

                NCP_DIR_SUFFIX="/"$2
                USE_WIFI=true
                optArgs+="chip_device_platform =\"efr32\" chip_crypto_keystore=\"psa\" "
                shift
                shift
                ;;
            --icd)
                optArgs+="chip_enable_icd_server=true chip_openthread_ftd=false sl_enable_test_event_trigger=true "
                shift
                ;;
            --low-power)
                optArgs+="chip_build_libshell=false enable_openthread_cli=false show_qr_code=false disable_lcd=true "
                shift
                ;;
            --chip_enable_wifi_ipv4)
                optArgs="chip_enable_wifi_ipv4=true chip_inet_config_enable_ipv4=true "
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
            --bootloader)
                USE_BOOTLOADER=true
                shift
                ;;
            --docker)
                optArgs+="efr32_sdk_root=\"$GSDK_ROOT\" "
                optArgs+="wiseconnect_sdk_root=\"$WISECONNECT_SDK_ROOT\" "
                optArgs+="wifi_sdk_root=\"$WIFI_SDK_ROOT\" "
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
            --use_pw_rpc)
                optArgs+="import(\"//with_pw_rpc.gni\") "
                shift
                ;;
            --slc_reuse_files)
                optArgs+="slc_reuse_files=true "
                shift
                ;;
            --gn_path)
                if [ -z "$2" ]; then
                    echo "--gn_path requires a path to GN"
                    exit 1
                else
                    GN_PATH="$2"
                fi
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
                    # NCP Mode so base MCU is an EFR32
                    optArgs+="chip_device_platform =\"efr32\" "
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

    # 917 exception. TODO find a more generic way
    if [ "$SILABS_BOARD" == "BRD4338A" ] || [ "$SILABS_BOARD" == "BRD2605A" ] || [ "$SILABS_BOARD" == "BRD4343A" ] || [ "$SILABS_BOARD" == "BRD4342A" ]; then
        echo "Compiling for 917 WiFi SOC"
        USE_WIFI=true
    fi

    if [ "$USE_GIT_SHA_FOR_VERSION" == true ]; then
        {
            ShortCommitSha=$(git describe --always --dirty --exclude '*')
            branchName=$(git rev-parse --abbrev-ref HEAD)
            optArgs+="sl_matter_version_str=\"v1.3-$branchName-$ShortCommitSha\" "
        } &>/dev/null
    fi

    if [ "$USE_SLC" == false ]; then
        # Activation needs to be after SLC generation which is done in gn gen.
        # Zap generation requires activation and is done in the build phase
        source "$CHIP_ROOT/scripts/activate.sh"
    fi

    if [ "$USE_WIFI" == true ]; then
        DOTFILE="$ROOT/build_for_wifi_gnfile.gn"
        PROTOCOL_DIR_SUFFIX="wifi"
    else
        DOTFILE="$ROOT/openthread.gn"
    fi

    PYTHON_PATH="$(which python3)"
    BUILD_DIR=$OUTDIR/$PROTOCOL_DIR_SUFFIX/$SILABS_BOARD$NCP_DIR_SUFFIX
    echo BUILD_DIR="$BUILD_DIR"

    if [ "$DIR_CLEAN" == true ]; then
        rm -rf "$BUILD_DIR"
    fi

    if [ "$USE_DOCKER" == true ] && [ "$USE_WIFI" == false ]; then
        echo "Switching OpenThread ROOT"
        optArgs+="openthread_root=\"$GSDK_ROOT/util/third_party/openthread\" "
    fi

    "$GN_PATH" gen --check --script-executable="$PYTHON_PATH" --fail-on-unused-args --add-export-compile-commands=* --root="$ROOT" --dotfile="$DOTFILE" --args="silabs_board=\"$SILABS_BOARD\" $optArgs" "$BUILD_DIR"

    if [ "$USE_SLC" == true ]; then
        # Activation needs to be after SLC generation which is done in gn gen.
        # Zap generation requires activation and is done in the build phase
        source "$CHIP_ROOT/scripts/activate.sh"
    fi

    ninja -C "$BUILD_DIR"/
    #print stats
    arm-none-eabi-size -A "$BUILD_DIR"/*.out

    # add bootloader to generated image
    if [ "$USE_BOOTLOADER" == true ]; then

        binName=""
        InternalBootloaderBoards=("BRD4337A" "BRD2704A" "BRD2703A" "BRD4319A")
        bootloaderPath=""
        commanderPath=""
        # find the matter root folder
        if [ -z "$MATTER_ROOT" ]; then
            MATTER_ROOT="$CHIP_ROOT"
        fi

        # set commander path
        if [ -z "$COMMANDER_PATH" ]; then
            commanderPath="commander"
        else
            commanderPath="$COMMANDER_PATH"
        fi

        # search bootloader directory for the respective bootloaders for the input board
        bootloaderFiles=("$(find "$MATTER_ROOT/third_party/silabs/matter_support/board-support/efr32/bootloader_binaries/" -maxdepth 1 -name "*$SILABS_BOARD*" | tr '\n' ' ')")

        if [ "${#bootloaderFiles[@]}" -gt 1 ]; then
            for i in "${!bootloaderFiles[@]}"; do
                # if a variant of the bootloader that uses external flash exists, use that one.
                if [[ "${bootloaderFiles[$i]}" =~ .*"spiflash".* ]]; then
                    bootloaderPath="${bootloaderFiles[$i]}"
                    break
                fi
            done
        elif [ "${#bootloaderFiles[@]}" -eq 0 ]; then
            echo "A bootloader for the $SILABS_BOARD currently doesn't exist!"
        else
            bootloaderPath="${bootloaderFiles[0]}"
        fi
        echo "$bootloaderPath"
        binName="$(find "$BUILD_DIR" -type f -name "*.s37")"
        echo "$binName"
        "$commanderPath" convert "$binName" "$bootloaderPath" -o "$binName"
    fi
fi
