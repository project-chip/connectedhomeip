#!/bin/bash

#
#    Copyright (c) 2022 Project CHIP Authors
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

# This script executes the command given as an argument within
# CHIP_ROOT in a specific network namespace.

NETWORK_NAMESPACE="default"

function show_usage() {
    cat <<EOF
Usage: $0 namespace command

Execute the command given as an argument in a specific network namespace.

Example:

scripts/run_in_ns.sh MatterNet "ip a"

Use "default" as a namespace argument to use the host network namespace directly.

EOF
}

if [[ $# -lt 2 ]]; then
    show_usage >&2
    exit 1
fi

NETWORK_NAMESPACE=$1
shift

if [[ $NETWORK_NAMESPACE == "default" ]]; then
    "$@"
else
    if [ ! -f /var/run/netns/"$NETWORK_NAMESPACE" ]; then
        echo "$NETWORK_NAMESPACE network namespace does not exist"
        show_usage >&2
        exit 1
    fi
    echo "Run command: $@ in $NETWORK_NAMESPACE namespace"
    if [ "$EUID" -ne 0 ]; then
        sudo env PATH="$PATH" ip netns exec "$NETWORK_NAMESPACE" "$@"
    else
        ip netns exec "$NETWORK_NAMESPACE" "$@"
    fi
fi
