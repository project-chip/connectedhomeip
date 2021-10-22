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


import sys
import os
import platform
import random

from chip import exceptions

if platform.system() == 'Darwin':
    from chip.ChipCoreBluetoothMgr import CoreBluetoothManager as BleManager
elif sys.platform.startswith('linux'):
    from chip.ChipBluezMgr import BluezManager as BleManager

import logging
log = logging.getLogger(__name__)


def scan_chip_ble_devices(devCtrl):
    """
    BLE scan CHIP device
    BLE scanning for 10 seconds and collect the results 
    :param devCtrl: device controller instance
    :return: List of visible BLE devices
    """
    devices = []
    bleMgr = BleManager(devCtrl)
    bleMgr.scan("-t 10")

    for device in bleMgr.peripheral_list:
        devIdInfo = bleMgr.get_peripheral_devIdInfo(device)
        if devIdInfo:
            log.info("Found CHIP device {}".format(device.Name))
            devices.append(devIdInfo)

    return devices

def run_wifi_provisioning(devCtrl, ssid, password, discriminator, pinCode, nodeId=None):
    """
    Run WiFi provisionning via BLE 
    :param devCtrl: device controller instance
    :param ssid: network ssid
    :param password: network password
    :param discriminator: CHIP device discriminator
    :param pinCode: CHIP device pin code
    :param nodeId: default value of node ID
    :return: node ID is provisioning  successful, otherwise None
    """
    if nodeId == None:
        nodeId = random.randint(1, 1000000)

    try:
        devCtrl.SetWifiCredential(ssid, password)
        devCtrl.ConnectBLE(int(discriminator), int(pinCode), int(nodeId))
    except exceptions.ChipStackException as ex:
        log.error("WiFi provisioning failed: {}".format(str(ex)))
        return None

    return nodeId