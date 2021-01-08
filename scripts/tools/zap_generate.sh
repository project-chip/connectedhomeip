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

# Retrieve absolute path of zap file
OUTPUT_DIRECTORY=$(cd "$(dirname "$1")" && pwd)/gen/
FILE_PATH=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")

# Check if input file exist
if [ ! -f "$FILE_PATH" ]; then
    echo "ZAP file doesn't exist."
    exit 1
fi

# Check if output dir exist, if not create it
if [ ! -d "$OUTPUT_DIRECTORY" ]; then
    mkdir -p "$OUTPUT_DIRECTORY"
fi

ZAP_TEMPLATES=$PWD/src/app/zap-templates
cd ./third_party/zap/repo/
node ./src-script/zap-generate.js -z "$ZAP_TEMPLATES"/zcl/zcl.json -g "$ZAP_TEMPLATES"/app-templates.json -i "$FILE_PATH" -o "$OUTPUT_DIRECTORY"

# Check if clang-format is available
if command -v clang-format &>/dev/null; then
    clang-format -i "$OUTPUT_DIRECTORY"*
fi
