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

set -e
# Build script for GN ASR examples GitHub workflow.
source "$(dirname "$0")/../../scripts/activate.sh"

# Install required software

set -x
env

#default
if [ -z "$ASR_IC" ]; then
    echo "ASR_IC not set! default asr582x"
    export ASR_IC=asr582x
fi
if [ -z "$ASR_BOARD" ]; then
    if [ "$ASR_IC" = "asr582x" ]; then
        echo "ASR_BOARD not set! default ASR582X"
        export ASR_BOARD=ASR582X
    elif [ "$ASR_IC" = "asr595x" ]; then
        echo "ASR_BOARD not set! default ASR595X"
        export ASR_BOARD=ASR595X
    fi
fi
if [ -z "$TOOLCHAIN_PATH" ]; then
    echo "TOOLCHAIN_PATH not set! set default"
    if [ "$ASR_IC" = "asr582x" ]; then
        export TOOLCHAIN_PATH=/workspace/compiler/gcc-arm-none-eabi-9-2019-q4-major/bin/
    elif [ "$ASR_IC" = "asr595x" ]; then
        export TOOLCHAIN_PATH=/workspace/compiler/asr_riscv_gnu_toolchain_10.2_ubuntu-16.04/bin/
    fi
fi

# Build steps
EXAMPLE_DIR=$1
OUTPUT_DIR=$2

USAGE="./scripts/examples/gn_asr_example.sh <AppRootFolder> <outputFolder> [<Build options>]"

optArgs="treat_warnings_as_errors=false "
optArgs+="custom_toolchain=\"//../../../config/asr/toolchain:asrtoolchain\" "

if [ "$#" == "0" ]; then
    echo "Build script for ASR Matter apps
    Format:
    $USAGE

    <AppRootFolder>
        Root Location of the app e.g: examples/lighting-app/asr/

    <outputFolder>
        Desired location for the output files

    <Build options> - optional noteworthy build options for ASR
        chip_build_libshell
            Enable libshell support. (Default false)
        setupDiscriminator
            Discriminatoor value used for BLE connexion. (Default 3840)
        setupPinCode
            PIN code for PASE session establishment. (Default 20202021)
        chip_logging
            Current value (Default true)
        chip_use_factory
            Current value (Default false)
    "
elif [ "$#" -lt "2" ]; then
    echo "Invalid number of arguments
    Format:
    $USAGE"
else
    shift
    shift

    while [ $# -gt 0 ]; do

        optArgs+=$1" "
        shift
    done

    gn gen --check --fail-on-unused-args "$OUTPUT_DIR" --root="$EXAMPLE_DIR" --args="$optArgs"
    ninja -C "$OUTPUT_DIR"

    #print stats
    if [ "$ASR_IC" = "asr582x" ]; then
        $TOOLCHAIN_PATH/arm-none-eabi-size -A "$OUTPUT_DIR"/*.out
    elif [ "$ASR_IC" = "asr595x" ]; then
        $TOOLCHAIN_PATH/riscv-asr-elf-size -A "$OUTPUT_DIR"/*.out
    fi
fi
