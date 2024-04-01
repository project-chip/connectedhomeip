#!/bin/bash

#
# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Function to display information for each network interface
display_interface_info() {
    interface=$1
    echo "Interface: $interface"

    # Check if interface is up
    if ip link show "$interface" | grep -q 'state UP'; then
        echo "  Status: UP"
    else
        echo "  Status: DOWN"
    fi

    # Get and display the IPv4 address
    ipv4_address=$(ip -4 addr show "$interface" | grep -oP '(?<=inet\s)\d+(\.\d+){3}')
    [ -z "$ipv4_address" ] && ipv4_address="N/A"
    echo "  IPv4 Address: $ipv4_address"

    # Get and display the IPv6 address
    ipv6_address=$(ip -6 addr show "$interface" | grep -oP '(?<=inet6\s)[a-f0-9:]+')
    [ -z "$ipv6_address" ] && ipv6_address="N/A"
    echo "  IPv6 Address: $ipv6_address"

    # Get and display the subnet mask
    subnet_mask=$(ifconfig "$interface" | grep -oP '(?<=Mask:)[0-9.]+')
    [ -z "$subnet_mask" ] && subnet_mask="N/A"
    echo "  Subnet Mask: $subnet_mask"

    # Get and display the MAC address
    mac_address=$(ip link show "$interface" | grep -oP '(?<=ether\s)[a-f0-9:]+')
    [ -z "$mac_address" ] && mac_address="N/A"
    echo "  MAC Address: $mac_address"
}

# Get a list of all network interfaces
interfaces=$(ip link show | grep -oP '(?<=^\d: )[e-w]+[0-9a-zA-Z-]+')

# Iterate over each interface and display relevant information
for intf in "$interfaces"; do
    display_interface_info "$intf"
    echo ""
done

# Get and display the default gateway
default_gateway=$(ip route | grep default | awk '{print $3}')
[ -z "$default_gateway" ] && default_gateway="N/A"
echo "Default Gateway: $default_gateway"

# Get and display the DNS server information
mapfile -t dns_servers < <(grep nameserver /etc/resolv.conf | awk '{print $2}')
if [ ${#dns_servers[@]} -eq 0 ]; then
    echo "DNS Servers: N/A"
else
    echo "DNS Servers: ${dns_servers[*]}"
fi
echo

# Check if Internet is available
echo "Checking Internet availability..."
if ping -c 1 8.8.8.8 &>/dev/null; then
    echo "Internet is available"
else
    echo "Internet is not available"
fi
