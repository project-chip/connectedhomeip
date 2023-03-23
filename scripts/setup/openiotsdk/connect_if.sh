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

# Enable/disable/restart TAP/TUN Open IoT SDK networking environment.

HOST_BRIDGE="ARMhbr"
DEFAULT_ROUTE_IF=""
USER="$(id -u -n)"
INTERFACES=()

declare -A default_if_info

if [ "$EUID" -ne 0 ]; then
    echo "Run a script with root permissions"
    exit 1
fi

function show_usage() {
    cat <<EOF
Usage: $0 command net_if <net_if> ...

Connect specific network interfaces with the default route interface. Create a bridge and link all interfaces to it.
Keep the default route of network traffic.

EOF
}

function get_default_if_info() {
    default_if_info[ip]="$(ifconfig "$DEFAULT_ROUTE_IF" | grep -w inet | awk '{print $2}' | cut -d ":" -f 2)"
    default_if_info[netmask]="$(ifconfig "$DEFAULT_ROUTE_IF" | grep -w inet | awk '{print $4}' | cut -d ":" -f 2)"
    default_if_info[broadcast]="$(ifconfig "$DEFAULT_ROUTE_IF" | grep -w inet | awk '{print $6}' | cut -d ":" -f 2)"
    default_if_info[gateway]="$(ip route show 0.0.0.0/0 dev "$DEFAULT_ROUTE_IF" | cut -d\  -f3)"
}

function connect_with_host() {
    ip link add name "$HOST_BRIDGE" type bridge
    ip link set "$DEFAULT_ROUTE_IF" master "$HOST_BRIDGE"
    ip addr flush dev "$DEFAULT_ROUTE_IF"
    for interface in "${INTERFACES[@]}"; do
        ip link set "$interface" master "$HOST_BRIDGE"
        ip addr flush dev "$interface"
    done
    ifconfig "$HOST_BRIDGE" "${default_if_info[ip]}" netmask "${default_if_info[netmask]}" broadcast "${default_if_info[broadcast]}"
    route add default gw "${default_if_info[gateway]}" "$HOST_BRIDGE"
}

if [[ $# -lt 1 ]]; then
    show_usage >&2
    exit 1
fi

INTERFACES=("$*")
DEFAULT_ROUTE_IF=$(route | grep '^default' | grep -o '[^ ]*$')
echo "Default route interface $DEFAULT_ROUTE_IF"
get_default_if_info
echo "Connect $INTERFACES to $DEFAULT_ROUTE_IF via bridge"
connect_with_host
