#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
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

import asyncio
import ctypes
import logging
import os
import random
import re
import shlex
from time import sleep

from chip import discovery, exceptions
from chip.clusters import Objects as GeneratedObjects
from chip.setup_payload import SetupPayload

log = logging.getLogger(__name__)

IP_ADDRESS_BUFFER_LEN = 100


def get_setup_payload(device):
    """
    Get device setup payload from logs
    :param device: serial device instance
    :return: setup payload or None
    """
    ret = device.wait_for_output("SetupQRCode")
    if ret == None or len(ret) < 2:
        return None

    qr_code = re.sub(
        r"[\[\]]", "", ret[-1].partition("SetupQRCode:")[2]).strip()
    try:
        setup_payload = SetupPayload().ParseQrCode(
            "VP:vendorpayload%{}".format(qr_code))
    except exceptions.ChipStackError as ex:
        log.error("SetupPayload failed {}".format(str(ex)))
        return None

    return setup_payload


def discover_device(devCtrl, setupPayload):
    """
    Discover specific device in network.
    Search by device discriminator from setup payload
    :param devCtrl: device controller instance
    :param setupPayload: device setup payload
    :return: CommissionableNode object if node device discovered or None if failed
    """
    log.info("Attempting to find device on network")
    longDiscriminator = int(setupPayload.attributes['Long discriminator'])
    try:
        res = devCtrl.DiscoverCommissionableNodes(
            discovery.FilterType.LONG_DISCRIMINATOR, longDiscriminator, stopOnFirst=True, timeoutSecond=5)
    except exceptions.ChipStackError as ex:
        log.error("DiscoverCommissionableNodes failed {}".format(str(ex)))
        return None
    if not res:
        log.info("Device not found")
        return None
    return res[0]


def connect_device(setupPayload, commissionableDevice, nodeId=None):
    """
    Connect to Matter discovered device on network
    :param setupPayload: device setup payload
    :param commissionableDevice: CommissionableNode object with discovered device
    :param nodeId: device node ID
    :return: node ID if connection successful or None if failed
    """
    if nodeId == None:
        nodeId = random.randint(1, 1000000)

    pincode = int(setupPayload.attributes['SetUpPINCode'])
    try:
        commissionableDevice.Commission(nodeId, pincode)
    except exceptions.ChipStackError as ex:
        log.error("Commission discovered device failed {}".format(str(ex)))
        return None
    return nodeId


def disconnect_device(devCtrl, nodeId):
    """
    Disconnect Matter device
    :param devCtrl: device controller instance
    :param nodeId: device node ID
    :return: node ID if connection successful or None if failed
    """
    try:
        devCtrl.CloseSession(nodeId)
    except exceptions.ChipStackException as ex:
        log.error("CloseSession failed {}".format(str(ex)))
        return False
    return True


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


def ParseValueWithStruct(value, cluster):
    return eval(f"GeneratedObjects.{cluster}.Structs.{value}")


def ParseValue(value, valueType, cluster):
    if valueType:
        return ParseValueWithType(value, valueType)
    elif value.find(":") > 0 and value.split(":", 1)[0] == "struct":
        return ParseValueWithStruct(value.split(":", 1)[1], cluster)
    else:
        raise ParsingError('Cannot parse value: {}'.format(value))


def FormatZCLArguments(cluster, args, cmdArgsWithType):
    cmdArgsDict = {}
    for kvPair in args:
        if kvPair.find("=") < 0:
            raise ParsingError("Argument should in key=value format")
        key, value = kvPair.split("=", 1)
        valueType = cmdArgsWithType.get(key, None)
        cmdArgsDict[key] = ParseValue(value, valueType, cluster)
    return cmdArgsDict


def send_zcl_command(devCtrl, line, requestTimeoutMs: int = None):
    """
    Format and send ZCL message to device.
    :param devCtrl: device controller instance
    :param line: command line
    :param requestTimeoutMs: command request timeout in ms
    :return: error code and command response
    """
    res = None
    err = 0
    try:
        args = shlex.split(line)
        if len(args) < 4:
            raise exceptions.InvalidArgumentCount(4, len(args))

        cluster, command, nodeId, endpoint = args[0:4]
        cmdArgsLine = args[4:]
        allCommands = devCtrl.ZCLCommandList()
        if cluster not in allCommands:
            raise exceptions.UnknownCluster(cluster)
        cmdArgsWithType = allCommands.get(cluster).get(command, None)
        # When command takes no arguments, (not command) is True
        if command == None:
            raise exceptions.UnknownCommand(cluster, command)

        args = FormatZCLArguments(cluster, cmdArgsLine, cmdArgsWithType)
        clusterObj = getattr(GeneratedObjects, cluster)
        commandObj = getattr(clusterObj.Commands, command)
        req = commandObj(**args)

        res = asyncio.run(devCtrl.SendCommand(int(nodeId), int(endpoint), req, timedRequestTimeoutMs=requestTimeoutMs))

    except exceptions.ChipStackException as ex:
        log.error("An exception occurred during processing ZCL command: {}".format(str(ex)))
        err = -1
    except Exception as ex:
        log.error("An exception occurred during processing input: {}".format(str(ex)))
        err = -1

    return (err, res)


def read_zcl_attribute(devCtrl, line):
    """
    Read ZCL attribute from device:
    <cluster> <attribute> <nodeid> <endpoint>
    :param devCtrl: device controller instance
    :param line: command line
    :return: error code and attribute response
    """
    res = None
    err = 0
    try:
        args = shlex.split(line)
        if len(args) < 4:
            raise exceptions.InvalidArgumentCount(4, len(args))

        cluster, attribute, nodeId, endpoint = args[0:4]
        allAttrs = devCtrl.ZCLAttributeList()
        if cluster not in allAttrs:
            raise exceptions.UnknownCluster(cluster)

        attrDetails = allAttrs.get(cluster).get(attribute, None)
        if attrDetails == None:
            raise exceptions.UnknownAttribute(cluster, attribute)

        res = devCtrl.ZCLReadAttribute(cluster, attribute, int(
            nodeId), int(endpoint), 0)
    except exceptions.ChipStackException as ex:
        log.error("An exception occurred during processing ZCL attribute: {}".format(str(ex)))
        err = -1
    except Exception as ex:
        log.error("An exception occurred during processing input: {}".format(str(ex)))
        err = -1

    return (err, res)
