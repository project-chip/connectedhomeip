#!/usr/bin/env bash

#
#    Copyright (c) 2020 Project CHIP Authors
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

# Build script for GN EFT32 examples GitHub workflow.

chip_root="$(dirname "$0")/../.."
source "$chip_root/scripts/activate.sh"

set -x
env

root="$1"
out="$2"
shift 2
if [ -n "$1" ]; then
    board="$1"
    shift
else
    board="$EFR32_BOARD"
fi

gn gen --check --fail-on-unused-args --root="$root" --args="efr32_board=\"$board\"" "$out/$board" "$@"
ninja -v -C "$out/$board"
