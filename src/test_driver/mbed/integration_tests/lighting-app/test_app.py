# Copyright (c) 2009-2021 Arm Limited
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

from chip.setup_payload import SetupPayload
from chip import exceptions

from common.utils import scan_chip_ble_devices, run_wifi_provisioning

import logging
log = logging.getLogger(__name__)

@pytest.mark.smoketest
def test_smoke_test(device):
    device.reset(duration=1)
    ret = device.wait_for_output("Mbed lighting-app example application start")
    assert ret != None and len(ret) > 0
    ret = device.wait_for_output("Mbed lighting-app example application run")
    assert ret != None and len(ret) > 0


def test_wifi_provisioning(device, network, device_controller):
    network_ssid = network[0]
    network_pass = network[1]
    
    ret = device.wait_for_output("SetupQRCode")
    assert ret != None and len(ret) > 1

    qr_code = ret[-1].split('[', 1)[1].split(']')[0]
    device_details = dict(SetupPayload().ParseQrCode("VP:vendorpayload%{}".format(qr_code)).attributes)
    assert device_details != None and len(device_details) != 0

    ble_chip_device = scan_chip_ble_devices(device_controller)
    assert ble_chip_device != None and len(ble_chip_device) != 0

    chip_device_found = False

    for ble_device in ble_chip_device:
        if (int(ble_device.discriminator) == int(device_details["Discriminator"]) and
            int(ble_device.vendorId) == int(device_details["VendorID"]) and
            int(ble_device.productId) == int(device_details["ProductID"])):
            chip_device_found = True
            break

    assert chip_device_found

    ret = run_wifi_provisioning(device_controller, network_ssid, network_pass, int(device_details["Discriminator"]), int(device_details["SetUpPINCode"]), DEVICE_NODE_ID)
    assert ret != None and ret == DEVICE_NODE_ID