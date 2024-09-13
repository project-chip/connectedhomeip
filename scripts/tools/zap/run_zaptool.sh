#!/usr/bin/env bash

#
# Copyright (c) 2021-2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

readonly ZAP_CMD_DEFAULT="zap"

ZAP_CMD="$ZAP_CMD_DEFAULT"
ZAP_FILE=""

function _get_fullpath() {
    cd "$(dirname "$1")" >/dev/null && echo "$PWD/$(basename "$1")"
}

function _print_usage() {
    local name="$(basename "${0}")"
    local status="${1}"

    echo "Usage: $name [ option ... ] [ <ZAP file path> ]"

    if [ "$status" -ne 0 ]; then
        echo "Try '$name -h' for more information."
    fi

    if [ "$status" -ne 1 ]; then
        echo ""
        echo "ZAP Tool Wrapper"
        echo ""
        echo " General Options:"
        echo ""
        echo "  -h, --help                  Display this help, then exit."
        echo "  --zap ZAP                   Use the zap program ZAP as the zap executable "
        echo "                              to run (default: $ZAP_CMD_DEFAULT)."
        echo ""
        echo " Some influential environment variables:"
        echo ""
        echo "  ZAP_DEVELOPMENT_PATH        The path to a zap development environment. This "
        echo "                              is likely a zap checkout, used for local "
        echo "                              development (current: \"${ZAP_DEVELOPMENT_PATH:-<null>}\")."
        echo ""
        echo "                              If set, this overrides both the '--zap' option "
        echo "                              and the 'ZAP_INTALL_PATH' environment variable."
        echo "  ZAP_INSTALL_PATH            The path where the 'zap' executable exists. "
        echo "                              This may be used if 'zap' is NOT in the current "
        echo "                              PATH (current: \"${ZAP_INSTALL_PATH:-<null>}\")."
        echo ""
        echo "                              If set, this overrides the '--zap' option."
        echo ""
    fi

    exit "$status"
}

# Main

set -e

# Command Line Option Parsing

while [ "${#}" -gt 0 ]; do

    case "${1}" in

        -h | --help)
            _print_usage 0
            ;;

        --zap)
            ZAP_CMD="${2}"
            shift 2
            ;;

        -*)
            echo "ERROR: Unknown or invalid option: '${1}'" >&2
            _print_usage 1
            ;;

        *)
            if [ -z "$ZAP_FILE" ]; then
                ZAP_FILE="${1}"
                shift 1
            else
                echo "ERROR: Unexpected argument: '${1}'" >&2
                _print_usage 1
            fi
            ;;

    esac

done

SCRIPT_PATH="$(_get_fullpath "$0")"
CHIP_ROOT="${SCRIPT_PATH%/scripts/tools/zap/run_zaptool.sh}"
[[ -n "${ZAP_FILE}" ]] && ZAP_ARGS="-i \"$(_get_fullpath "$ZAP_FILE")\"" || ZAP_ARGS=""

if [[ -z "$ZAP_INSTALL_PATH" && -n "$PW_ZAP_CIPD_INSTALL_DIR" ]]; then
    ZAP_INSTALL_PATH="$PW_ZAP_CIPD_INSTALL_DIR"
fi

if [ -n "$ZAP_DEVELOPMENT_PATH" ]; then
    WORKING_DIR=$ZAP_DEVELOPMENT_PATH
    ZAP_CMD="node src-script/zap-start.js"
    # Make sure we don't try to munge the package.json in the ZAP repo.
    export ZAP_SKIP_REAL_VERSION=1

    "$CHIP_ROOT"/scripts/tools/zap/zap_bootstrap.sh
elif [ -n "$ZAP_INSTALL_PATH" ]; then
    if [[ "$OSTYPE" == "darwin"* ]]; then
        ZAP_CMD="$ZAP_INSTALL_PATH/zap.app/Contents/MacOS/zap"
    else
        ZAP_CMD="$ZAP_INSTALL_PATH/zap"
    fi
    WORKING_DIR="$CHIP_ROOT"
else
    [ -z "$ZAP_CMD" ] && ZAP_CMD="$ZAP_CMD_DEFAULT"
    WORKING_DIR="$CHIP_ROOT"
fi

(
    cd "$WORKING_DIR"

    echo "CMD: $ZAP_CMD"
    echo "ARGS: $ZAP_ARGS"

    if [[ "${ZAP_ARGS[*]}" == *"/all-clusters-app.zap"* ]]; then
        ZCL_FILE="$CHIP_ROOT/src/app/zap-templates/zcl/zcl-with-test-extensions.json"
    else
        ZCL_FILE="$CHIP_ROOT/src/app/zap-templates/zcl/zcl.json"
    fi

    bash -c " \
        $ZAP_CMD \
        --logToStdout \
        --gen \"$CHIP_ROOT/src/app/zap-templates/app-templates.json\" \
        --zcl \"$ZCL_FILE\" \
        $ZAP_ARGS \
    "
)
