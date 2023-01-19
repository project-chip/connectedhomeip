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

import logging
from time import sleep

import pytest
from chip.clusters.Objects import DoorLock
from common.utils import *

log = logging.getLogger(__name__)


@pytest.fixture(scope="session")
def binaryPath(request, rootDir):
    if request.config.getoption('binaryPath'):
        return request.config.getoption('binaryPath')
    else:
        return os.path.join(rootDir, 'examples/lock-app/openiotsdk/build/chip-openiotsdk-lock-app-example.elf')


@pytest.mark.smoketest
def test_smoke_test(device):
    ret = device.wait_for_output("Open IoT SDK lock-app example application start")
    assert ret != None and len(ret) > 0
    ret = device.wait_for_output("Open IoT SDK lock-app example application run")
    assert ret != None and len(ret) > 0


@pytest.mark.commissioningtest
def test_commissioning(device, controller):
    assert controller != None
    devCtrl = controller

    setupPayload = get_setup_payload(device)
    assert setupPayload != None

    commissionable_device = discover_device(devCtrl, setupPayload)
    assert commissionable_device != None

    assert commissionable_device.vendorId == int(setupPayload.attributes['VendorID'])
    assert commissionable_device.productId == int(setupPayload.attributes['ProductID'])
    assert commissionable_device.addresses[0] != None

    nodeId = connect_device(setupPayload, commissionable_device)
    assert nodeId != None
    log.info("Device {} connected".format(commissionable_device.addresses[0]))

    ret = device.wait_for_output("Commissioning completed successfully", timeout=30)
    assert ret != None and len(ret) > 0

    assert disconnect_device(devCtrl, nodeId)


LOCK_CTRL_TEST_PIN_CODE = 12345
LOCK_CTRL_TEST_USER_INDEX = 1
LOCK_CTRL_TEST_ENDPOINT_ID = 1
LOCK_CTRL_TEST_USER_NAME = 'testUser'
LOCK_CTRL_TEST_CREDENTIAL_INDEX = 1


@pytest.mark.ctrltest
def test_lock_ctrl(device, controller):
    assert controller != None
    devCtrl = controller

    setupPayload = get_setup_payload(device)
    assert setupPayload != None

    commissionable_device = discover_device(devCtrl, setupPayload)
    assert commissionable_device != None

    nodeId = connect_device(setupPayload, commissionable_device)
    assert nodeId != None

    ret = device.wait_for_output("Commissioning completed successfully", timeout=30)
    assert ret != None and len(ret) > 0

    err, res = send_zcl_command(
        devCtrl, "DoorLock SetUser {} {} operationType={} userIndex={} userName={} userUniqueId={} "
        "userStatus={} userType={} credentialRule={}".format(nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                                             DoorLock.Enums.DlDataOperationType.kAdd,
                                                             LOCK_CTRL_TEST_USER_INDEX,
                                                             LOCK_CTRL_TEST_USER_NAME,
                                                             LOCK_CTRL_TEST_USER_INDEX,
                                                             DoorLock.Enums.DlUserStatus.kOccupiedEnabled,
                                                             DoorLock.Enums.DlUserType.kUnrestrictedUser,
                                                             DoorLock.Enums.DlCredentialRule.kSingle), requestTimeoutMs=1000)
    assert err == 0

    ret = device.wait_for_output("Successfully set the user [mEndpointId={},index={},adjustedIndex=0]".format(
        LOCK_CTRL_TEST_ENDPOINT_ID,
        LOCK_CTRL_TEST_USER_INDEX))
    assert ret != None and len(ret) > 0

    err, res = send_zcl_command(
        devCtrl, "DoorLock GetUser {} {} userIndex={}".format(nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                                              LOCK_CTRL_TEST_USER_INDEX))
    assert err == 0
    assert res.userIndex == LOCK_CTRL_TEST_USER_INDEX
    assert res.userName == LOCK_CTRL_TEST_USER_NAME
    assert res.userUniqueID == LOCK_CTRL_TEST_USER_INDEX
    assert res.userStatus == DoorLock.Enums.DlUserStatus.kOccupiedEnabled
    assert res.userType == DoorLock.Enums.DlUserType.kUnrestrictedUser
    assert res.credentialRule == DoorLock.Enums.DlCredentialRule.kSingle

    err, res = send_zcl_command(
        devCtrl, "DoorLock SetCredential {} {} operationType={} "
        "credential=struct:DlCredential(credentialType={},credentialIndex={}) credentialData=str:{} "
        "userIndex={} userStatus={} userType={}".format(nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                                        DoorLock.Enums.DlDataOperationType.kAdd,
                                                        DoorLock.Enums.DlCredentialType.kPin,
                                                        LOCK_CTRL_TEST_CREDENTIAL_INDEX,
                                                        LOCK_CTRL_TEST_PIN_CODE,
                                                        LOCK_CTRL_TEST_USER_INDEX,
                                                        DoorLock.Enums.DlUserStatus.kOccupiedEnabled,
                                                        DoorLock.Enums.DlUserType.kUnrestrictedUser), requestTimeoutMs=1000)
    assert err == 0
    assert res.status == DoorLock.Enums.DlStatus.kSuccess

    ret = device.wait_for_output("Successfully set the credential [mEndpointId={},index={},"
                                 "credentialType={},creator=1,modifier=1]".format(LOCK_CTRL_TEST_ENDPOINT_ID,
                                                                                  LOCK_CTRL_TEST_USER_INDEX, DoorLock.Enums.DlCredentialType.kPin))
    assert ret != None and len(ret) > 0

    err, res = send_zcl_command(
        devCtrl, "DoorLock GetCredentialStatus {} {} credential=struct:DlCredential(credentialType={},"
        "credentialIndex={})".format(nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                     DoorLock.Enums.DlCredentialType.kPin,
                                     LOCK_CTRL_TEST_CREDENTIAL_INDEX), requestTimeoutMs=1000)
    assert err == 0
    assert res.credentialExists
    assert res.userIndex == LOCK_CTRL_TEST_USER_INDEX

    err, res = send_zcl_command(
        devCtrl, "DoorLock LockDoor {} {} pinCode=str:{}".format(nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                                                 LOCK_CTRL_TEST_PIN_CODE), requestTimeoutMs=1000)
    assert err == 0

    ret = device.wait_for_output("Setting door lock state to \"Locked\" [endpointId={}]".format(LOCK_CTRL_TEST_ENDPOINT_ID))
    assert ret != None and len(ret) > 0

    err, res = read_zcl_attribute(
        devCtrl, "DoorLock LockState {} {}".format(nodeId, LOCK_CTRL_TEST_ENDPOINT_ID))
    assert err == 0
    assert res.value == DoorLock.Enums.DlLockState.kLocked

    err, res = send_zcl_command(
        devCtrl, "DoorLock UnlockDoor {} {} pinCode=str:{}".format(nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                                                   LOCK_CTRL_TEST_PIN_CODE), requestTimeoutMs=1000)
    assert err == 0

    ret = device.wait_for_output("Setting door lock state to \"Unlocked\" [endpointId={}]".format(LOCK_CTRL_TEST_ENDPOINT_ID))
    assert ret != None and len(ret) > 0

    err, res = read_zcl_attribute(
        devCtrl, "DoorLock LockState {} {}".format(nodeId, LOCK_CTRL_TEST_ENDPOINT_ID))
    assert err == 0
    assert res.value == DoorLock.Enums.DlLockState.kUnlocked

    assert disconnect_device(devCtrl, nodeId)
