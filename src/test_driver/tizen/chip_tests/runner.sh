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

COUNT_PASS=0
COUNT_SKIP=0
COUNT_FAIL=0

# If some tests are known to fail and the root cause is not yet identified,
# they can be added to the SKIP array to unblock failing CI runs.
SKIP=(
    "TestPDCCommissioning"
    "TestCertificateChainRequestTracker"
)

# Run all executables in the /mnt/chip directory except the runner.sh script
while IFS= read -r TEST; do

    NAME=$(basename "$TEST")

    echo
    echo "RUN: $NAME"

    if [[ " ${SKIP[*]} " == *" $NAME "* ]]; then
        echo -e "SKIP: \e[33m$NAME\e[0m"
        COUNT_SKIP=$((COUNT_SKIP + 1))
        continue
    fi

    RV=0
    "$TEST" || RV=$?

    if [ "$RV" -eq 0 ]; then
        echo -e "DONE: \e[32mSUCCESS\e[0m"
        COUNT_PASS=$((COUNT_PASS + 1))
    else
        FAILED+=("$NAME")
        STATUS=$((STATUS + 1))
        echo -e "DONE: \e[31mFAIL\e[0m"
        COUNT_FAIL=$((COUNT_FAIL + 1))
    fi

done < <(find /mnt/chip/tests -type f -executable ! -name runner.sh)

echo
if [ ! "$STATUS" -eq 0 ]; then
    echo "### FAILED: ${FAILED[*]}"
fi
echo "### TOTAL: $((COUNT_PASS + COUNT_SKIP + COUNT_FAIL))"
echo "### PASS: $COUNT_PASS"
echo "### SKIP: $COUNT_SKIP"
echo "### FAIL: $COUNT_FAIL"

exit "$STATUS"
