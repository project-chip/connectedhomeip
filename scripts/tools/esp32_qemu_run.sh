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
  echo "Usage: $0 <device firmware directory> <flash image file>"
  exit "$exitcode"
}

me=$(basename "$0")
die() {
  echo "$me: *** ERROR: " "${*}"
  exit 1
}

[[ $# -eq 2 ]] || usage "Incorrect number of arguments"

[[ -n $QEMU_ESP32 ]] || die "Environment variable QEMU_ESP32 is undefined."

firmware_path=$(realpath "$1")
flash_image=$(realpath "$2")

[[ -r "${firmware_path}/rom.bin" ]] || usage "Could not read file $firmware_path/rom.bin"
[[ -r "${firmware_path}/rom1.bin" ]] || usage "Could not read file $firmware_path/rom1.bin"
[[ -r $flash_image ]] || usage "Could not read file $flash_image"

tempdir=$(mktemp -d)
echo "Created $tempdir"

trap "{ rm -rf $tempdir; }" EXIT

ln -s "$firmware_path/rom.bin" "$tempdir/rom.bin"
ln -s "$firmware_path/rom1.bin" "$tempdir/rom1.bin"
ln -s "$flash_image" "$tempdir/esp32flash.bin"

cd "$tempdir" || die 'ack!, where am I?!?'

"$QEMU_ESP32" -d guest_errors,unimp -cpu esp32 -M esp32 -m 4M -s >io.txt
