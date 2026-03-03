#!/usr/bin/env bash

#
# Copyright (c) 2020 Project CHIP Authors
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

if [[ "$OSTYPE" == "darwin"* ]] && [[ "$GITHUB_ACTIONS" == "true" ]]; then
    # Use sudo to avoid Local Network Privacy restrictions on macOS 15+.
    # macOS 15 introduced Local Network Privacy which restricts multicast traffic in several ways.
    # Running from ssh, running from Terminal.app, or accepting a UI prompt can allow access, but those
    # aren't great options in CI.
    # Processes run as root are exempt from these restrictions.
    # For details, see https://developer.apple.com/documentation/technotes/tn3179-understanding-local-network-privacy#macOS-considerations
    sudo ninja -v -C "$CHIP_ROOT/out/$BUILD_TYPE" -k 0 check
else
    ninja -v -C "$CHIP_ROOT/out/$BUILD_TYPE" -k 0 check
fi
