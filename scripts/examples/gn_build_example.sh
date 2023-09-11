#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Build script for GN examples GitHub workflow.

source "$(dirname "$0")/../../scripts/activate.sh"

GN_ARGS=()

EXAMPLE_DIR=$1
shift
OUTPUT_DIR=$1
shift

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

set -x
env

gn gen --check --fail-on-unused-args --root="$EXAMPLE_DIR" "$OUTPUT_DIR" --args="${GN_ARGS[*]}"

ninja -C "$OUTPUT_DIR" "${NINJA_ARGS[@]}"
