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

from chip import ChipDeviceCtrl
from chip.ChipBluezMgr import BluezManager as BleManager

import logging
log = logging.getLogger(__name__)

def scan_chip_ble_devices():
    devices = []
    bleMgr = BleManager(devMgr=ChipDeviceCtrl.ChipDeviceController())
    bleMgr.scan("-t 5")

    for device in bleMgr.peripheral_list:
        devIdInfo = bleMgr.get_peripheral_devIdInfo(device)
        if devIdInfo:
            log.info("Found CHIP device {}".format(device.Name))
            devices.append(device)

    return devices

@pytest.mark.smoketest
def test_btp_device_visiable(device):
    # Check if device is visible
    assert len(scan_chip_ble_devices()) != 0

@pytest.mark.bletest
def test_btp_adv_check(device):
    # Enable advertisement
    ret = device.send(command="btp adv start", expected_output="Done")
    assert ret != None

    # Check if device is visible
    assert len(scan_chip_ble_devices()) != 0

    # Disable advertisement
    ret = device.send(command="btp adv stop", expected_output="Done")
    assert ret != None

    # Check if device is not visible
    assert len(scan_chip_ble_devices()) == 0