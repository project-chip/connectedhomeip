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

set -e

# Build script for GN Bouffalolab examples GitHub workflow.

MATTER_ROOT=$(dirname "$(readlink -f "$0")")/../../
source "$MATTER_ROOT/scripts/activate.sh"

bl602_boards=("BL602-IoT-Matter-V1" "BL602-NIGHT-LIGHT")
bl602_module_type="BL602"

bl702_boards=("XT-ZB6-DevKit" "BL706-NIGHT-LIGHT" "BL706-ETH" "BL706-WIFI")
bl702_modules=("BL702" "BL706C-22")
bl702_module_type="BL706C-22"

bl702l_boards=("BL704L-DVK")
bl702l_modules=("BL702L")
bl702l_module_type="BL704l"

print_help() {
    bl602_boards_help=""
    for board in "${bl602_boards[@]}"; do
        bl602_boards_help=$bl602_boards_help$board"\n            "
    done
    bl702_boards_help=""
    for board in "${bl702_boards[@]}"; do
        bl702_boards_help=$bl702_boards_help$board"\n            "
    done
    bl702l_boards_help=""
    for board in "${bl702l_boards[@]}"; do
        bl702l_boards_help=$bl702l_boards_help$board"\n            "
    done

    echo -e "Build script for Bouffalolab Matter examples
    Format:
    ./scripts/examples/gn_bouffalolab_example.sh <Example folder> <Output folder> <Bouffalolab_board_name> [<Build options>]

    <Example folder>
        Folder of example application, e.g: lighting-app

    <Output folder>
        Desired location for the output files

    <Bouffalolab_board_name>
        Identifier of the board for which this app is built
        Currently Supported :
            $bl602_boards_help
            $bl702_boards_help
            $bl702l_boards_help
    <Build options> - optional noteworthy build options for Bouffalolab IOT Matter examples
        chip_build_libshell
            Enable libshell support. (Default false)
        chip_openthread_ftd
            Use openthread Full Thread Device, else, use Minimal Thread Devic. (Default true)
        enable_heap_monitoring
            Monitor & log memory usage at runtime. (Default false)
        setupDiscriminator
            Discriminatoor value used for commission. (Default 3840)
        setupPinCode
            PIN code for PASE session establishment. (Default 20202021)
        'import("//with_pw_rpc.gni")'
            Use to build the example with pigweed RPC
        OTA_periodic_query_timeout
            Periodic query timeout variable for OTA in seconds
        enable_psram
            Enable PSRAM memory. (Default true for BL702/BL706)
        baudrate
            UART baudrate for log output and UART shell command, e.g, baudrate=2000000, by default.
        module_type
            Bouffalolab chip module.
    "
}

if [ "$#" -lt "3" ]; then
    print_help
else
    example_name=$1
    output_folder=$2
    board_name=$3
    bouffalo_chip=
    module_type=
    baudrate=2000000
    optArgs=""

    optArgs=custom_toolchain=\"$MATTER_ROOT/config/bouffalolab/toolchain:riscv_gcc\"

    shift
    shift
    shift

    while [ $# -gt 0 ]; do
        if [[ "$1" == "module_type"* ]]; then
            module_type=$(echo "$1" | awk -F'=' '{print $2}')

            shift
            continue
        fi
        if [[ "$1" == "baudrate"* ]]; then
            baudrate=$(echo "$1" | awk -F'=' '{print $2}')

            shift
            continue
        fi

        optArgs=$optArgs$1" "
        shift
    done

    if [[ "${bl602_boards[@]}" =~ "$board_name" ]]; then
        bouffalo_chip="bl602"

        optArgs=board=\"$board_name\"" "$optArgs
        optArgs=module_type=\"$bl602_module_type\"" "$optArgs
        optArgs=baudrate=\"$baudrate\"" "$optArgs
        optArgs=baudrate=\"$baudrate\"" "$optArgs
        optArgs=chip_enable_openthread=false" "$optArgs
        optArgs=chip_enable_wifi=true" "$optArgs

    elif [[ "${bl702_boards[@]}" =~ "$board_name" ]]; then
        bouffalo_chip="bl702"

        optArgs=board=\"$board_name\"" "$optArgs

        if [[ "$module_type" != "" ]]; then
            if [[ ! "${bl702_modules[@]}" =~ "$module_type" ]]; then
                echo "Module $module_type is not supported."
                exit 1
            fi

            optArgs=module_type=\"$module_type\"" "$optArgs
        fi

        optArgs=baudrate=\"$baudrate\"" "$optArgs

        if [[ "$board_name" == "BL706-ETH" ]]; then
            optArgs=chip_config_network_layer_ble=false" "$optArgs
            optArgs=chip_enable_openthread=false" "$optArgs
            optArgs=chip_enable_wifi=false" "$optArgs
        elif [[ "$board_name" == "BL706-WIFI" ]]; then
            optArgs=chip_enable_openthread=false" "$optArgs
            optArgs=chip_enable_wifi=true" "$optArgs
        else
            optArgs=chip_enable_openthread=true" "$optArgs
            optArgs=chip_enable_wifi=false" "$optArgs
        fi

    elif [[ "${bl702l_boards[@]}" =~ "$board_name" ]]; then
        bouffalo_chip="bl702l"

        optArgs=board=\"$board_name\"" "$optArgs

        if [[ "$module_type" != "" ]]; then
            if [[ ! "${bl702l_modules[@]}" =~ "$module_type" ]]; then
                echo "Module $module_type is not supported."
                exit 1
            fi

            optArgs=module_type=\"$module_type\"" "$optArgs
        fi

        optArgs=baudrate=\"$baudrate\"" "$optArgs
        optArgs=chip_enable_openthread=true" "$optArgs
        optArgs=chip_enable_wifi=false" "$optArgs

    else
        echo "Board $board_name is not supported."
        exit 1
    fi

    if [[ "$BOUFFALOLAB_SDK_ROOT" == "" ]]; then

        echo -e "\e[31mPlease make sure Bouffalo Lab SDK installs as below:\e[0m"
        echo -e "\e[31m\tcd third_party/bouffalolab/repo\e[0m"
        echo -e "\e[31m\tsudo bash scripts/setup.sh\e[0m"

        echo -e "\e[31mPlease make sure BOUFFALOLAB_SDK_ROOT exports before building as below:\e[0m"
        echo -e "\e[31m\texport BOUFFALOLAB_SDK_ROOT=/opt/bouffalolab_sdk\e[0m"

        exit 1
    fi
    optArgs=$optArgs' bouffalolab_sdk_root="'$BOUFFALOLAB_SDK_ROOT'"'

    example_dir=$MATTER_ROOT/examples/$example_name/bouffalolab/$bouffalo_chip
    output_dir=$MATTER_ROOT/$output_folder

    gn gen --check --fail-on-unused-args --export-compile-commands --root="$example_dir" "$output_dir" --args="${optArgs[*]}"

    ninja -C "$output_dir"
fi
