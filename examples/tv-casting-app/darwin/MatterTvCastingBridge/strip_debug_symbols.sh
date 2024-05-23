#!/usr/bin/env bash

#
#    Copyright (c) 2023 Project CHIP Authors
#    All rights reserved.
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

# This script runs as a part of the Post-Actions of the MatterTvCastingBridge build scheme
#

set -ex

if [ "$CONFIGURATION" == "Debug" ]; then
    echo "Skipping stripping debug symbols since we are using the '$CONFIGURATION' configuration"
    exit 0
fi

files_to_strip=("libTvCastingCommon.a")

for lib_path in "${LIBRARY_SEARCH_PATHS[@]}"; do
    for lib_name in "$(ls "$lib_path")"; do
        if [[ ${files_to_strip[*]} =~ ${lib_name} ]]; then
            echo "Stripping file $lib_path/$lib_name"
            strip -S -x -r "$lib_path/$lib_name"
        fi
    done
done
