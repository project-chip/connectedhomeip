#!/usr/bin/env bash
#
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
#      This is a utility script that builds flash image for ESP32 QEMU.
#

[[ $# -eq 2 ]] || {
    echo "Usage: $0 <app-image.bin> <output-flash-image.bin>"
    exit 1
}

here=$(cd "$(dirname "$0")" && pwd)
build_dir="$(dirname "$1")"
bootload_file="$build_dir"/bootloader.bin
partitions_file="$build_dir"/partitions.bin

"$here"/flash_image_builder.sh 0x400000 "$2" 0x1000="$bootload_file" 0x8000="$partitions_file" 0x10000="$1"
