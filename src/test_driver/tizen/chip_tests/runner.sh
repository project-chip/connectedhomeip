#!/bin/bash

#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Print CHIP logs on stdout
dlogutil CHIP &

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

done < <(find /mnt/chip -type f -executable ! -name runner.sh)

if [ ! "$STATUS" -eq 0 ]; then
    echo
    echo "### FAILED: ${FAILED[*]}"
fi

exit "$STATUS"
