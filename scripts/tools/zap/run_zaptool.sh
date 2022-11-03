#!/bin/bash

#
# SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

function _get_fullpath() {
    cd "$(dirname "$1")" && echo "$PWD/$(basename "$1")"
}

set -e

[[ "$1" == "--help" ]] && {
    echo "Usage: $0 [ZAP-file-path]" >&2
    exit 0
}

SCRIPT_PATH="$(_get_fullpath "$0")"
CHIP_ROOT="${SCRIPT_PATH%/scripts/tools/zap/run_zaptool.sh}"
[[ -n "$1" ]] && ZAP_ARGS=(-i "$(_get_fullpath "$1")") || ZAP_ARGS=()

(

    "$CHIP_ROOT"/scripts/tools/zap/zap_bootstrap.sh

    cd "$CHIP_ROOT/third_party/zap/repo"

    echo "ARGS: ${ZAP_ARGS[@]}"

    if [[ "${ZAP_ARGS[@]}" == *"/all-clusters-app.zap"* ]]; then
        ZCL_FILE="$CHIP_ROOT/src/app/zap-templates/zcl/zcl-with-test-extensions.json"
    else
        ZCL_FILE="$CHIP_ROOT/src/app/zap-templates/zcl/zcl.json"
    fi

    node src-script/zap-start.js --logToStdout \
        --gen "$CHIP_ROOT/src/app/zap-templates/app-templates.json" \
        --zcl "$ZCL_FILE" \
        "${ZAP_ARGS[@]}"
)
