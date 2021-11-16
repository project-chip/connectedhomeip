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
import shlex

from chip import exceptions

if platform.system() == 'Darwin':
    from chip.ChipCoreBluetoothMgr import CoreBluetoothManager as BleManager
elif sys.platform.startswith('linux'):
    from chip.ChipBluezMgr import BluezManager as BleManager

import logging
log = logging.getLogger(__name__)

class ParsingError(exceptions.ChipStackException):
    def __init__(self, msg=None):
        self.msg = "Parsing Error: " + msg

    def __str__(self):
        return self.msg

def ParseEncodedString(value):
    if value.find(":") < 0:
        raise ParsingError(
            "Value should be encoded in encoding:encodedvalue format")
    enc, encValue = value.split(":", 1)
    if enc == "str":
        return encValue.encode("utf-8") + b'\x00'
    elif enc == "hex":
        return bytes.fromhex(encValue)
    raise ParsingError("Only str and hex encoding is supported")


def ParseValueWithType(value, type):
    if type == 'int':
        return int(value)
    elif type == 'str':
        return value
    elif type == 'bytes':
        return ParseEncodedString(value)
    elif type == 'bool':
        return (value.upper() not in ['F', 'FALSE', '0'])
    else:
        raise ParsingError('Cannot recognize type: {}'.format(type))

def FormatZCLArguments(args, command):
    commandArgs = {}
    for kvPair in args:
        if kvPair.find("=") < 0:
            raise ParsingError("Argument should in key=value format")
        key, value = kvPair.split("=", 1)
        valueType = command.get(key, None)
        commandArgs[key] = ParseValueWithType(value, valueType)
    return commandArgs

def send_zcl_command(devCtrl, line):
    """
    Send ZCL message to device:
    <cluster> <command> <nodeid> <endpoint> <groupid> [key=value]...
    :param devCtrl: device controller instance
    :param line: command line
    :return: error code and command responde
    """
    res = None
    err = 0
    try:
        args = shlex.split(line)
        all_commands = devCtrl.ZCLCommandList()
        if len(args) < 5:
            raise exceptions.InvalidArgumentCount(5, len(args))

        if args[0] not in all_commands:
            raise exceptions.UnknownCluster(args[0])
        command = all_commands.get(args[0]).get(args[1], None)
        # When command takes no arguments, (not command) is True
        if command == None:
            raise exceptions.UnknownCommand(args[0], args[1])
        err, res = devCtrl.ZCLSend(args[0], args[1], int(
            args[2]), int(args[3]), int(args[4]), FormatZCLArguments(args[5:], command), blocking=True)
        if err != 0:
            log.error("Failed to receive command response: {}".format(res))
        elif res != None:
            log.info("Received command status response:")
            log.info(res)
        else:
            log.info("Success, no status code is attached with response.")
    except exceptions.ChipStackException as ex:
        log.error("An exception occurred during process ZCL command:")
        log.error(str(ex))
        err = -1
    except Exception as ex:
        log.error("An exception occurred during processing input:")
        log.error(str(ex))
        err = -1

    return (err, res)

def scan_chip_ble_devices(devCtrl, name):
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
        if device.Name != name:
            continue
        devIdInfo = bleMgr.get_peripheral_devIdInfo(device)
        if devIdInfo:
            devices.append(devIdInfo)

    return devices


def connect_device_over_ble(devCtrl, discriminator, pinCode, nodeId=None):
    """
    Connect to Matter accessory device over BLE 
    :param devCtrl: device controller instance
    :param discriminator: CHIP device discriminator
    :param pinCode: CHIP device pin code
    :param nodeId: default value of node ID
    :return: node ID is provisioning successful, otherwise None
    """
    if nodeId == None:
        nodeId = random.randint(1, 1000000)

    try:
        devCtrl.ConnectBLE(int(discriminator), int(pinCode), int(nodeId))
    except exceptions.ChipStackException as ex:
        log.error("Connect device over BLE failed: {}".format(str(ex)))
        return None

    return nodeId

def close_ble(devCtrl):
    """
    Close the BLE connection
    :param devCtrl: device controller instance
    :return: true if successful, otherwise false
    """
    try:
        devCtrl.CloseBLEConnection()
    except exceptions.ChipStackException as ex:
        log.error("Close BLE connection failed: {}".format(str(ex)))
        return False

    return True

def commissioning_wifi(devCtrl, ssid, password, nodeId):
    """
    Commissioning a Wi-Fi device 
    :param devCtrl: device controller instance
    :param ssid: network ssid
    :param password: network password
    :param nodeId: value of node ID
    :return: error code
    """

    # Inject the credentials to the device
    err, res = send_zcl_command(devCtrl, "NetworkCommissioning AddWiFiNetwork {} 0 0 ssid=str:{} credentials=str:{} breadcrumb=0 timeoutMs=1000".format(nodeId, ssid, password))
    if err != 0 and res["Status"] != 0:
        log.error("Set Wi-Fi credentials failed [{}]".format(err))
        return err
    
    # Enable the Wi-Fi interface
    err, res = send_zcl_command(devCtrl, "NetworkCommissioning EnableNetwork {} 0 0 networkID=str:{} breadcrumb=0 timeoutMs=1000".format(nodeId, ssid))
    if err != 0 and res["Status"] != 0:
        log.error("Enable Wi-Fi failed [{}]".format(err))
        return err

    return err