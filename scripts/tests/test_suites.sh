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

find_result=$(find src/app/tests/suites -type f -name "*.yaml" -exec basename {} \; | awk -F '.yaml' '{print $1}' | sed -e ':a' -e 'N' -e '$!ba' -e 's/\n/;/g')
test_array=($(echo $find_result | tr ";" "\n"))

echo "Found tests:"
for i in "${test_array[@]}"; do
    echo "   $i"
done
echo ""
echo ""

for i in "${test_array[@]}"; do
    echo "  ===== Running test: $i"
    echo "          * Starting cluster server"
    rm -rf /tmp/chip_tool_config.ini
    sleep 1
    out/debug/chip-all-clusters-app &
    background_pid=$!
    sleep 1
    echo "          * Pairing to device"
    out/debug/standalone/chip-tool pairing onnetwork 1 34567890 3840 127.0.0.1 11097
    sleep 1
    echo "          * Starting test run: $i"
    out/debug/standalone/chip-tool tests "$i"
    kill -9 $background_pid || true
    echo "  ===== Test complete: $i"
    sleep 2
done
