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

# Build script for GN test examples GitHub workflow.

CHIP_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"/../..

INPUT_DIR="$CHIP_ROOT/examples/placeholder/linux"
OUTPUT_DIR="$CHIP_ROOT/zzz_generated/placeholder"

source "$CHIP_ROOT/scripts/activate.sh"

APP_DIR=$1

function runZAP() {
    ZAP_INPUT_FILE=$INPUT_DIR/apps/$APP_DIR/config.zap
    ZAP_OUTPUT_DIR=$OUTPUT_DIR/$APP_DIR/zap-generated

    # Create the folder to host the generated content if needed
    mkdir -p "$ZAP_OUTPUT_DIR"

    # Generates the generic files for the given zap configuration
    "$CHIP_ROOT"/scripts/tools/zap/generate.py "$ZAP_INPUT_FILE" -o "$ZAP_OUTPUT_DIR"
}

function runGN() {
    GN_ARGS="chip_tests_zap_config=\"$APP_DIR\""
    GN_ARGS+="chip_project_config_include_dirs=[\"$INPUT_DIR/apps/$APP_DIR/include\", \"$CHIP_ROOT/config/standalone\"]"
    GN_ARGS+="chip_config_network_layer_ble=false"

    gn gen --check --fail-on-unused-args --root=examples/placeholder/linux "$CHIP_ROOT/out/$APP_DIR" --args="$GN_ARGS"
}

function runNinja() {
    ninja -C "$CHIP_ROOT/out/$APP_DIR"
}

function runAll() {
    runZAP
    runGN
    runNinja
}

runAll
