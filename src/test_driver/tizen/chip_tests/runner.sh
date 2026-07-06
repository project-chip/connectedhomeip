#!/bin/bash

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

# Override the default value to account for slower execution on QEMU.
export CHIP_TEST_EVENT_LOOP_HANDLER_MAX_DURATION_MS=1000

# Set the correct path for .gcda files
export GCOV_PREFIX=/mnt/chip
export GCOV_PREFIX_STRIP=5

# Create dump directory (may not exist if emu hasn't created it yet)
mkdir -p /mnt/chip/dump

# Restore the core_pattern configuration file on exit
if [ -f /proc/sys/kernel/core_pattern ]; then
    ORIGINAL_CORE_PATTERN=$(cat /proc/sys/kernel/core_pattern 2>/dev/null)
    trap 'echo "$ORIGINAL_CORE_PATTERN" > /proc/sys/kernel/core_pattern 2>/dev/null' EXIT
else
    trap 'rm -f /proc/sys/kernel/core_pattern 2>/dev/null' EXIT
fi

# Try to apply the custom pattern needed for the test execution
echo "/mnt/chip/dump/core.%e.%p.%t" > /proc/sys/kernel/core_pattern 2>/dev/null

FAILED=()
STATUS=0

# Run all executables in the /mnt/chip directory except the runner.sh script
while IFS= read -r TEST; do

    NAME=$(basename "$TEST")

    echo
    echo "RUN: $NAME"

    RV=0
    "$TEST" || RV=$?

    if [ "$RV" -eq 0 ]; then
        echo -e "DONE: \e[32mSUCCESS\e[0m"
    else
        FAILED+=("$NAME")
        STATUS=$((STATUS + 1))
        echo -e "DONE: \e[31mFAIL\e[0m (exit code: $RV)"

        # Raw core dumps are created instantly in /mnt/chip/dump/
        # No need to wait or copy - they're already on shared filesystem
        if [ "$RV" -gt 128 ]; then
            echo "Raw core dump created: /mnt/chip/dump/core.$NAME.*"
        fi
    fi

done < <(find /mnt/chip/tests -type f -executable ! -name runner.sh)

if [ ! "$STATUS" -eq 0 ]; then
    echo
    echo "### FAILED: ${FAILED[*]}"
fi

exit "$STATUS"
