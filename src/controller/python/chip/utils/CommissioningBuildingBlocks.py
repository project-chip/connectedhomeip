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
import os

import chip.clusters as Clusters
import chip.tlv
from chip.clusters import OperationalCredentials as opCreds
from chip.clusters import GeneralCommissioning as generalCommissioning

_UINT16_MAX = 65535

logger = logging.getLogger()


async def _IsNodeInFabricList(devCtrl, nodeId):
    resp = await devCtrl.ReadAttribute(nodeId, [(opCreds.Attributes.Fabrics)])
    listOfFabricsDescriptor = resp[0][opCreds][Clusters.OperationalCredentials.Attributes.Fabrics]
    for fabricDescriptor in listOfFabricsDescriptor:
        if fabricDescriptor.nodeId == nodeId:
            return True

    return False


async def AddNOCForNewFabricFromExisting(commissionerDevCtrl, newFabricDevCtrl, existingNodeId, newNodeId):
    """ Perform sequence to commission new frabric using existing commissioned fabric.

    Args:
        commissionerDevCtrl (ChipDeviceController): Already commissioned device controller used
            to commission a new fabric on `newFabricDevCtrl`.
        newFabricDevCtrl (ChipDeviceController): New device controller which is used for the new
            fabric we are establishing.
        existingNodeId (int): Node ID of the server we are establishing a CASE session on the
            existing fabric that we will used to perform AddNOC.
        newNodeId (int): Node ID that we would like to server to used on the new fabric being
            added.

    Return:
        bool: True if successful, False otherwise.

    """
    resp = await commissionerDevCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(60), timedRequestTimeoutMs=1000)
    if resp.errorCode is not generalCommissioning.Enums.CommissioningError.kOk:
        return False

    csrForAddNOC = await commissionerDevCtrl.SendCommand(existingNodeId, 0, opCreds.Commands.CSRRequest(CSRNonce=os.urandom(32)))

    chainForAddNOC = newFabricDevCtrl.IssueNOCChain(csrForAddNOC, newNodeId)
    if chainForAddNOC.rcacBytes is None or chainForAddNOC.icacBytes is None or chainForAddNOC.nocBytes is None or chainForAddNOC.ipkBytes is None:
        # Expiring the failsafe timer in an attempt to clean up.
        await commissionerDevCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False

    await commissionerDevCtrl.SendCommand(existingNodeId, 0, opCreds.Commands.AddTrustedRootCertificate(chainForAddNOC.rcacBytes))
    resp = await commissionerDevCtrl.SendCommand(existingNodeId, 0, opCreds.Commands.AddNOC(chainForAddNOC.nocBytes, chainForAddNOC.icacBytes, chainForAddNOC.ipkBytes, newFabricDevCtrl.GetNodeId(), 0xFFF1))
    if resp.statusCode is not opCreds.Enums.OperationalCertStatus.kSuccess:
        # Expiring the failsafe timer in an attempt to clean up.
        await commissionerDevCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False

    resp = await newFabricDevCtrl.SendCommand(newNodeId, 0, generalCommissioning.Commands.CommissioningComplete())
    if resp.errorCode is not generalCommissioning.Enums.CommissioningError.kOk:
        # Expiring the failsafe timer in an attempt to clean up.
        await commissionerDevCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False

    if not await _IsNodeInFabricList(newFabricDevCtrl, newNodeId):
        return False

    return True


async def UpdateNOC(devCtrl, existingNodeId, newNodeId):
    """ Perform sequence to generate a new NOC cert and issue updated NOC to server.

    Args:
        commissionerDevCtrl (ChipDeviceController): Already commissioned device controller used
            which we wish to update the NOC certificate for.
        existingNodeId (int): Node ID of the server we are establishing a CASE session to
            perform UpdateNOC.
        newNodeId (int): Node ID that we would like to update the server to use. This can be
            the same as `existingNodeId` if you wish to keep the node ID unchanged, but only
            update the NOC certificate.

    Return:
        bool: True if successful, False otherwise.

    """
    resp = await devCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(600), timedRequestTimeoutMs=1000)
    if resp.errorCode is not generalCommissioning.Enums.CommissioningError.kOk:
        return False
    csrForUpdateNOC = await devCtrl.SendCommand(
        existingNodeId, 0, opCreds.Commands.CSRRequest(CSRNonce=os.urandom(32), isForUpdateNOC=True))
    chainForUpdateNOC = devCtrl.IssueNOCChain(csrForUpdateNOC, newNodeId)
    if chainForUpdateNOC.rcacBytes is None or chainForUpdateNOC.icacBytes is None or chainForUpdateNOC.nocBytes is None or chainForUpdateNOC.ipkBytes is None:
        await devCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False

    resp = await devCtrl.SendCommand(existingNodeId, 0, opCreds.Commands.UpdateNOC(chainForUpdateNOC.nocBytes, chainForUpdateNOC.icacBytes))
    if resp.statusCode is not opCreds.Enums.OperationalCertStatus.kSuccess:
        # Expiring the failsafe timer in an attempt to clean up.
        await devCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False

    resp = await devCtrl.SendCommand(newNodeId, 0, generalCommissioning.Commands.CommissioningComplete())
    if resp.errorCode is not generalCommissioning.Enums.CommissioningError.kOk:
        # Expiring the failsafe timer in an attempt to clean up.
        await devCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False

    if not await _IsNodeInFabricList(devCtrl, newNodeId):
        return False

    return True
