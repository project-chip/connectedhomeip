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

sudo bash $1/scripts/setup/raspberry_pi/start_wlan_ap.sh start --interface $AP_NAME --ap_gateway $AP_GATEWAY --ap_ssid $AP_SSID --ap_pswd $AP_PASSWORD
sudo bash $1/scripts/setup/raspberry_pi/start_echo_server.sh

cd $HOME

# Build chip tools for functional testing
mkdir -p FunctionalTests
export FUNCTIONAL_TESTS_DIR=${HOME}/FunctionalTests

cd $1
# Build CHIP main
./scripts/build/default.sh
export CHIP_TOOLS_DIR=$1/out/default
cd $HOME

# Install Python Chip Device Controller
virtualenv FunctionalTests/.venv
source FunctionalTests/.venv/bin/activate
pip install $CHIP_TOOLS_DIR/controller/python/chip*.whl
pip install -r $1/src/test_driver/mbed-functional/requirements.txt
deactivate
