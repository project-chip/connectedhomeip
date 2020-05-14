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
#      This is a utility script that runs CHIP tests using ESP32 QEMU.
#

here=$(cd "$(dirname "$0")" && pwd)
chip_dir="$here/../../.."

set -e

die() {
    echo "$me: *** ERROR: " "${*}"
    exit 1
}

# move to the example folder, I don't work anywhere else
cd "$here" || die 'ack!, where am I?!?'

source idf.sh
SDKCONFIG_DEFAULTS=sdkconfig_qemu.defaults idf make defconfig
idf make

flash_image_file=$(mktemp)
trap "{ rm -f $flash_image_file; }" EXIT

"$chip_dir"/scripts/tools/build_esp32_flash_image.sh ./build/chip-crypto-tests.bin "$flash_image_file"
"$chip_dir"/scripts/tools/esp32_qemu_run.sh "$flash_image_file" | tee results.log

# If the logs contain failure message
if grep -F "] : FAILED" results.log; then
    die 'Some tests failed. Check results.log'
fi

# If the logs do not contain final success status
if grep -F "CHIP-tests: CHIP test status: 0" results.log; then
    echo "$me: All tests passed"
else
    die 'Tests did not run to completion. Check results.log'
fi
