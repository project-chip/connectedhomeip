# Copyright (c) 2009-2020 Arm Limited
# SPDX-License-Identifier: Apache-2.0
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

import pytest
from common.utils import validIPAddress, is_network_visible

import logging
log = logging.getLogger(__name__)

DEVICE_STATUS_LABEL = ['WIFI status', 'Connection status', 'MAC', 'IP', 'Netmask', 'Gateway', 'RSSI']
WIFI_STATION_MODE_OPTIONS = {'NotSupported' : None, 'ApplicationControlled' : 'app_ctrl', 'Disabled' : 'disable', 'Enabled' : 'enable', 'Unknown' : None}

def get_status_value(status_list, label):
    for param in status_list:
        if label in param:
            return param.split(':')[1]
    return None

@pytest.mark.smoketest
def test_device_status_check(device):
    ret = device.send(command="device status", expected_output="Done")
    assert ret != None and len(ret) > len(DEVICE_STATUS_LABEL) + 1

    ret.pop(0)

    #Check if response contains all labels
    for line, label in zip(ret, DEVICE_STATUS_LABEL):
        assert label in line

@pytest.mark.networktest
def test_device_sta_check(device):
    # Check sta mode
    ret = device.send(command="device sta mode", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip() in WIFI_STATION_MODE_OPTIONS

    save_mode = ret[-2].rstrip()

    # Check sta set mode
    ret = device.send(command="device sta set_mode dummy", expected_output="Invalid argument")
    assert ret != None

    for mode, set_mode in WIFI_STATION_MODE_OPTIONS.items():
        if set_mode != None:
            ret = device.send(command="device sta set_mode {}".format(set_mode), expected_output="Done")
            assert ret != None
            ret = device.send(command="device sta mode", expected_output="Done")
            assert ret != None and len(ret) > 1
            new_mode = ret[-2].rstrip()
            assert new_mode == mode

    # Restore default mode
    ret = device.send(command="device sta set_mode {}".format(WIFI_STATION_MODE_OPTIONS[save_mode]), expected_output="Done")
    assert ret != None
    ret = device.send(command="device sta mode", expected_output="Done")
    assert ret != None and len(ret) > 1
    new_mode = ret[-2].rstrip()
    assert new_mode == save_mode

    ret = device.send(command="device sta enabled", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip() in ("true", "false")

    ret = device.send(command="device sta provisioned", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip() in ("true", "false")

    ret = device.send(command="device sta controlled", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip() in ("true", "false")

    ret = device.send(command="device sta connected", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip() in ("true", "false")

@pytest.mark.networktest
def test_device_connection_check(device, network):
    network_ssid = network[0]
    network_pass = network[1]

    # Check if device support STA mode
    ret = device.send(command="device sta mode", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert "NotSupported" != ret[-2].rstrip()

    # Check if network is visible
    ret = device.send(command="device scan", expected_output="Done", wait_before_read=5)
    assert ret != None and len(ret) > 1
    assert is_network_visible(ret, network_ssid)

    # Check connection status
    ret = device.send(command="device sta connected", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().lower() == "false"

    # Check provisioned status
    ret = device.send(command="device sta provisioned", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().lower() == "false"

    # Connect to network
    ret = device.send(command="device connect {} {}".format(network_ssid, network_pass), expected_output="Done", wait_before_read=5)
    assert ret != None

    ret = device.wait_for_output("Internet connectivity ESTABLISHED")
    assert ret != None

    # Check connection status
    ret = device.send(command="device sta connected", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().lower() == "true"

    # Check provisioned status
    ret = device.send(command="device sta provisioned", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().lower() == "true"

    # Network disconnect
    ret = device.send(command="device sta clear_provision", expected_output="Done", wait_before_read=5)
    assert ret != None

    ret = device.wait_for_output("Internet connectivity LOST")
    assert ret != None

    # Check connection status
    ret = device.send(command="device sta connected", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().lower() == "false"

    # Check provisioned status
    ret = device.send(command="device sta provisioned", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().lower() == "false"


pytest.mark.networktest
def test_device_internet_connection_check(device, network):
    network_ssid = network[0]
    network_pass = network[1]

    # Check if device support STA mode
    ret = device.send(command="device sta mode", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert "NotSupported" != ret[-2].rstrip()

    # Check if network is visible
    ret = device.send(command="device scan", expected_output="Done", wait_before_read=5)
    assert ret != None and len(ret) > 1
    assert is_network_visible(ret, network_ssid)

    # Check connection status
    ret = device.send(command="device sta connected", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().lower() == "false"

    # Connect to network
    ret = device.send(command="device connect {} {}".format(network_ssid, network_pass), expected_output="Done", wait_before_read=5)
    assert ret != None

    ret = device.wait_for_output("Internet connectivity ESTABLISHED")
    assert ret != None

    # Check connection status
    ret = device.send(command="device sta connected", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().lower() == "true"

    # Check internet connection
    ret = device.send(command="device status", expected_output="Done")
    assert ret != None and len(ret) > len(DEVICE_STATUS_LABEL) + 1
    ret.pop(0)

    status = get_status_value(ret, 'Connection status')
    assert status != None and "UP" in status

    ip_address = "".join(get_status_value(ret, 'IP').split())
    assert ip_address != None 
    assert validIPAddress(ip_address) != "Invalid"
    assert ip_address != "0.0.0.0"

    # Network disconnect
    ret = device.send(command="device sta clear_provision", expected_output="Done", wait_before_read=5)
    assert ret != None

    ret = device.wait_for_output("Internet connectivity LOST")
    assert ret != None

    # Check connection status
    ret = device.send(command="device sta connected", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().lower() == "false"