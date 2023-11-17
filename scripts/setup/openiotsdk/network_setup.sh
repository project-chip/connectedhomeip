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

# Enable/disable/restart Open IoT SDK networking environment.

NAMESPACE_NAME="ns"
HOST_SIDE_IF_NAME="hveth"
NAMESPACE_SIDE_IF_NAME="nveth"
TAP_TUN_INTERFACE_NAME="tap"
BRIDGE_INTERFACE_NAME="br"
HOST_IPV6_ADDR="fe00::1"
NAMESPACE_IPV6_ADDR="fe00::2"
HOST_IPV4_ADDR="10.200.1.1"
NAMESPACE_IPV4_ADDR="10.200.1.2"
NAME="ARM"
INTERNET_ENABLE=false
USER="$(id -u -n)"

if [ "$EUID" -ne 0 ]; then
    echo "Run a script with root permissions"
    exit 1
fi

function show_usage() {
    cat <<EOF
Usage: $0 [options] command

Enable, disable or restart Open IoT SDK networking environment.

Options:
    -h,--help                           Show this help
    -n,--name    <base_name>            Open IoT SDK network base name <base_name - default is ARM>
    -u,--user    <user_name>            Network user <user_name - default is current user>
    -I,--Internet                       Add Internet connection support to network namespace <disabled by default>

command:
    up
    down
    restart

EOF
}

function net_ns_up() {
    # Enable IPv6 and IP-forwarding
    sysctl net.ipv6.conf.all.disable_ipv6=0 net.ipv4.conf.all.forwarding=1 net.ipv6.conf.all.forwarding=1

    echo "Create $NAMESPACE_NAME network namespace"
    # Create namespace.
    ip netns add "$NAMESPACE_NAME"

    # Enable lo interface in namespace
    ip netns exec "$NAMESPACE_NAME" ip link set dev lo up

    echo "Adding $HOST_SIDE_IF_NAME veth with peer $NAMESPACE_SIDE_IF_NAME"
    # Create two virtual interfaces and link them - one on host side, one on namespace side.
    ip link add "$HOST_SIDE_IF_NAME" type veth peer name "$NAMESPACE_SIDE_IF_NAME"

    # Give the host a known IPv6 addr and set the host side up
    echo "Set IP addresses $HOST_IPV4_ADDR/24 $HOST_IPV6_ADDR/64 to $HOST_SIDE_IF_NAME interface"
    ip addr add "$HOST_IPV4_ADDR"/24 dev "$HOST_SIDE_IF_NAME"
    ip -6 addr add "$HOST_IPV6_ADDR"/64 dev "$HOST_SIDE_IF_NAME"
    ip link set "$HOST_SIDE_IF_NAME" up

    echo "Adding $NAMESPACE_SIDE_IF_NAME veth to namespace $NAMESPACE_NAME"
    # Associate namespace IF with the namespace
    ip link set "$NAMESPACE_SIDE_IF_NAME" netns "$NAMESPACE_NAME"
    ip netns exec "$NAMESPACE_NAME" ip link set dev "$NAMESPACE_SIDE_IF_NAME" up

    echo "Create $TAP_TUN_INTERFACE_NAME TAP device"
    ip netns exec "$NAMESPACE_NAME" ip tuntap add dev "$TAP_TUN_INTERFACE_NAME" mode tap user "$USER"
    ip netns exec "$NAMESPACE_NAME" ifconfig "$TAP_TUN_INTERFACE_NAME" 0.0.0.0 promisc

    echo "Create $BRIDGE_INTERFACE_NAME bridge interface between $NAMESPACE_SIDE_IF_NAME and $TAP_TUN_INTERFACE_NAME"
    ip netns exec "$NAMESPACE_NAME" ip link add "$BRIDGE_INTERFACE_NAME" type bridge
    echo "Set IP addresses $NAMESPACE_IPV4_ADDR/24 $NAMESPACE_IPV6_ADDR/64 to $BRIDGE_INTERFACE_NAME bridge interface"
    ip netns exec "$NAMESPACE_NAME" ip -6 addr add "$NAMESPACE_IPV6_ADDR"/64 dev "$BRIDGE_INTERFACE_NAME"
    ip netns exec "$NAMESPACE_NAME" ip addr add "$NAMESPACE_IPV4_ADDR"/24 dev "$BRIDGE_INTERFACE_NAME"
    ip netns exec "$NAMESPACE_NAME" ip addr flush dev "$NAMESPACE_SIDE_IF_NAME"
    ip netns exec "$NAMESPACE_NAME" ip link set "$TAP_TUN_INTERFACE_NAME" master "$BRIDGE_INTERFACE_NAME"
    ip netns exec "$NAMESPACE_NAME" ip link set "$NAMESPACE_SIDE_IF_NAME" master "$BRIDGE_INTERFACE_NAME"
    ip netns exec "$NAMESPACE_NAME" ip link set dev "$BRIDGE_INTERFACE_NAME" up

    ip netns exec "$NAMESPACE_NAME" ip route add default via "$HOST_IPV4_ADDR"

    if "$INTERNET_ENABLE"; then
        echo "Set Internet connection to $NAMESPACE_NAME namespace"
        DEFAULT_ROUTE=$(route | grep '^default' | grep -o '[^ ]*$')
        echo "Default route interface $DEFAULT_ROUTE"
        # Enable masquerading of namespace IP address
        iptables -t nat -A POSTROUTING -s "$NAMESPACE_IPV4_ADDR"/24 -o "$DEFAULT_ROUTE" -j MASQUERADE

        iptables -A FORWARD -i "$DEFAULT_ROUTE" -o "$HOST_SIDE_IF_NAME" -j ACCEPT
        iptables -A FORWARD -o "$DEFAULT_ROUTE" -i "$HOST_SIDE_IF_NAME" -j ACCEPT
    fi

    echo "$NAMESPACE_NAME namespace configuration"
    ip netns exec "$NAMESPACE_NAME" ifconfig
    echo "Host configuration"
    ifconfig
}

