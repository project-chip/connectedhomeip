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
import bluetooth

import logging
log = logging.getLogger(__name__)

DEVICE_BLE_NAME = "CHIP-3840"

def is_ble_device_visible(device_name):
    nearby_devices = bluetooth.discover_devices(duration=20, lookup_names=True,
                                            flush_cache=True, lookup_class=False)

    log.info("Found {} devices".format(len(nearby_devices)))

    for addr, name in nearby_devices:
        try:
            log.info("Device:   {} - {}".format(addr, name))
        except UnicodeEncodeError:
            log.info("Device:   {} - {}".format(addr, name.encode("utf-8", "replace")))
        if name == device_name:
            return True

    return False

@pytest.mark.bletest
def test_btp_adv_check(device):
    # Enable advertisement
    ret = device.send(command="btp adv start", expected_output="Done")
    assert ret != None

    # Check if device is visible
    assert is_ble_device_visible(DEVICE_BLE_NAME) == True

    # Disable advertisement
    ret = device.send(command="btp adv stop", expected_output="Done")
    assert ret != None

    # Check if device is not visible
    assert is_ble_device_visible(DEVICE_BLE_NAME) == False