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
#      This is a utility script that compiles ESP32 QEMU and sets it up
#      for testing.
#

here=$(cd "$(dirname "$0")" && pwd)

set -e

die() {
    echo "${me:?}: *** ERROR: " "${*}"
    exit 1
}

# move to the example folder, I don't work anywhere else
cd "$here" || die 'ack!, where am I?!?'

# shellcheck source=/dev/null
source idf.sh
rm -f ./build/sdkconfig
SDKCONFIG=./build/sdkconfig SDKCONFIG_DEFAULTS=sdkconfig_qemu.defaults idf make defconfig
SDKCONFIG=./build/sdkconfig idf make esp32_elf_builder
