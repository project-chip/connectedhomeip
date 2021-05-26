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

if [ $# -eq 0 ]; then
    echo "No arguments. Please enter CHIP source path"
    exit 1
fi

AP_NAME=WIFI_AP
export AP_SSID=CHIPnet
export AP_PASSWORD=CHIPnet123
export AP_GATEWAY=192.168.4.1
export ECHO_SERVER_PORT=7

sudo apt-get install -fy \
    network-manager \
    net-tools \
    openbsd-inetd \
    python3-virtualenv &&
    true

if ! nmcli con show | grep -q "$AP_NAME"; then
    # Run access point
    echo "Run access point"

    sudo nmcli con add type wifi ifname wlan0 mode ap con-name "$AP_NAME" autoconnect true ssid "$AP_SSID"
    sudo nmcli con modify "$AP_NAME" 802-11-wireless.band bg 802-11-wireless.channel 7
    sudo nmcli con modify "$AP_NAME" 802-11-wireless-security.proto rsn 802-11-wireless-security.group ccmp 802-11-wireless-security.pairwise ccmp 802-11-wireless-security.key-mgmt wpa-psk 802-11-wireless-security.psk "$AP_PASSWORD"
    sudo nmcli con modify "$AP_NAME" ipv4.method shared
    sudo nmcli con modify "$AP_NAME" ipv4.addr "$AP_GATEWAY"/24
    sudo nmcli con up "$AP_NAME"

    sudo nmcli -f GENERAL.STATE con show "$AP_NAME"
fi

if ! netstat -a | less | grep -q "echo"; then
    #Setting TCP and UDP echo server
    echo "Setting TCP and UDP echo server"

    echo 'echo            stream  tcp     nowait  root    internal' | sudo tee -a /etc/inetd.conf
    echo 'echo            dgram   udp     wait    root    internal' | sudo tee -a /etc/inetd.conf
    sudo service openbsd-inetd restart

    netstat -a | less | grep "echo"
fi

cd $HOME

# Build chip tools for functional testing
mkdir -p FunctionalTests
export FUNCTIONAL_TESTS_DIR=${HOME}/FunctionalTests

cd $1
# Build CHIP main
./scripts/build/default.sh
export CHIP_TOOLS_DIR=${HOME}/CHIP/out/default
cd $HOME

# Install Python Chip Device Controller
virtualenv FunctionalTests/.venv
source FunctionalTests/.venv/bin/activate
pip install $CHIP_TOOLS_DIR/controller/python/chip*.whl
pip install -r $HOME/CHIP/src/test_driver/mbed-functional/requirements.txt
deactivate
