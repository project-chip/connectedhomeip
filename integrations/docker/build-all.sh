#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

#
# temporary wrapper build script until we can build something better
#  https://github.com/project-chip/connectedhomeip/issues/710
#

set -e

SUB_PATH="*"
ALL_ARGS=""

for i in "$@"; do
    case $i in
    -p=* | --path=*)
        SUB_PATH="${i#*=}"
        ;;
    *)
        ALL_ARGS="$ALL_ARGS ${i#*=}"
        ;;
    esac
done

echo "SUB_PATH: $SUB_PATH"

function build_image() {
    PARSE_PATH=$1
    ARGS_TO_PASS=$2

    echo "PARSE_PATH: $PARSE_PATH"
    echo "ARGS_TO_PASS: $ARGS_TO_PASS"

    find "$(git rev-parse --show-toplevel)"/integrations/docker/images/$PARSE_PATH -name Dockerfile ! -path "*chip-cert-bins/*" | while read -r dockerfile; do
        echo "$(dirname "$dockerfile")"
        pushd "$(dirname "$dockerfile")" >/dev/null
        ./build.sh "$ARGS_TO_PASS"
        popd >/dev/null
    done
}

build_image "$SUB_PATH" "$ALL_ARGS"