function net_ns_down() {
    ip netns delete "$NAMESPACE_NAME"
    ip link delete dev "$HOST_SIDE_IF_NAME"
    echo "Host configuration"
    ifconfig
}

SHORT=n:,u:,I,h,
LONG=name:,user:,Internet,help
OPTS=$(getopt -n build --options "$SHORT" --longoptions "$LONG" -- "$@")

eval set -- "$OPTS"

while :; do
    case "$1" in
        -h | --help)
            show_usage
            exit 0
            ;;
        -n | --name)
            NAME=$2
            shift 2
            ;;
        -u | --user)
            USER=$2
            shift 2
            ;;
        -I | --Internet)
            INTERNET_ENABLE=true
            shift
            ;;
        -* | --*)
            shift
            break
            ;;
        *)
            echo "Unexpected option: $1"
            show_usage
            exit 2
            ;;
    esac
done

if [[ $# -lt 1 ]]; then
    show_usage >&2
    exit 1
fi

case "$1" in
    up | down | restart)
        COMMAND=$1
        ;;
    *)
        echo "ERROR: Command $COMMAND not supported"
        show_usage
        exit 1
        ;;
esac

NAMESPACE_NAME="$NAME$NAMESPACE_NAME"
HOST_SIDE_IF_NAME="$NAME$HOST_SIDE_IF_NAME"
NAMESPACE_SIDE_IF_NAME="$NAME$NAMESPACE_SIDE_IF_NAME"
TAP_TUN_INTERFACE_NAME="$NAME$TAP_TUN_INTERFACE_NAME"
BRIDGE_INTERFACE_NAME="$NAME$BRIDGE_INTERFACE_NAME"

if [[ "$COMMAND" == *"down"* || "$COMMAND" == *"restart"* ]]; then
    net_ns_down
fi

if [[ "$COMMAND" == *"up"* || "$COMMAND" == *"restart"* ]]; then
    net_ns_up
fi
