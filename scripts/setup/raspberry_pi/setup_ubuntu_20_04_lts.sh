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

# Installs required packages and configuration for Raspberry Pi to be able
# to build CHIP and run example/integration applications
#
# See docs/BUILDING.md for more details

set -ex

sudo apt-get install -fy \
    g++ \
    gcc \
    git \
    libavahi-client-dev \
    libcairo2-dev \
    libdbus-1-dev \
    libgirepository1.0-dev \
    libglib2.0-dev \
    libssl-dev \
    ninja-build \
    pi-bluetooth \
    pkg-config \
    protobuf-compiler \
    python \
    python3-dev \
    python3-venv \
    unzip &&
    true

sudo apt-get install -fy \
    rfkill \
    hostapd \
    iptables \
    isc-dhcp-server \
    iproute2 \
    iw \
    net-tools \
    iputils-ping &&
    true

sudo echo "" > /var/lib/dhcp/dhcpd.leases

sudo apt-get install -fy \
    net-tools \
    openbsd-inetd \
    python3-virtualenv &&
    true

echo "You must REBOOT after pi-bluetooth has been installed"
