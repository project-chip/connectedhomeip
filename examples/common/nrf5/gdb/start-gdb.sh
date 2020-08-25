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

SCRIPT_ROOT=$(dirname "$0")
GNU_INSTALL_ROOT=${GNU_INSTALL_ROOT:-${HOME}/tools/arm/gcc-arm-none-eabi-7-2018-q2-update/bin}
GDB=$GNU_INSTALL_ROOT/arm-none-eabi-gdb
DEFAULT_APP=./build/chip-nrf52840-lock-example.out
STARTUP_CMDS=$SCRIPT_ROOT/gdb-startup-cmds.txt

if [[ $# -eq 0 ]]; then
    APP=$DEFAULT_APP
else
    APP=$1
fi

EXTRA_STARTUP_CMDS=()
if [[ -n $OPENTHREAD_ROOT ]]; then
    EXTRA_STARTUP_CMDS+=(
        "set substitute-path /build/KNGP-TOLL1-JOB1/openthread/examples/.. $OPENTHREAD_ROOT"
    )
fi

exec "$GDB" -q -x "$STARTUP_CMDS" "${EXTRA_STARTUP_CMDS[@]/#/-ex=}" "$APP"
