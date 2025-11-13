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

# Print CHIP logs on stdout
dlogutil CHIP &

# Override the default value to account for slower execution on QEMU.
export CHIP_TEST_EVENT_LOOP_HANDLER_MAX_DURATION_MS=1000

# Set the correct path for .gcda files
export GCOV_PREFIX=/mnt/chip
export GCOV_PREFIX_STRIP=5

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
        echo -e "DONE: \e[31mFAIL\e[0m"
    fi

done < <(find /mnt/chip/tests -type f -executable ! -name runner.sh)

if [ ! "$STATUS" -eq 0 ]; then
    echo
    echo "### FAILED: ${FAILED[*]}"
fi

exit "$STATUS"
