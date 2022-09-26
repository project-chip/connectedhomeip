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
from chip.FabricAdmin import FabricAdmin as FabricAdmin
import typing
from chip.ChipDeviceCtrl import ChipDeviceController as ChipDeviceController
from chip.clusters.Types import *

_UINT16_MAX = 65535

logger = logging.getLogger('CommissioningBuildingBlocks')


async def _IsNodeInFabricList(devCtrl, nodeId):
    resp = await devCtrl.ReadAttribute(nodeId, [(opCreds.Attributes.Fabrics)])
    listOfFabricsDescriptor = resp[0][opCreds][Clusters.OperationalCredentials.Attributes.Fabrics]
    for fabricDescriptor in listOfFabricsDescriptor:
        if fabricDescriptor.nodeId == nodeId:
            return True

    return False


async def GrantPrivilege(adminCtrl: ChipDeviceController, grantedCtrl: ChipDeviceController, privilege: Clusters.AccessControl.Enums.Privilege, targetNodeId: int, targetCatTags: typing.List[int] = []):
    ''' Given an existing controller with admin privileges over a target node, grants the specified privilege to the new ChipDeviceController instance to the entire Node. This is achieved
        by updating the ACL entries on the target.

        This will automatically take care of working within the minimas of the target as well as doing an efficient read-modify-write operation that takes
        into consideration the existing entries on the target and minimizing the total number of ACL entries written per fabric.

        Args:
            adminCtrl:      ChipDeviceController instance with admin privileges over the target node
            grantedCtrl:    ChipDeviceController instance that is being granted the new privilege.
            privilege:      Privilege to grant to the granted controller. If None, no privilege is granted.
            targetNodeId:   Target node to which the controller is granted privilege.
            targetCatTag:   Target 32-bit CAT tag that is granted privilege. If provided, this will be used in the subject list instead of the nodeid of that of grantedCtrl.
    '''
    data = await adminCtrl.ReadAttribute(targetNodeId, [(Clusters.AccessControl.Attributes.Acl)])
    if 0 not in data:
        raise ValueError("Did not get back any data (possible cause: controller has no access..")

    currentAcls = data[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

    if len(targetCatTags) != 0:
        # Convert to an ACL subject format in CAT range
        targetSubjects = [tag | 0xFFFF_FFFD_0000_0000 for tag in targetCatTags]
    else:
        targetSubjects = [grantedCtrl.nodeId]

    if (len(targetSubjects) > 4):
        raise ValueError(f"List of target subjects of len {len(targetSubjects)} exceeeded the minima of 4!")

    # Step 1: Wipe the subject from all existing ACLs.
    for acl in currentAcls:
        if (acl.subjects != NullValue):
            acl.subjects = [subject for subject in acl.subjects if subject not in targetSubjects]

    if (privilege):
        addedPrivilege = False

        # Step 2: Attempt to add the subject to an existing ACL entry if possible where
        #         the existing privilege in that entry matches our desired privilege.
        for acl in currentAcls:
            if acl.privilege == privilege:
                subjectSet = set(acl.subjects)
                subjectSet.update(targetSubjects)
                acl.subjects = list(subjectSet)
                addedPrivilege = True
                break

        # Step 3: If there isn't an existing entry to add to, make a new one.
        if (not(addedPrivilege)):
            if len(currentAcls) >= 3:
                raise ValueError(
                    f"Cannot add another ACL entry to grant privilege to existing count of {currentAcls} ACLs -- will exceed minimas!")

            currentAcls.append(Clusters.AccessControl.Structs.AccessControlEntry(privilege=privilege, authMode=Clusters.AccessControl.Enums.AuthMode.kCase,
                                                                                 subjects=targetSubjects))

    # Step 4: Prune ACLs which have empty subjects.
    currentAcls = [acl for acl in currentAcls if acl.subjects != NullValue and len(acl.subjects) != 0]

    logger.info(f'GrantPrivilege: Writing acls: {currentAcls}')
    await adminCtrl.WriteAttribute(targetNodeId, [(0, Clusters.AccessControl.Attributes.Acl(currentAcls))])


async def CreateControllersOnFabric(fabricAdmin: FabricAdmin, adminDevCtrl: ChipDeviceController, controllerNodeIds: typing.List[int], privilege: Clusters.AccessControl.Enums.Privilege, targetNodeId: int, catTags: typing.List[int] = []) -> typing.List[ChipDeviceController]:
    ''' Create new ChipDeviceController instances on a given fabric with a specific privilege on a target node.

        Args:
            fabricAdmin:                A FabricAdmin object that is capable of vending new controller instances on a fabric.
            adminDevCtrl:               An existing ChipDeviceController instance that already has admin privileges on the target node.
            controllerNodeIds:          List of desired nodeIds for the controllers.
            privilege:                  The specific ACL privilege to grant to the newly minted controllers.
            targetNodeId:               The Node ID of the target.
            catTags:                    CAT Tags to include in the NOC of controller, as well as when setting up the ACLs on the target.
    '''

    controllerList = []

    for nodeId in controllerNodeIds:
        newController = fabricAdmin.NewController(nodeId=nodeId, catTags=catTags)
        await GrantPrivilege(adminDevCtrl, newController, privilege, targetNodeId, catTags)
        controllerList.append(newController)

    return controllerList


async def AddNOCForNewFabricFromExisting(commissionerDevCtrl, newFabricDevCtrl, existingNodeId, newNodeId):
    ''' Perform sequence to commission new fabric using existing commissioned fabric.

    Args:
        commissionerDevCtrl (ChipDeviceController): Already commissioned device controller used
            to commission a new fabric on `newFabricDevCtrl`.
        newFabricDevCtrl (ChipDeviceController): New device controller which is used for the new
            fabric we are establishing.
        existingNodeId (int): Node ID of the target where an AddNOC needs to be done for a new fabric.
        newNodeId (int): Node ID to use for the target node on the new fabric.

    Return:
        bool: True if successful, False otherwise.

    '''
    resp = await commissionerDevCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(60))
    if resp.errorCode is not generalCommissioning.Enums.CommissioningError.kOk:
        return False

    csrForAddNOC = await commissionerDevCtrl.SendCommand(existingNodeId, 0, opCreds.Commands.CSRRequest(CSRNonce=os.urandom(32)))

    chainForAddNOC = newFabricDevCtrl.IssueNOCChain(csrForAddNOC, newNodeId)
    if chainForAddNOC.rcacBytes is None or chainForAddNOC.icacBytes is None or chainForAddNOC.nocBytes is None or chainForAddNOC.ipkBytes is None:
        # Expiring the failsafe timer in an attempt to clean up.
        await commissionerDevCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0))
        return False

    await commissionerDevCtrl.SendCommand(existingNodeId, 0, opCreds.Commands.AddTrustedRootCertificate(chainForAddNOC.rcacBytes))
    resp = await commissionerDevCtrl.SendCommand(existingNodeId, 0, opCreds.Commands.AddNOC(chainForAddNOC.nocBytes, chainForAddNOC.icacBytes, chainForAddNOC.ipkBytes, newFabricDevCtrl.nodeId, 0xFFF1))
    if resp.statusCode is not opCreds.Enums.OperationalCertStatus.kSuccess:
        # Expiring the failsafe timer in an attempt to clean up.
        await commissionerDevCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0))
        return False

    resp = await newFabricDevCtrl.SendCommand(newNodeId, 0, generalCommissioning.Commands.CommissioningComplete())
    if resp.errorCode is not generalCommissioning.Enums.CommissioningError.kOk:
        # Expiring the failsafe timer in an attempt to clean up.
        await commissionerDevCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0))
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
    resp = await devCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(600))
    if resp.errorCode is not generalCommissioning.Enums.CommissioningError.kOk:
        return False
    csrForUpdateNOC = await devCtrl.SendCommand(
        existingNodeId, 0, opCreds.Commands.CSRRequest(CSRNonce=os.urandom(32), isForUpdateNOC=True))
    chainForUpdateNOC = devCtrl.IssueNOCChain(csrForUpdateNOC, newNodeId)
    if chainForUpdateNOC.rcacBytes is None or chainForUpdateNOC.icacBytes is None or chainForUpdateNOC.nocBytes is None or chainForUpdateNOC.ipkBytes is None:
        await devCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0))
        return False

    resp = await devCtrl.SendCommand(existingNodeId, 0, opCreds.Commands.UpdateNOC(chainForUpdateNOC.nocBytes, chainForUpdateNOC.icacBytes))
    if resp.statusCode is not opCreds.Enums.OperationalCertStatus.kSuccess:
        # Expiring the failsafe timer in an attempt to clean up.
        await devCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0))
        return False

    # Forget our session since the peer deleted it
    devCtrl.ExpireSessions(existingNodeId)

    resp = await devCtrl.SendCommand(newNodeId, 0, generalCommissioning.Commands.CommissioningComplete())
    if resp.errorCode is not generalCommissioning.Enums.CommissioningError.kOk:
        # Expiring the failsafe timer in an attempt to clean up.
        await devCtrl.SendCommand(existingNodeId, 0, generalCommissioning.Commands.ArmFailSafe(0))
        return False

    if not await _IsNodeInFabricList(devCtrl, newNodeId):
        return False

    return True
