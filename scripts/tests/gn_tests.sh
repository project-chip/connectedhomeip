#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Test script for GN GitHub workflow.

CHIP_ROOT="$(dirname "$0")/../.."

source "$CHIP_ROOT/scripts/activate.sh"

dmalloc=$(gn --root="$CHIP_ROOT" args "$CHIP_ROOT/out/$BUILD_TYPE" --short --list=chip_config_memory_debug_dmalloc)

case "$dmalloc" in
    "chip_config_memory_debug_dmalloc = true")
        eval "$(dmalloc -b -i 1 high)"
        export G_SLICE
        G_SLICE=always-malloc # required for dmalloc compatibility
        ;;
    "chip_config_memory_debug_dmalloc = false") ;;

    *)
        echo >&2 "Invalid output: \"$dmalloc\""
        exit 1
        ;;
esac

env

set -x

ninja -v -C "$CHIP_ROOT/out/$BUILD_TYPE" -k 0 check
