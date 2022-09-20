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

import os
import random
import shlex
import re
import ctypes
import asyncio
from time import sleep

from chip.setup_payload import SetupPayload
from chip import exceptions

from chip.clusters import Objects as GeneratedObjects

import logging
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


def wait_for_one_discovered_device(devCtrl):
    """
    Wait for discovered single node device
    :param devCtrl: device controller instance
    :return: True if node device discovered or False if failed
    """
    addrBuffer = ctypes.create_string_buffer(IP_ADDRESS_BUFFER_LEN)
    count = 0
    maxWaitTime = 2
    try:
        while (not devCtrl.GetIPForDiscoveredDevice(0, addrBuffer, IP_ADDRESS_BUFFER_LEN) and count < maxWaitTime):
            sleep(0.2)
            count = count + 0.2
        return count < maxWaitTime
    except exceptions.ChipStackError as ex:
        log.error("GetIPForDiscoveredDevice failed {}".format(str(ex)))
        return False


def discover_device(devCtrl, setupPayload):
    """
    Discover specific device in network.
    Search by device discriminator from setup payload
    :param devCtrl: device controller instance
    :param setupPayload: device setup payload
    :return: True if node device discovered or False if failed
    """
    log.info("Attempting to find device on network")
    longDiscriminator = ctypes.c_uint16(
        int(setupPayload.attributes['Long discriminator']))
    try:
        devCtrl.DiscoverCommissionableNodesLongDiscriminator(longDiscriminator)
    except exceptions.ChipStackError as ex:
        log.error("DiscoverCommissionableNodesLongDiscriminator failed {}".format(str(ex)))
        return False
    log.info("Waiting for device responses...")
    return wait_for_one_discovered_device(devCtrl)


def get_discovered_device_ip_address(devCtrl, index):
    """
    Get discovered device IP address under index
    :param devCtrl: device controller instance
    :param index: discovered device index
    :return: IP address of discovered device or None if failed
    """
    addrBuffer = ctypes.create_string_buffer(IP_ADDRESS_BUFFER_LEN)
    try:
        devCtrl.GetIPForDiscoveredDevice(index, addrBuffer, IP_ADDRESS_BUFFER_LEN)
    except exceptions.ChipStackError as ex:
        log.error("GetIPForDiscoveredDevice failed {}".format(str(ex)))
        return None
    return addrBuffer.value.decode('utf-8')


def connect_device(devCtrl, setupPayload, ipAddress, nodeId=None):
    """
    Connect to Matter device on network
    :param devCtrl: device controller instance
    :param setupPayload: device setup payload
    :param ipAddress: device IP address
    :param nodeId: device node ID
    :return: node ID if connection successful or None if failed
    """
    if nodeId == None:
        nodeId = random.randint(1, 1000000)

    pincode = ctypes.c_uint32(
        int(setupPayload.attributes['SetUpPINCode']))
    try:
        devCtrl.CommissionIP(ipAddress, pincode, nodeId)
    except exceptions.ChipStackError as ex:
        log.error("CommissionIP failed {}".format(str(ex)))
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
