#!/bin/bash

#
#    Copyright (c) 2025 Project CHIP Authors
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

# For each target
for target in out/tizen-*; do
    # For each coredump
    for zip in "$target/dump"/*.zip; do
        basepath=$(dirname "$zip")
        filename=$(basename "$zip")
        binary="${filename%%_*}"
        path="$basepath/${filename%.*}"
        coredump="$path/${filename%.*}.coredump"

        unzip "$zip" -d "$basepath"
        tar -xf "$path"/*.tar -C "$path"

        echo "----------------------------------------------------------------------------------------------------"
        gdb-multiarch --batch \
            -ex "set auto-load safe-basepath /" \
            -ex "set sysroot $TIZEN_SDK_SYSROOT" \
            -ex "thread apply all bt full" \
            "$target/$binary" "$coredump"
    done
done
