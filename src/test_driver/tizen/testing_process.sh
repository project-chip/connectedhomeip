#!/usr/bin/env bash

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

# This script is only for running inside qemu machine with Tizen image
# The script contains a set of tasks that perform integration tests

APP_DIR=/mnt/chip
CHECK_ALL=false

# Function check status after execute
function check_status() {
    echo "CHIP test $1 status: $2"
    if [[ $2 != 0 ]] && ! "$CHECK_ALL"; then
        echo o >/proc/sysrq-trigger
    fi
}

# Install application Matter on Tizen
pkgcmd -i -t tpk -p /mnt/chip/org.tizen.matter.*
check_status "install Matter app" $?

# Start debug CHIP
dlogutil CHIP &

# Run application
app_launcher -s org.tizen.matter.example.lighting
check_status "run application chip" $?

# Testing
# -----------------------------------------------------------------------
# For correct automatic verification of tests, complete the checking
# part in the run_qemu_test.py script
# -----------------------------------------------------------------------
# Pair app
"$APP_DIR"/chip-tool pairing onnetwork 1 20202021
check_status "pair app" $?

# Test on cluster
"$APP_DIR"/chip-tool onoff on 1 1
check_status "on cluster" $?

# Test off cluster
"$APP_DIR"/chip-tool onoff off 1 1
check_status "off cluster" $?

# Fast Shutdown emulated system
echo o >/proc/sysrq-trigger
