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

# Disable Tizen memory resources daemon (resourced) to avoid tight memory limits on background processes/dumpers
systemctl stop resourced 2>/dev/null || true

# Ensure Tizen crash-manager is active and running cleanly
systemctl restart crash-manager 2>/dev/null || true

# Print CHIP logs on stdout
dlogutil CHIP &

# Override the default value to account for slower execution on QEMU.
export CHIP_TEST_EVENT_LOOP_HANDLER_MAX_DURATION_MS=1000

# Set the correct path for .gcda files
export GCOV_PREFIX=/mnt/chip
export GCOV_PREFIX_STRIP=5

FAILED=()
STATUS=0
CRASH_DUMP_DIR="/opt/usr/share/crash/dump"

FILTER=""
if [ -f "/mnt/chip/test_filter" ]; then
    FILTER=$(cat /mnt/chip/test_filter)
    echo "Using test filter: $FILTER"
fi

# Run all executables in the /mnt/chip directory except the runner.sh script
while IFS= read -r TEST; do

    NAME=$(basename "$TEST")

    if [ -n "$FILTER" ]; then
        if [[ ! "$NAME" == $FILTER ]]; then
            continue
        fi
    fi

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

        # Only collect crash dumps if the process actually crashed (exit code > 128 due to a signal)
        if [ "$RV" -gt 128 ]; then
            # Wait for the crash dump to be fully written and stabilized
            echo -n "Process crashed (exit code: $RV). Waiting for crash files to appear"
            PREV_SIZE=-1
            STABLE_COUNT=0
            REQUIRED_STABILITY=5  # Must remain the same size for 5 consecutive seconds
            for i in $(seq 1 60); do
                echo -n "."
                if [ -n "$(ls /mnt/chip/dump/*.zip 2>/dev/null)" ]; then
                    if [ "$PREV_SIZE" -eq -1 ]; then
                        echo ""
                        echo -n "Crash files found! Waiting for size to stabilize"
                    fi
                    # Use du to get total size of the zip files
                    CURR_SIZE=$(du -c /mnt/chip/dump/*.zip 2>/dev/null | tail -n 1 | cut -f1 || echo 0)
                    if [ "$CURR_SIZE" -eq "$PREV_SIZE" ]; then
                        STABLE_COUNT=$((STABLE_COUNT + 1))
                        if [ "$STABLE_COUNT" -ge "$REQUIRED_STABILITY" ]; then
                            echo ""
                            echo "Crash dump stabilized (size: $CURR_SIZE KB)."
                            break
                        fi
                    else
                        STABLE_COUNT=0
                    fi
                    PREV_SIZE=$CURR_SIZE
                else
                    STABLE_COUNT=0
                    PREV_SIZE=-1
                fi
                sleep 1
            done
            if [ "$STABLE_COUNT" -lt "$REQUIRED_STABILITY" ]; then
                echo ""
                echo "Timeout waiting for crash dump stabilization."
            fi

            CRASH_DIR="/mnt/chip/dump/$(date +%Y%m%d%H%M%S)_${NAME}"
            mkdir -p "$CRASH_DIR"

            # Move the stabilized .zip files from /mnt/chip/dump/ to $CRASH_DIR/
            if [ -n "$(ls /mnt/chip/dump/*.zip 2>/dev/null)" ]; then
                echo "Moving crash files to $CRASH_DIR"
                mv /mnt/chip/dump/*.zip "$CRASH_DIR/" 2>/dev/null || true

                # Unzip on the guest to read .so_info and copy libraries
                ZIP_FILE=$(find "$CRASH_DIR" -name "*.zip" 2>/dev/null | head -n 1)
                if [ -n "$ZIP_FILE" ]; then
                    EXTRACT_DIR="${ZIP_FILE%.zip}"
                    mkdir -p "$EXTRACT_DIR"
                    unzip -qo "$ZIP_FILE" -d "$EXTRACT_DIR"

                    SO_INFO_FILE=$(find "$EXTRACT_DIR" -name "*.so_info" 2>/dev/null | head -n 1)
                    if [ -f "$SO_INFO_FILE" ]; then
                        echo "Copying runtime shared libraries from guest to host..."
                        while IFS= read -r line; do
                            LIB_PATH=$(echo "$line" | cut -d' ' -f1)
                            if [ -f "$LIB_PATH" ]; then
                                LIB_DIR=$(dirname "$LIB_PATH")
                                mkdir -p "/mnt/chip/system_libs$LIB_DIR"
                                cp -P "$LIB_PATH"* "/mnt/chip/system_libs$LIB_PATH" 2>/dev/null || true
                            fi
                        done < "$SO_INFO_FILE"
                    fi
                fi
            else
                echo "No crash files found in /mnt/chip/dump/"
            fi

            ls -la "$CRASH_DIR" 2>&1
        else
            echo "Process failed without crashing (exit code: $RV). Skipping crash dump collection."
        fi
    fi

done < <(find /mnt/chip/tests -type f -executable ! -name runner.sh)

if [ ! "$STATUS" -eq 0 ]; then
    echo
    echo "### FAILED: ${FAILED[*]}"
fi

exit "$STATUS"
