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


import asyncio
import logging
import platform
import random
import re
import shlex
import sys

from chip import exceptions
from chip.setup_payload import SetupPayload

if platform.system() == 'Darwin':
    from chip.ChipCoreBluetoothMgr import CoreBluetoothManager as BleManager
elif sys.platform.startswith('linux'):
    from chip.ChipBluezMgr import BluezManager as BleManager

log = logging.getLogger(__name__)


class ParsingError(exceptions.ChipStackException):
    def __init__(self, msg=None):
        self.msg = "Parsing Error: " + msg

    def __str__(self):
        return self.msg


def get_device_details(device):
    """
    Get device details from logs
    :param device: serial device instance
    :return: device details dictionary or None
    """
    ret = device.wait_for_output("SetupQRCode")
    if ret is None or len(ret) < 2:
        return None

    qr_code = re.sub(
        r"[\[\]]", "", ret[-1].partition("SetupQRCode:")[2]).strip()
    try:
        device_details = dict(SetupPayload().ParseQrCode(
            "VP:vendorpayload%{}".format(qr_code)).attributes)
    except exceptions.ChipStackError as ex:
        log.error(ex.msg)
        return None

    return device_details


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

        cluster = args[0]
        command = args[1]
        if cluster not in all_commands:
            raise exceptions.UnknownCluster(cluster)
        commandObj = all_commands.get(cluster).get(command, None)
        # When command takes no arguments, (not command) is True
        if commandObj is None:
            raise exceptions.UnknownCommand(cluster, command)

        try:
            req = commandObj(**FormatZCLArguments(args[5:], commandObj))
        except BaseException:
            raise exceptions.UnknownCommand(cluster, command)

        nodeid = int(args[2])
        endpoint = int(args[3])
        try:
            res = asyncio.run(devCtrl.SendCommand(nodeid, endpoint, req))
            logging.debug(f"CommandResponse {res}")
            if res is not None:
                log.info("Success, received command response:")
                log.info(res)
            else:
                log.info("Success, no command response.")
        except exceptions.InteractionModelError as ex:
            return (int(ex.status), None)
            log.error("Failed to send ZCL command [{}] {}.".format(int(ex.status), None))
    except exceptions.ChipStackException as ex:
        log.error("An exception occurred during processing ZCL command:")
        log.error(str(ex))
        err = -1
    except Exception as ex:
        log.error("An exception occurred during processing input:")
        log.error(str(ex))
        err = -1

    return (err, res)


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
            devInfo = devIdInfo.__dict__
            devInfo["name"] = device.Name
            devices.append(devInfo)

    return devices


def check_chip_ble_devices_advertising(devCtrl, name, deviceDetails=None):
    """
    Check if CHIP device advertise
    BLE scanning for 10 seconds and compare with device details
    :param devCtrl: device controller instance
    :param name: device advertising name
    :param name: device details
    :return: True if device advertise else False
    """
    ble_chip_device = scan_chip_ble_devices(devCtrl)
    if ble_chip_device is None or len(ble_chip_device) == 0:
        log.info("No BLE CHIP device found")
        return False

    chip_device_found = False

    for ble_device in ble_chip_device:
        if deviceDetails is not None:
            if (ble_device["name"] == name and
                int(ble_device["discriminator"]) == int(deviceDetails["Discriminator"]) and
                int(ble_device["vendorId"]) == int(deviceDetails["VendorID"]) and
                    int(ble_device["productId"]) == int(deviceDetails["ProductID"])):
                chip_device_found = True
                break
        else:
            if (ble_device["name"] == name):
                chip_device_found = True
                break

    return chip_device_found


def connect_device_over_ble(devCtrl, discriminator, pinCode, nodeId=None):
    """
    Connect to Matter accessory device over BLE
    :param devCtrl: device controller instance
    :param discriminator: CHIP device discriminator
    :param pinCode: CHIP device pin code
    :param nodeId: default value of node ID
    :return: node ID is provisioning successful, otherwise None
    """
    if nodeId is None:
        nodeId = random.randint(1, 1000000)

    try:
        devCtrl.ConnectBLE(int(discriminator), int(pinCode), int(nodeId))
    except exceptions.ChipStackException as ex:
        log.error("Connect device over BLE failed: {}".format(str(ex)))
        return None

    return nodeId


def close_connection(devCtrl, nodeId):
    """
    Close the BLE connection
    :param devCtrl: device controller instance
    :return: true if successful, otherwise false
    """
    try:
        devCtrl.MarkSessionDefunct(nodeId)
    except exceptions.ChipStackException as ex:
        log.error("Close session failed: {}".format(str(ex)))
        return False

    return True


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
    err, res = send_zcl_command(devCtrl,
                                "NetworkCommissioning "
                                "AddOrUpdateWiFiNetwork {} 0 0 "
                                "ssid=str:{} credentials=str:{} breadcrumb=0 timeoutMs=1000".format(
                                    nodeId,
                                    ssid,
                                    password
                                ))
    if err != 0 and res["Status"] != 0:
        log.error("Set Wi-Fi credentials failed [{}]".format(err))
        return err

    # Enable the Wi-Fi interface
    err, res = send_zcl_command(
        devCtrl, "NetworkCommissioning ConnectNetwork {} 0 0 networkID=str:{} breadcrumb=0 timeoutMs=1000".format(nodeId, ssid))
    if err != 0 and res["Status"] != 0:
        log.error("Enable Wi-Fi failed [{}]".format(err))
        return err

    return err


def resolve_device(devCtrl, nodeId):
    """
    Discover IP address and port of the device.
    :param devCtrl: device controller instance
    :param nodeId: value of node ID
    :return: device IP address and port if successful, otherwise None
    """
    ret = None
    try:
        devCtrl.ResolveNode(int(nodeId))
        ret = devCtrl.GetAddressAndPort(int(nodeId))
        if ret is None:
            log.error("Get address and port failed")
    except exceptions.ChipStackException as ex:
        log.error("Resolve node failed {}".format(str(ex)))

    return ret
