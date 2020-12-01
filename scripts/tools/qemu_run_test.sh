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
set -x

die() {
    echo "${me:?}: *** ERROR: " "${*}"
    exit 1
}

SRC_DIR="$(dirname "$0")/../.."
BUILD_DIR="$1"
shift
QEMU_TEST_TARGET="$1"
shift
EXTRA_COMPILE_ARGUMENTS="$*" # generally -lFooHelperLibrary

# shellcheck source=/dev/null
source "$BUILD_DIR"/env.sh
bash "$BUILD_DIR"/esp32_elf_builder.sh "$BUILD_DIR/lib/$QEMU_TEST_TARGET" "$EXTRA_COMPILE_ARGUMENTS"

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
