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
import subprocess

from chip.setup_payload import SetupPayload
from chip import exceptions

from common.utils import scan_chip_ble_devices, run_wifi_provisioning

import logging
log = logging.getLogger(__name__)

DEVICE_NODE_ID=1234

@pytest.mark.deviceControllerTest
def test_lock_ctrl_dev_ctrl(device, network, device_controller):
    network_ssid = network[0]
    network_pass = network[1]
    
    ret = device.wait_for_output("SetupQRCode")
    assert ret != None and len(ret) > 1

    qr_code = ret[-1].split('[', 1)[1].split(']')[0]
    device_details = dict(SetupPayload().ParseQrCode("VP:vendorpayload%{}".format(qr_code)).attributes)
    assert device_details != None and len(device_details) != 0

    ret = run_wifi_provisioning(device_controller, network_ssid, network_pass, int(device_details["Discriminator"]), int(device_details["SetUpPINCode"]), DEVICE_NODE_ID)
    assert ret == DEVICE_NODE_ID

    device.flush(5)

    args = {}
    
    try:
        # Check on command
        err, res = device_controller.ZCLSend("OnOff", "On", DEVICE_NODE_ID, 1, 0, args, blocking=True)
        assert err == 0

        ret = device.wait_for_output("Lock Action has been completed")
        assert ret != None and len(ret) > 1

        # Check off command
        err, res = device_controller.ZCLSend("OnOff", "Off", DEVICE_NODE_ID, 1, 0, args, blocking=True)
        assert err == 0

        ret = device.wait_for_output("Unlock Action has been completed")
        assert ret != None and len(ret) > 1

        # Check toggle command
        err, res = device_controller.ZCLSend("OnOff", "Toggle", DEVICE_NODE_ID, 1, 0, args, blocking=True)
        assert err == 0

        ret = device.wait_for_output("Lock Action has been completed")
        assert ret != None and len(ret) > 1

    except exceptions.ChipStackException as ex:
        log.error("Exception occurred during process ZCL command: {}".format(str(ex)))
        assert False
    except Exception as ex:
        log.error("Exception occurred during processing input: {}".format(str(ex)))
        assert False

@pytest.mark.chipToolTest
def test_lock_ctrl_chip_tool(device, network, chip_tools_dir):
    network_ssid = network[0]
    network_pass = network[1]
    
    ret = device.wait_for_output("SetupQRCode")
    assert ret != None and len(ret) > 1

    qr_code = ret[-1].split('[', 1)[1].split(']')[0]
    device_details = dict(SetupPayload().ParseQrCode("VP:vendorpayload%{}".format(qr_code)).attributes)
    assert device_details != None and len(device_details) != 0

    process = subprocess.Popen(["./chip-tool", "pairing", "ble", network_ssid, network_pass, str(device_details["SetUpPINCode"]), str(device_details["Discriminator"])], cwd=chip_tools_dir, stdout=subprocess.PIPE, universal_newlines=True)
    out = process.stdout.read()
    process.wait()
    assert process.returncode == 0
    device_connected = "ConnectDevice complete" in out
    assert device_connected
    device_provisioned = "Network Provisioning Success" in out
    assert device_provisioned

    process = subprocess.Popen(["./chip-tool", "onoff", "on", "1"], cwd=chip_tools_dir, stdout=subprocess.PIPE, universal_newlines=True)
    out = process.stdout.read()
    process.wait()
    assert process.returncode == 0
    assert "SendMessage returned No Error" in out and "Data model processing success" in out

    ret = device.wait_for_output("Lock Action has been completed")
    assert ret != None and len(ret) > 1

    process = subprocess.Popen(["./chip-tool", "onoff", "off", "1"], cwd=chip_tools_dir, stdout=subprocess.PIPE, universal_newlines=True)
    out = process.stdout.read()
    process.wait()
    assert process.returncode == 0
    assert "SendMessage returned No Error" in out and "Data model processing success" in out

    ret = device.wait_for_output("Unlock Action has been completed")
    assert ret != None and len(ret) > 1

    process = subprocess.Popen(["./chip-tool", "onoff", "toggle", "1"], cwd=chip_tools_dir, stdout=subprocess.PIPE, universal_newlines=True)
    out = process.stdout.read()
    process.wait()
    assert process.returncode == 0
    assert "SendMessage returned No Error" in out and "Data model processing success" in out

    ret = device.wait_for_output("Lock Action has been completed")
    assert ret != None and len(ret) > 1
