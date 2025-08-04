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
import logging
import random
import re

from chip import discovery, exceptions
from chip.clusters import Attribute as ClusterAttribute
from chip.clusters import Objects as GeneratedObjects
from chip.interaction_model import delegate as IM
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
    if ret is None or len(ret) < 2:
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
    log.info("Device found at %r" % res[0])
    return res[0]


def connect_device(devCtrl, setupPayload, commissionableDevice, nodeId=None):
    """
    Connect to Matter discovered device on network
    :param devCtrl: device controller instance
    :param setupPayload: device setup payload
    :param commissionableDevice: CommissionableNode object with discovered device
    :param nodeId: device node ID
    :return: node ID if connection successful or None if failed
    """
    if nodeId is None:
        nodeId = random.randint(1, 1000000)

    log.info("Connecting to device %d" % nodeId)

    pincode = int(setupPayload.attributes['SetUpPINCode'])
    try:
        devCtrl.CommissionOnNetwork(
            nodeId, pincode, filterType=discovery.FilterType.INSTANCE_NAME, filter=commissionableDevice.instanceName)
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
        devCtrl.MarkSessionDefunct(nodeId)
    except exceptions.ChipStackException as ex:
        log.error("CloseSession failed {}".format(str(ex)))
        return False
    return True


def send_zcl_command(devCtrl, cluster: str, command: str, nodeId: int, endpoint: int, args, requestTimeoutMs: int = None):
    """
    Send ZCL command to device.
    :param devCtrl: device controller instance
    :param cluster: cluster name
    :param command: command name
    :param nodeId: device node ID
    :param endpoint: device endpoint
    :parma args: command argument in dictionary format
    :param requestTimeoutMs: command request timeout in ms
    :return: error code and command response
    """
    res = None
    err = 0
    try:
        allCommands = devCtrl.ZCLCommandList()
        if cluster not in allCommands:
            raise exceptions.UnknownCluster(cluster)
        cmd = allCommands.get(cluster).get(command)
        if cmd is None:
            raise exceptions.UnknownCommand(cluster, command)

        clusterObj = getattr(GeneratedObjects, cluster)
        commandObj = getattr(clusterObj.Commands, command)
        if args is not None:
            req = commandObj(**args)
        else:
            req = commandObj()

        res = asyncio.run(devCtrl.SendCommand(int(nodeId), int(endpoint), req, timedRequestTimeoutMs=requestTimeoutMs))

    except exceptions.ChipStackException as ex:
        log.error("An exception occurred during processing ZCL command: {}".format(str(ex)))
        err = -1
    except Exception as ex:
        log.error("An exception occurred during processing input: {}".format(str(ex)))
        err = -1

    return (err, res)


def write_zcl_attribute(devCtrl, cluster: str, attribute: str, nodeId: int, endpoint: int, value):
    """
    Write ZCL attribute to device.
    :param devCtrl: device controller instance
    :param cluster: cluster name
    :param attribute: attribute name
    :param nodeId: device node ID
    :param endpoint: device endpoint
    :parma value: attribute value to write
    :return: error code and attribute response
    """
    res = None
    err = 0
    try:
        allAttrs = devCtrl.ZCLAttributeList()
        if cluster not in allAttrs:
            raise exceptions.UnknownCluster(cluster)

        attrDetails = allAttrs.get(cluster).get(attribute)
        if attrDetails is None:
            raise exceptions.UnknownAttribute(cluster, attribute)

        clusterObj = getattr(GeneratedObjects, cluster)
        attributeObj = getattr(clusterObj.Attributes, attribute)
        req = attributeObj(value)

        res = asyncio.run(devCtrl.WriteAttribute(nodeId, [(endpoint, req)]))

    except exceptions.ChipStackException as ex:
        log.error("An exception occurred during processing ZCL attribute: {}".format(str(ex)))
        err = -1
    except Exception as ex:
        log.error("An exception occurred during processing input: {}".format(str(ex)))
        err = -1

    return (err, res)


def read_zcl_attribute(devCtrl, cluster: str, attribute: str, nodeId: int, endpoint: int):
    """
    Read ZCL attribute from device.
    :param devCtrl: device controller instance
    :param cluster: cluster name
    :param attribute: attribute name
    :param nodeId: device node ID
    :param endpoint: device endpoint
    :return: error code and attribute response
    """
    res = None
    err = 0
    try:
        allAttrs = devCtrl.ZCLAttributeList()
        if cluster not in allAttrs:
            raise exceptions.UnknownCluster(cluster)

        attrDetails = allAttrs.get(cluster).get(attribute)
        if attrDetails is None:
            raise exceptions.UnknownAttribute(cluster, attribute)

        clusterObj = getattr(GeneratedObjects, cluster)
        attributeObj = getattr(clusterObj.Attributes, attribute)

        result = asyncio.run(devCtrl.ReadAttribute(nodeId, [(endpoint, attributeObj)]))

        path = ClusterAttribute.AttributePath(
            EndpointId=endpoint, Attribute=attributeObj)

        res = IM.AttributeReadResult(path=IM.AttributePath(nodeId=nodeId, endpointId=path.EndpointId, clusterId=path.ClusterId,
                                     attributeId=path.AttributeId), status=0, value=result[endpoint][clusterObj][attributeObj],
                                     dataVersion=result[endpoint][clusterObj][ClusterAttribute.DataVersion])

    except exceptions.ChipStackException as ex:
        log.error("An exception occurred during processing ZCL attribute: {}".format(str(ex)))
        err = -1
    except Exception as ex:
        log.error("An exception occurred during processing input: {}".format(str(ex)))
        err = -1

    return (err, res)


def get_shell_commands_from_help_response(response):
    """
    Parse shell help command response to get the list of supported commands
    :param response: help command response
    :return: list of supported commands
    """
    return [line.split()[0].strip() for line in response]


def get_log_messages_from_response(response):
    """
    Parse shell help command response to get the list of supported commands
    :param response: device response
    :return: raw log messages
    """
    return [' '.join(line.split()[2:]) for line in response]
