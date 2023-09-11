#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
