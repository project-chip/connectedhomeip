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

JLINK_GDB_SERVER=JLinkGDBServer
DEVICE_TYPE=NRF52840_XXAA
RTT_PORT=19021

command -v "$JLINK_GDB_SERVER" || {
    echo "ERROR: $JLINK_GDB_SERVER not found"
    echo "Please install SEGGER J-Link software package"
    exit 1
}

command -v nc || {
    echo "ERROR: nc command not found"
    exit 1
}

# Launch JLink GDB Server in background; redirect output thru sed to add prefix.
"$JLINK_GDB_SERVER" -device "$DEVICE_TYPE" -if SWD -speed 4000 -rtos GDBServer/RTOSPlugin_FreeRTOS "$@" > >(exec sed -e 's/^/JLinkGDBServer: /') 2>&1 &
GDB_SERVER_PID=$!

# Repeatedly open a connection to the GDB server's RTT port until
# the user kills the GDB server with an interrupt character.
while true; do
    # Wait for GDB server to begin listening on RTT port
    while ! lsof -nP -i4TCP:"$RTT_PORT" -sTCP:LISTEN >/dev/null; do
        # Quit if the GDB server exits.
        if ! kill -0 "$GDB_SERVER_PID" >/dev/null 2>&1; then
            echo ""
            exit
        fi

        # Wait a bit.
        sleep 0.1

    done

    # Connect to RTT port.
    nc localhost "$RTT_PORT"

done
