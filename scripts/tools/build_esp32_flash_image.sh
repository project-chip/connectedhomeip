#!/usr/bin/env bash
#
#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
