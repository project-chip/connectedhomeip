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

set -e

die() {
    echo "${me:?}: *** ERROR: " "${*}"
    exit 1
}

BUILD_DIR="${abs_top_builddir:?}"
SRC_DIR="${abs_top_srcdir:?}"

# shellcheck source=/dev/null
source "$BUILD_DIR"/env.sh
bash "$BUILD_DIR"/esp32_elf_builder.sh "$QEMU_TEST_TARGET"

flash_image_file=$(mktemp)
log_file=$(mktemp)
trap '{ rm -f $flash_image_file $log_file; }' EXIT

"$SRC_DIR"/scripts/tools/build_esp32_flash_image.sh "$BUILD_DIR"/chip-tests.bin "$flash_image_file"
"$SRC_DIR"/scripts/tools/esp32_qemu_run.sh "$flash_image_file" | tee "$log_file"

# If the logs contain failure message
if grep -F "] : FAILED" "$log_file"; then
    die 'Some tests failed.'
fi

# If the logs do not contain final success status
if grep -F "CHIP-tests: CHIP test status: 0" "$log_file"; then
    echo "$me: All tests passed"
else
    die 'Tests did not run to completion.'
fi
