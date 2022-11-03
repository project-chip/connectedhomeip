#!/bin/bash

#
# SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

function _get_fullpath() {
    cd "$(dirname "$1")" && echo "$PWD/$(basename "$1")"
}

function _usage() {
    cat <<EOF
Invalid arguments passed.
Usage:
    zap_bootstrap.sh -> install and update required packages
    zap_bootstrap.sh -c -> run a clean bootstrap, install packages from scratch
EOF
    exit 1
}

set -e

SCRIPT_PATH="$(_get_fullpath "$0")"
CHIP_ROOT="${SCRIPT_PATH%/scripts/tools/zap/zap_bootstrap.sh}"

(
    cd "$CHIP_ROOT" &&
        git submodule update --init third_party/zap/repo

    cd "third_party/zap/repo"

    if [ $# -eq 0 ]; then
        echo "Running ZAP bootstrap"
        if ! npm list installed-check &>/dev/null; then
            npm install installed-check
        fi

        if ! ./node_modules/.bin/installed-check -c &>/dev/null; then
            npm install
        fi
    elif [ $# -eq 1 ] && [ "$1" = "-c" ]; then
        echo "Running clean ZAP bootstrap"
        npm ci
        npm run version-stamp
        npm rebuild canvas --update-binary
        npm run build-spa
    else
        _usage
    fi
)

echo "ZAP bootstrap done!"
