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

declare -i iterations=20
declare -i background_pid=0

# read shell arguments
# - a for application
# - i for number of iterations you want to have
while getopts a:i: flag; do
    case "$flag" in
        a) application=$OPTARG ;;
        i) iterations=$OPTARG ;;
    esac
done

echo "Running tests for application: $application, with iterations set to: $iterations"

if [[ $application == "tv" ]]; then
    declare -a test_array="($(find src/app/tests/suites -type f -name "TV_*.yaml" -exec basename {} .yaml \;))"
    cp examples/tv-app/linux/include/endpoint-configuration/chip_tv_config.ini /tmp/chip_tv_config.ini
# in case there's no application argument
# always default to all-cluters app
else
    application="all-clusters"
    declare -a test_array="($(find src/app/tests/suites -type f -name "Test*.yaml" -exec basename {} .yaml \;))"
fi

cleanup() {
    if [[ $background_pid != 0 ]]; then
        # In case we died on a failure before we cleaned up our background task.
        kill -9 "$background_pid" || true
    fi
}
trap cleanup EXIT

if [[ $iterations == 0 ]]; then
    echo "Invalid iteration count: '$1'"
    exit 1
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
        # This part is a little complicated.  We want to
        # 1) Start chip-app in the background
        # 2) Pipe its output through tee so we can wait until it's ready for a
        #    PASE handshake.
        # 3) Save its pid off so we can kill it.
        #
        # The subshell with echoing of $! to a file descriptor and
        # then reading things out of there accomplishes item 3;
        # otherwise $! would be the last-started command which would
        # be the tee.  This part comes from https://stackoverflow.com/a/3786955
        # and better ideas are welcome.
        #
        # The stdbuf -o0 is to make sure our output is flushed through
        # tee expeditiously; otherwise it will buffer things up and we
        # will never see the string we want.

        # Clear out our temp files so we don't accidentally do a stale
        # read from them before we write to them.
        rm -rf /tmp/"$application"-log
        touch /tmp/"$application"-log
        rm -rf /tmp/pid
        (
            stdbuf -o0 out/debug/standalone/chip-"$application"-app &
            echo $! >&3
        ) 3>/tmp/pid | tee /tmp/"$application"-log &
        while ! grep -q "Server Listening" /tmp/"$application"-log; do
            :
        done
        # Now read $background_pid from /tmp/pid; presumably it's
        # landed there by now.  If we try to read it immediately after
        # kicking off the subshell, sometimes we try to do it before
        # the data is there yet.
        background_pid="$(</tmp/pid)"
        echo "          * Pairing to device"
        out/debug/standalone/chip-tool pairing onnetwork 0 20202021 3840 ::1 11097
        echo "          * Starting test run: $i"
        out/debug/standalone/chip-tool tests "$i"
        # Prevent cleanup trying to kill a process we already killed.
        temp_background_pid=$background_pid
        background_pid=0
        kill -9 "$temp_background_pid"
        echo "  ===== Test complete: $i"
    done
    echo " ===== Iteration $j completed"
done
