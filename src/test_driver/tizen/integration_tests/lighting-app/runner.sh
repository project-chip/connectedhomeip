#!/bin/sh

#
#    Copyright (c) 2021 Project CHIP Authors
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

set -e

# Allow unlimited core dump size
ulimit -c unlimited

# Print CHIP logs on stdout
dlogutil CHIP &

# Set the correct path for .gcda files
export GCOV_PREFIX=/mnt/chip
export GCOV_PREFIX_STRIP=5

# Create dump directory (may not exist if emu hasn't created it yet)
mkdir -p /mnt/chip/dump

# Save original core_pattern and set to raw core dumps (no crash-manager)
ORIGINAL_CORE_PATTERN=$(cat /proc/sys/kernel/core_pattern 2>/dev/null || echo "")
if [ -n "$ORIGINAL_CORE_PATTERN" ]; then
    echo "/mnt/chip/dump/core.%e.%p.%t" >/proc/sys/kernel/core_pattern
    echo "Core pattern set to: /mnt/chip/dump/core.%e.%p.%t (raw dumps, no crash-manager)"
fi

# Restore original core_pattern on exit
trap 'if [ -n "$ORIGINAL_CORE_PATTERN" ]; then echo "$ORIGINAL_CORE_PATTERN" > /proc/sys/kernel/core_pattern 2>/dev/null; fi' EXIT

# Install lighting Matter app
pkgcmd -i -t tpk -p /mnt/chip/org.tizen.matter.*/out/org.tizen.matter.*.tpk
# Launch lighting Matter app
app_launcher -s org.tizen.matter.example.lighting

# Helper function to run test and handle crashes
run_test_with_crash_handling() {
    TEST_NAME="$1"
    shift

    echo "RUN: $TEST_NAME"
    RV=0
    "$@" || RV=$?

    if [ "$RV" -ne 0 ]; then
        echo "DONE: FAIL (exit code: $RV)"

        # Raw core dumps are created instantly in /mnt/chip/dump/
        # No need to wait - they're already on shared filesystem
        if [ "$RV" -gt 128 ]; then
            echo "Raw core dump created: /mnt/chip/dump/core.$TEST_NAME.*"
        fi
    else
        echo "DONE: SUCCESS"
    fi

    return "$RV"
}

# TEST: pair app using network commissioning
run_test_with_crash_handling "pairing" /mnt/chip/chip-tool pairing onnetwork 1 20202021
# TEST: turn on light
run_test_with_crash_handling "on" /mnt/chip/chip-tool onoff on 1 1
# TEST: turn off light
run_test_with_crash_handling "off" /mnt/chip/chip-tool onoff off 1 1
