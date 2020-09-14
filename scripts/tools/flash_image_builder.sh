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

usage() {
    exitcode=0
    if [[ -n "$1" ]]; then
        exitcode=1
        echo "Error: $*"
    fi
    echo "Usage: $0 <image-size> <output-file> <offset1=input-file1> [<offset2=input-file2> ..]"
    exit "$exitcode"
}

[[ $# -ge 3 ]] || usage "Incorrect number of arguments"

image_size="$1"
image_size=$((image_size))
[[ $? -eq 0 ]] || usage "Image size ($1) must be a number"
[[ $image_size -gt 0 ]] || usage "Image size ($1) must be a valid number and greater than 0"

# Create an empty <image_size> file containing all 0xff
dd if=/dev/zero bs="$image_size" count=1 | tr '\000' '\377' >"$2"

written=0

# Argument 3 onwards have input files and corresponding offsets
for item in "${@:3}"; do
    IFS='=' read -r start file <<<"$item"

    offset=$((start))
    [[ $? -eq 0 ]] || usage "Offset ($start) must be a number"

    [[ -r $file ]] || usage "Cannot read file $file"
    [[ $written -le $offset ]] || usage "Writing $file at $offset will overwrite previous segment"

    read -r _perms _ _user _group filesize _rest < <(ls -l "$file")
    ((written += filesize))
    [[ $written -lt $image_size ]] || usage "Writing $file at $offset will overflow image"

    dd if="$file" of="$2" conv=notrunc bs="$offset" seek=1
done
