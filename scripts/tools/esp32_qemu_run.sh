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
#      This is a utility script that runs ESP32 QEMU using the given
#      application image.
#

usage() {
    exitcode=0
    if [[ -n "$1" ]]; then
        exitcode=1
        echo "*** Error: $*"
    fi
    echo "Usage: $0 <flash image file>"
    exit "$exitcode"
}

me=$(basename "$0")
die() {
    echo "$me: *** ERROR: " "${*}"
    exit 1
}

realpath() {
    path=$1 # input

    [[ -z $path ]] && return 0

    # trim trailing slashes
    while [[ ${#path} -gt 1 && $path = */ ]]; do
        path=${path%/}
    done

    # if we're at root we're done
    if [[ $path = / ]]; then
        echo "$path"
        return 0
    fi

    [[ $path != /* ]] && path=$PWD/$path

    if [[ -d $path ]]; then
        (cd "$path" && pwd)
    else
        echo "$(realpath "${path%/*}")/${path##*/}"
    fi
}

[[ $# -eq 1 ]] || usage "Incorrect number of arguments"

[[ -n $QEMU_ESP32 ]] || die "Environment variable QEMU_ESP32 is undefined."

flash_image=$(realpath "$1")

[[ -r $flash_image ]] || usage "Could not read file $flash_image"

"$QEMU_ESP32" -nographic -machine esp32 -drive file="$flash_image",if=mtd,format=raw -no-reboot
