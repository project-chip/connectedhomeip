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

USER="$(id -u -n)"
TAP_TUN_INTERFACE_NAME="ARMfmtap"
BRIDGE_INTERFACE_NAME="ARMfmbr"
NETWORK_INTERFACE="eth0"
USER_PREFIX=""
DHCP_ENABLE=false

if [ "$EUID" -ne 0 ]; then
    USER_PREFIX="sudo"
fi

function show_usage {
    cat <<EOF
Usage: $0 [options] command

Enable, disable or restart Open IoT SDK networking environment.

Options:
    -h,--help                           Show this help
    -d,--dhcp                           Run dhcp client on bridge interface
    -b,--bridge  <bridge_name>          Network bridge name <bridge_name - default is ARMfmbr>
    -t,--tap     <tap_name>             Network TAP device name <tap_name - default is ARMfmtap>
    -u,--user    <user_name>            Network user <user_name - default is current user>
    -n,--network <network_interface>    Network/parent interface name <network_interface - default is eth0>

command:
    up
    down
    restart

EOF
}

function net_up {
    echo "Create ${TAP_TUN_INTERFACE_NAME} TAP device for ${USER} user"
    ${USER_PREFIX} ip tuntap add dev $TAP_TUN_INTERFACE_NAME mode tap user $USER
    ${USER_PREFIX} ifconfig $TAP_TUN_INTERFACE_NAME 0.0.0.0 promisc

    echo "Create ${BRIDGE_INTERFACE_NAME} bridge interface between ${NETWORK_INTERFACE} and ${TAP_TUN_INTERFACE_NAME}"
    ${USER_PREFIX} ip link add $BRIDGE_INTERFACE_NAME type bridge
    ${USER_PREFIX} ip link set $TAP_TUN_INTERFACE_NAME master $BRIDGE_INTERFACE_NAME
    ${USER_PREFIX} ip address flush dev $NETWORK_INTERFACE
    ${USER_PREFIX} ip link set $NETWORK_INTERFACE master $BRIDGE_INTERFACE_NAME
    ${USER_PREFIX} ip link set dev $BRIDGE_INTERFACE_NAME up
    if $DHCP_ENABLE; then
        ${USER_PREFIX} dhclient -v $BRIDGE_INTERFACE_NAME
    fi
    ifconfig
}

function net_down {
    ${USER_PREFIX} ip link set $NETWORK_INTERFACE nomaster
    echo "Delete ${BRIDGE_INTERFACE_NAME} bridge and and ${TAP_TUN_INTERFACE_NAME} TAP device"
    ${USER_PREFIX} ip link set dev $TAP_TUN_INTERFACE_NAME down
    ${USER_PREFIX} ip link set dev $BRIDGE_INTERFACE_NAME down
    ${USER_PREFIX} ip link delete $BRIDGE_INTERFACE_NAME type bridge
    ${USER_PREFIX} ip tuntap del dev $TAP_TUN_INTERFACE_NAME mode tap
    if $DHCP_ENABLE; then
        ${USER_PREFIX} dhclient -v $NETWORK_INTERFACE
    fi
    ifconfig
}

SHORT=d,b:,t:,u:,n:,h
LONG=dhcp,bridge:,tap:,user:,network:,help
OPTS=$(getopt -n build --options $SHORT --longoptions $LONG -- "$@")

eval set -- "$OPTS"

while :; do
    case "$1" in
    -h | --help)
        show_usage
        exit 0
        ;;
    -d | --dhcp)
        DHCP_ENABLE=true
        shift
        ;;
    -b | --bridge)
        BRIDGE_INTERFACE_NAME=$2
        shift 2
        ;;
    -t | --tap)
        TAP_TUN_INTERFACE_NAME=$2
        shift 2
        ;;
    -u | --user)
        USER=$2
        shift 2
        ;;
    -n | --network)
        NETWORK_INTERFACE=$2
        shift 2
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

if [[ "$COMMAND" == *"down"* || "$COMMAND" == *"restart"* ]]; then
    net_down
fi

if [[ "$COMMAND" == *"up"* || "$COMMAND" == *"restart"* ]]; then
    net_up
fi
