#!/usr/bin/env bash

#
# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

declare -a test_array="($(find src/app/tests/suites -type f -name "*.yaml" -exec basename {} .yaml \;))"
declare -i iterations=20
declare -i background_pid=0

cleanup() {
    if [[ $background_pid != 0 ]]; then
        # In case we died on a failure before we cleaned up our background task.
        kill -9 "$background_pid" || true
    fi
}
trap cleanup EXIT

if [[ -n $1 ]]; then
    iterations=$1
    if [[ $iterations == 0 ]]; then
        echo "Invalid iteration count: '$1'"
        exit 1
    fi
fi

echo "Found tests:"
for i in "${test_array[@]}"; do
    echo "   $i"
done
echo ""
echo ""

ulimit -c unlimited || true

declare -a iter_array="($(seq "$iterations"))"
for j in "${iter_array[@]}"; do
    echo " ===== Iteration $j starting"
    for i in "${test_array[@]}"; do
        echo "  ===== Running test: $i"
        echo "          * Starting cluster server"
        rm -rf /tmp/chip_tool_config.ini
        out/debug/chip-all-clusters-app &
        background_pid=$!
        echo "          * Pairing to device"
        out/debug/standalone/chip-tool pairing onnetwork 1 20202021 3840 ::1 11097
        echo "          * Starting test run: $i"
        out/debug/standalone/chip-tool tests "$i"
        # Prevent cleanup trying to kill a process we already killed.
        temp_background_pid=$background_pid
        background_pid=0
        kill -9 "$temp_background_pid" || true
        echo "  ===== Test complete: $i"
    done
    echo " ===== Iteration $j completed"
done
