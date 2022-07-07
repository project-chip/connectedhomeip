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
import chip.clusters as Clusters
import chip.tlv

_UINT16_MAX = 65535

async def _IsBasicAttributeReadSuccessful(devCtrl, nodeId):
    resp = await devCtrl.ReadAttribute(nodeId, [(Clusters.Basic.Attributes.ClusterRevision)])
    clusterRevision = resp[0][Clusters.Basic][Clusters.Basic.Attributes.ClusterRevision]
    if not isinstance(clusterRevision, chip.tlv.uint) or clusterRevision < 1 or clusterRevision > _UINT16_MAX:
        return False
    return True


async def AddNOC(commissionedDevCtrl, newDevCtrl, existingNodeId, newNodeId):
    resp = await commissionedDevCtrl.SendCommand(existingNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(60), timedRequestTimeoutMs=1000)
    if resp.errorCode is not Clusters.GeneralCommissioning.Enums.CommissioningError.kOk:
        return False
  
    csrForAddNOC = await commissionedDevCtrl.SendCommand(existingNodeId, 0, Clusters.OperationalCredentials.Commands.CSRRequest(CSRNonce=b'1' * 32))
  
    chainForAddNOC = newDevCtrl.IssueNOCChain(csrForAddNOC, newNodeId)
    if chainForAddNOC.rcacBytes is None or chainForAddNOC.icacBytes is None or chainForAddNOC.nocBytes is None or chainForAddNOC.ipkBytes is None:
        # Expiring the failsafe timer in an attempt to clean up.
        await commissionedDevCtrl.SendCommand(existingNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False
  
    # TODO error check on the commands below
    await commissionedDevCtrl.SendCommand(existingNodeId, 0, Clusters.OperationalCredentials.Commands.AddTrustedRootCertificate(chainForAddNOC.rcacBytes))
    resp = await commissionedDevCtrl.SendCommand(existingNodeId, 0, Clusters.OperationalCredentials.Commands.AddNOC(chainForAddNOC.nocBytes, chainForAddNOC.icacBytes, chainForAddNOC.ipkBytes, newDevCtrl.GetNodeId(), 0xFFF1))
    if resp.statusCode is not Clusters.OperationalCredentials.Enums.OperationalCertStatus.kSuccess:
        # Expiring the failsafe timer in an attempt to clean up.
        await commissionedDevCtrl.SendCommand(existingNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False

    resp = await newDevCtrl.SendCommand(newNodeId, 0, Clusters.GeneralCommissioning.Commands.CommissioningComplete())
    if resp.errorCode is not Clusters.GeneralCommissioning.Enums.CommissioningError.kOk:
        # Expiring the failsafe timer in an attempt to clean up.
        await commissionedDevCtrl.SendCommand(existingNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False
    
    if not await _IsBasicAttributeReadSuccessful(newDevCtrl, newNodeId):
        return False

    return True

async def UpdateNOC(devCtrl, existingNodeId, newNodeId):
    resp = await devCtrl.SendCommand(existingNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(600), timedRequestTimeoutMs=1000)
    if resp.errorCode is not Clusters.GeneralCommissioning.Enums.CommissioningError.kOk:
        return False
    csrForUpdateNOC = await devCtrl.SendCommand(
        existingNodeId, 0, Clusters.OperationalCredentials.Commands.CSRRequest(CSRNonce=b'1' * 32, isForUpdateNOC=True))
    chainForUpdateNOC = devCtrl.IssueNOCChain(csrForUpdateNOC, newNodeId)
    if chainForUpdateNOC.rcacBytes is None or chainForUpdateNOC.icacBytes is None or chainForUpdateNOC.nocBytes is None or chainForUpdateNOC.ipkBytes is None:
        await devCtrl.SendCommand(existingNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False

    resp = await devCtrl.SendCommand(existingNodeId, 0, Clusters.OperationalCredentials.Commands.UpdateNOC(chainForUpdateNOC.nocBytes, chainForUpdateNOC.icacBytes))
    if resp.statusCode is not Clusters.OperationalCredentials.Enums.OperationalCertStatus.kSuccess:
        # Expiring the failsafe timer in an attempt to clean up.
        await devCtrl.SendCommand(existingNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False

    resp = await devCtrl.SendCommand(newNodeId, 0, Clusters.GeneralCommissioning.Commands.CommissioningComplete())
    if resp.errorCode is not Clusters.GeneralCommissioning.Enums.CommissioningError.kOk:
        # Expiring the failsafe timer in an attempt to clean up.
        await devCtrl.SendCommand(existingNodeId, 0, Clusters.GeneralCommissioning.Commands.ArmFailSafe(0), timedRequestTimeoutMs=1000)
        return False

    if not await _IsBasicAttributeReadSuccessful(devCtrl, newNodeId):
        return False

    return True
