#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Test script for GN GitHub workflow.

CHIP_ROOT="$(dirname "$0")/../.."

source "$CHIP_ROOT/scripts/activate.sh"

dmalloc=$(gn --root="$CHIP_ROOT" args "$CHIP_ROOT/out/$BUILD_TYPE" --short --list=chip_config_memory_debug_dmalloc)
if [ "x$dmalloc" == "xchip_config_memory_debug_dmalloc = true" ]; then
    eval "$(dmalloc -b -l DMALLOC_LOG -i 1 high)"
    export G_SLICE
    G_SLICE=always-malloc # required for dmalloc compatibility
fi

env

set -x

ninja -v -C "$CHIP_ROOT/out/$BUILD_TYPE" -k 0 check
