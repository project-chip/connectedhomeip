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

#
#    Description:
#      This script can be used to build an image file containing multiple
#      source binaries at specific offset in the image
#

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"

# Activate Matter environment
source "$DIR/../activate.sh"

# Set RPC console directory
RPC_CONSOLE_DIR="$DIR/../../examples/common/pigweed/rpc_console"
OUTPUT_DIR=rpc_console_out

for i in "$@"; do
    case $i in
    -o=* | --output=*)
        OUTPUT_DIR="${i#*=}"
        shift
        ;;
    *)
        # unknown option
        ;;
    esac
done

gn gen --check --fail-on-unused-args --root="$RPC_CONSOLE_DIR" "$OUTPUT_DIR"
ninja -C "$OUTPUT_DIR"
