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
import os

import pytest
from chip.clusters.Objects import DoorLock
from chip.clusters.Types import NullValue
from common.utils import connect_device, disconnect_device, discover_device, get_setup_payload, read_zcl_attribute, send_zcl_command

log = logging.getLogger(__name__)


@pytest.fixture(scope="session")
def binaryPath(request, rootDir):
    if request.config.getoption('binaryPath'):
        return request.config.getoption('binaryPath')
    else:
        return os.path.join(rootDir, 'examples/lock-app/openiotsdk/build/chip-openiotsdk-lock-app-example.elf')


@pytest.fixture(scope="session")
def controllerConfig(request):
    config = {
        'vendorId': 0xFFF1,
        'fabricId': 1,
        'persistentStoragePath': '/tmp/openiotsdk-test-storage.json'
    }
    return config


@pytest.mark.smoketest
def test_smoke_test(device):
    ret = device.wait_for_output("Open IoT SDK lock-app example application start")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("Open IoT SDK lock-app example application run")
    assert ret is not None and len(ret) > 0


@pytest.mark.commissioningtest
def test_commissioning(device, controller):
    assert controller is not None
    devCtrl = controller

    ret = device.wait_for_output("Open IoT SDK lock-app example application start")
    assert ret is not None and len(ret) > 0

    setupPayload = get_setup_payload(device)
    assert setupPayload is not None

    commissionable_device = discover_device(devCtrl, setupPayload)
    assert commissionable_device is not None

    assert commissionable_device.vendorId == int(setupPayload.attributes['VendorID'])
    assert commissionable_device.productId == int(setupPayload.attributes['ProductID'])
    assert commissionable_device.addresses[0] is not None

    nodeId = connect_device(devCtrl, setupPayload, commissionable_device)
    assert nodeId is not None
    log.info("Device {} connected".format(commissionable_device.addresses[0]))

    ret = device.wait_for_output("Commissioning completed successfully")
    assert ret is not None and len(ret) > 0

    assert disconnect_device(devCtrl, nodeId)


LOCK_CTRL_TEST_PIN_CODE = b"123456"
LOCK_CTRL_TEST_USER_INDEX = 1
LOCK_CTRL_TEST_ENDPOINT_ID = 1
LOCK_CTRL_TEST_USER_NAME = 'testUser'
LOCK_CTRL_TEST_CREDENTIAL_INDEX = 1


@pytest.mark.ctrltest
def test_lock_ctrl(device, controller):
    assert controller is not None
    devCtrl = controller

    ret = device.wait_for_output("Open IoT SDK lock-app example application start")
    assert ret is not None and len(ret) > 0

    setupPayload = get_setup_payload(device)
    assert setupPayload is not None

    commissionable_device = discover_device(devCtrl, setupPayload)
    assert commissionable_device is not None

    nodeId = connect_device(devCtrl, setupPayload, commissionable_device)
    assert nodeId is not None

    ret = device.wait_for_output("Commissioning completed successfully")
    assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(devCtrl, "DoorLock", "SetUser", nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                dict(operationType=DoorLock.Enums.DataOperationTypeEnum.kAdd,
                                     userIndex=LOCK_CTRL_TEST_USER_INDEX,
                                     userName=LOCK_CTRL_TEST_USER_NAME,
                                     userUniqueID=LOCK_CTRL_TEST_USER_INDEX,
                                     userStatus=DoorLock.Enums.UserStatusEnum.kOccupiedEnabled,
                                     userType=DoorLock.Enums.UserTypeEnum.kUnrestrictedUser,
                                     credentialRule=DoorLock.Enums.CredentialRuleEnum.kSingle),
                                requestTimeoutMs=1000)
    assert err == 0

    ret = device.wait_for_output("Successfully set the user [mEndpointId={},index={},adjustedIndex=0]".format(
        LOCK_CTRL_TEST_ENDPOINT_ID,
        LOCK_CTRL_TEST_USER_INDEX))
    assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(devCtrl, "DoorLock", "GetUser", nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                dict(userIndex=LOCK_CTRL_TEST_USER_INDEX),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.userIndex == LOCK_CTRL_TEST_USER_INDEX
    assert res.userName == LOCK_CTRL_TEST_USER_NAME
    assert res.userUniqueID == LOCK_CTRL_TEST_USER_INDEX
    assert res.userStatus == DoorLock.Enums.UserStatusEnum.kOccupiedEnabled
    assert res.userType == DoorLock.Enums.UserTypeEnum.kUnrestrictedUser
    assert res.credentialRule == DoorLock.Enums.CredentialRuleEnum.kSingle

    err, res = send_zcl_command(devCtrl, "DoorLock", "SetCredential", nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                dict(operationType=DoorLock.Enums.DataOperationTypeEnum.kAdd,
                                     credential=DoorLock.Structs.CredentialStruct(
                                         credentialType=DoorLock.Enums.CredentialTypeEnum.kPin, credentialIndex=LOCK_CTRL_TEST_CREDENTIAL_INDEX),
                                     credentialData=LOCK_CTRL_TEST_PIN_CODE,
                                     userIndex=LOCK_CTRL_TEST_USER_INDEX,
                                     userStatus=NullValue,
                                     userType=NullValue),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.status == DoorLock.Enums.DlStatus.kSuccess

    ret = device.wait_for_output("Successfully set the credential [mEndpointId={},index={},"
                                 "credentialType={},creator=1,modifier=1]".format(
                                     LOCK_CTRL_TEST_ENDPOINT_ID,
                                     LOCK_CTRL_TEST_USER_INDEX,
                                     DoorLock.Enums.CredentialTypeEnum.kPin
                                 ))
    assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(devCtrl, "DoorLock", "GetCredentialStatus", nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                dict(credential=DoorLock.Structs.CredentialStruct(
                                    credentialType=DoorLock.Enums.CredentialTypeEnum.kPin, credentialIndex=LOCK_CTRL_TEST_CREDENTIAL_INDEX)),
                                requestTimeoutMs=1000)
    assert err == 0
    assert res.credentialExists
    assert res.userIndex == LOCK_CTRL_TEST_USER_INDEX

    err, res = send_zcl_command(devCtrl, "DoorLock", "LockDoor",  nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                dict(PINCode=LOCK_CTRL_TEST_PIN_CODE),
                                requestTimeoutMs=1000)
    assert err == 0

    ret = device.wait_for_output("setting door lock state to \"Locked\"")
    assert ret is not None and len(ret) > 0

    err, res = read_zcl_attribute(devCtrl, "DoorLock", "LockState", nodeId, LOCK_CTRL_TEST_ENDPOINT_ID)
    assert err == 0
    assert res.value == DoorLock.Enums.DlLockState.kLocked

    err, res = send_zcl_command(devCtrl, "DoorLock", "UnlockDoor",  nodeId, LOCK_CTRL_TEST_ENDPOINT_ID,
                                dict(PINCode=LOCK_CTRL_TEST_PIN_CODE),
                                requestTimeoutMs=1000)
    assert err == 0

    ret = device.wait_for_output("setting door lock state to \"Unlocked\"")
    assert ret is not None and len(ret) > 0

    err, res = read_zcl_attribute(devCtrl, "DoorLock", "LockState", nodeId, LOCK_CTRL_TEST_ENDPOINT_ID)
    assert err == 0
    assert res.value == DoorLock.Enums.DlLockState.kUnlocked

    assert disconnect_device(devCtrl, nodeId)
