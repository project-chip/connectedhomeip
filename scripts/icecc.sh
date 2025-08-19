#!/usr/bin/env bash
#
# Copyright (c) 2024 Project CHIP Authors
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

# This scripts starts or stops the iceccd.
# It's meant to be use within devcontainer.

if [ "$1" = "start" ]; then
    IFS='.' read -ra PARTS <<<"$HOSTNAME"
    if iceccd -d -m 0 -N "devcontainer-${PARTS[0]}"; then
        echo "iceccd started"
    else
        echo "Failed to start iceccd"
    fi
fi

if [ "$1" = "stop" ]; then
    if pkill icecc; then
        echo "iceccd stopped"
    else
        echo "Failed to stop iceccd"
    fi
fi
