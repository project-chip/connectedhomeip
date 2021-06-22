#!/usr/bin/env bash
#
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
#
#    Description:
#      This is a utility script that can be used by developers to do
#    simulate a device and controller on the same linux machine. This
#    script is not intended to be used in a testing framework as-is
#    because it requires root access to set up network namespaces.
#
#    To use this script, compile the required device example, and
#    run inside the namespace using
#    sudo <path>/linux_ip_namespace_setup.sh -r <path_to_app>
#
#    The controller can then be started in a new terminal and will
#    be able to communicate with the application as if it were on
#    a separate network.
#

NAMESPACE="MatterTester"
HOST_SIDE_IF_NAME="heth0"
NAMESPACE_SIDE_IF_NAME="neth0"
BRIDGE_NAME="nbridge"
BRIDGE_ADDR="192.168.4.50"
NAMESPACE_ADDR="192.168.4.45"
HOST_IPV6_ADDR=fc00::1
BRIDGE_IPV6_ADDR=fc00::b
NAMESPACE_IPV6_ADDR=fc00::a

function run_setup() {
    # Create namespace.
    ip netns add "$NAMESPACE"

    # Create two virtual interfaces and link them - one on host side, one on namespace side.
    ip link add "$HOST_SIDE_IF_NAME" type veth peer name "$NAMESPACE_SIDE_IF_NAME"

    # Give the host a known IPv6 addr and set the host side up
    ip -6 addr add "$HOST_IPV6_ADDR"/64 dev "$HOST_SIDE_IF_NAME"
    ip link set "$HOST_SIDE_IF_NAME" up

    # Associate namespace IF with the namespace
    ip link set "$NAMESPACE_SIDE_IF_NAME" netns "$NAMESPACE"

    # Give the namespace IF an address (something nothing else is using) and set it up
    echo "Adding address for namespace IF"
    ip netns exec "$NAMESPACE" ip -6 addr add "$NAMESPACE_IPV6_ADDR"/64 dev "$NAMESPACE_SIDE_IF_NAME"
    ip netns exec "$NAMESPACE" ip link set dev "$NAMESPACE_SIDE_IF_NAME" up

    # Add a route to the namespace to go through the bridge
    echo "Setting routes for namespace"
    ip netns exec "$NAMESPACE" ip -6 route add default dev "$NAMESPACE_SIDE_IF_NAME"

    echo "Setup complete."
}

function run_add_ipv4() {
    # Give the namespace an IPv4 address
    ip netns exec "$NAMESPACE" ip addr add "$NAMESPACE_ADDR"/24 dev "$NAMESPACE_SIDE_IF_NAME"

    # Add a bridge, give it an address (something nothing else is using)
    echo "Setting up bridge"
    ip link add name "$BRIDGE_NAME" type bridge
    ip -6 addr add "$BRIDGE_IPV6_ADDR"/64 dev "$BRIDGE_NAME"
    ip addr add "$BRIDGE_ADDR"/24 brd + dev "$BRIDGE_NAME"

    # For ipv6 and ipv4 to work together, need the bridge to ignore the ipv6 packets (DROP here means don't bridge)
    ebtables-legacy -t broute -A BROUTING -p ipv6 -j DROP -i "$HOST_SIDE_IF_NAME"
    ip link set "$BRIDGE_NAME" up

    # Connect the host side to the bridge, so now we have bridge <-> host_side_if <-> namespace_if
    echo "Connecting host virtual IF to bridge"
    ip link set "$HOST_SIDE_IF_NAME" master "$BRIDGE_NAME"

    #ip netns exec ${NAMESPACE} ip route add default via ${BRIDGE_ADDR} dev ${NAMESPACE_SIDE_IF_NAME}
}

function run_cmd() {
    # Start the app in the namespace
    echo "Running $1 in namespace."
    ip netns exec "$NAMESPACE" "$1"
}

function run_cleanup() {
    # Deleting the namespace will remove the namespace and peer'd interfaces to
    have_ebtables_legacy=$1
    ip netns delete "$NAMESPACE"
    if ifconfig | grep "$BRIDGE_NAME"; then
        if [ "$have_ebtables_legacy" = true ]; then
            # Just try to drop the additional rule - it references our interface
            # so if it's there, we added it.
            ebtables-legacy -t broute -D BROUTING -p ipv6 -j DROP -i "$HOST_SIDE_IF_NAME" >/dev/null
        fi
        ip link delete dev "$BRIDGE_NAME" type bridge
    fi
}

function help() {
    echo "Usage: $file_name [ options ... ]"
    echo ""

    echo "This script is used to set up linux namespaces for Matter device testing"
    echo "between a controller and device on the same linux machine."
    echo ""
    echo "To use this script, run the device code in a namespace using the -r command"
    echo "and a controller in a seperate terminal to simulate two devices communicating"
    echo "across a network."
    echo "Example:"
    echo "--------"
    echo "Terminal 1:"
    echo "sudo <path>/$file_name -r <path>/<application_name>"
    echo ""
    echo "Terminal 2:"
    echo "<path>/chip-device-ctrl"
    echo ""
    echo "This script requires sudo for setup and requires access to ebtables-legacy"
    echo "to set up dual ipv4/ipv6 namespaces. Defaults to ipv6 only."
    echo ""

    echo "Options:
  -h, --help                Display this information.
  -s, --setup               Setup an IP namespace. Will run cleanup if namespace exists.
  -4, --ipv4                Add ipv4 support.
  -r, --run filename        Run file in the namespace. Will setup namespace if required.
  -c, --cleanup             Delete namespace and routes
"
}

declare setup=false
declare filename=""
declare run=false
declare cleanup=false
declare ipv4=false

file_name=${0##*/}

while (($#)); do
    case $1 in
        --help | -h)
            help
            exit 1
            ;;
        --setup | -s)
            setup=true
            ;;
        --run | -r)
            run=true
            filename=$2
            shift
            ;;
        --cleanup | -c)
            cleanup=true
            ;;
        --ipv4 | -4)
            ipv4=true
            ;;
        -*)
            help
            echo "Unknown Option \"$1\""
            exit 1
            ;;
    esac
    shift
done

if [[ $EUID -ne 0 ]]; then
    echo "You must run this script with superuser privileges."
    exit 1
fi

if ifconfig | grep "$HOST_SIDE_IF_NAME"; then
    issetup=true
else
    issetup=false
fi

if [ "$setup" = false ] && [ "$run" = false ] && [ "$cleanup" = false ]; then
    echo "Must specify one or more of -s, -r, -c."
    exit 1
fi

if command -v ebtables-legacy >/dev/null; then
    have_ebtables_legacy=true
else
    have_ebtables_legacy=false
fi

if [ "$ipv4" = true ] && [ "$have_ebtables_legacy" = false ]; then
    echo "To set up namespaces with ipv4/ipv6 connectivity, ebtables-legacy"
    echo "is required. For example, to install on machines using APT:"
    echo "sudo apt-get install ebtables"
    exit 1
fi

if [ "$run" = true ]; then
    if [ "$issetup" = false ]; then
        setup=true
    fi
fi

if [ "$setup" = true ]; then
    if [ "$issetup" = true ]; then
        cleanup=true
    fi
fi

if [ "$cleanup" = true ]; then
    run_cleanup "$have_ebtables_legacy"
fi

if [ "$setup" = true ]; then
    run_setup
    if [ "$ipv4" = true ]; then
        run_add_ipv4
    fi
fi

if [ "$run" = true ]; then
    run_cmd "$filename"
fi
