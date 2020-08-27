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
#      This is scripts builds ESP32 QEMU, and runs CHIP unit tests using it.
#

here=$(cd "$(dirname "$0")" && pwd)
chip_dir="$here"/../..

# shellcheck source=/dev/null
source "$chip_dir"/src/test_driver/esp32/idf.sh
"$chip_dir"/src/test_driver/esp32/qemu_setup.sh

if [ $? -ne 0 ]; then
    echo "Setup failure"
    exit 1
fi

# Currently only crypto, inet, and system tests are configured to run on QEMU.
# The specific qualifiers will be removed, once all CHIP unit tests are
# updated to run on QEMU.
idf make V=1 -C "$chip_dir"/src/test_driver/esp32/build/chip/src/crypto check
idf make V=1 -C "$chip_dir"/src/test_driver/esp32/build/chip/src/inet check
idf make V=1 -C "$chip_dir"/src/test_driver/esp32/build/chip/src/system check
idf make V=1 -C "$chip_dir"/src/test_driver/esp32/build/chip/src/transport check
