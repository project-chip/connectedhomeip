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
# Build script for GN BL602 examples GitHub workflow.
source "$(dirname "$0")/../../scripts/activate.sh"

set -x
env

# Build steps
EXAMPLE_DIR=$1
shift
OUTPUT_DIR=out/example_app
BL602_BOARD=BL-HWC-G1
MATTER_BL_ROOT=$PWD
export BL602_SDK_ROOT="$MATTER_BL_ROOT"/third_party/bouffalolab/bl602_sdk/repo
export PATH="$BL602_SDK_ROOT/toolchain/riscv/Linux/bin:$PATH"

if [[ ! -z "$1" ]]; then
    OUTPUT_DIR=$1
    shift
fi

GN_ARGS=()
NINJA_ARGS=()

for arg; do
    case $arg in
        -v)
            NINJA_ARGS+=(-v)
            ;;
        *=*)
            GN_ARGS+=("$arg")
            ;;
        *import*)
            GN_ARGS+=("$arg")
            ;;
        *)
            echo >&2 "invalid argument: $arg"
            exit 2
            ;;
    esac
done

#gn clean out/lighting_app_bl602
gn gen "$OUTPUT_DIR" --root="$EXAMPLE_DIR" --args="${GN_ARGS[*]}"
ninja -C "$OUTPUT_DIR" "${NINJA_ARGS[@]}"
