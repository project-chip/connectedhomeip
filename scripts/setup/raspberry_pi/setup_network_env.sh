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

# Installs required packages and configuration for Raspberry Pi to setup
# networking environment
#
# See docs/BUILDING.md for more details

AP_NAME=CHIPnet
AP_PASSWORD="CHIPnet123"

set -ex

sudo apt-get install -fy \
    network-manager \
    net-tools \
    openbsd-inetd \
    tcpdump \
    python3-virtualenv &&
    true

# Run access point
echo "Run access point"

sudo nmcli con add type wifi ifname wlan0 con-name WiFiAp autoconnect true ssid "$AP_NAME"
sudo nmcli con modify WiFiAp 802-11-wireless.mode ap 802-11-wireless.band bg ipv4.method shared
sudo nmcli con modify WiFiAp 802-11-wireless-security.proto rsn wifi-sec.key-mgmt wpa-psk wifi-sec.psk "$AP_PASSWORD"
sudo nmcli con up WiFiAp

sudo nmcli -f GENERAL.STATE con show WiFiAp

#Setting TCP and UDP echo server
echo "Setting TCP and UDP echo server"

echo 'echo            stream  tcp     nowait  root    internal' | sudo tee -a /etc/inetd.conf
echo 'echo            dgram   udp     wait    root    internal' | sudo tee -a /etc/inetd.conf
sudo service openbsd-inetd restart

netstat -a | less | grep "echo"

# Build chip tools for network testing
mkdir -p $HOME/NetTools

source scripts/bootstrap.sh

# Build CHIP main
scripts/build/default.sh

# Copy CLI tools to output directory (NetTools)
cp out/default/chip-echo-requester out/default/chip-echo-responder out/default/chip-tool $HOME/NetTools

# Install Python Chip Device Controller
virtualenv $HOME/NetTools/python_env
source $HOME/NetTools/python_env/bin/activate
pip install out/default/controller/python/chip*.whl
deactivate
