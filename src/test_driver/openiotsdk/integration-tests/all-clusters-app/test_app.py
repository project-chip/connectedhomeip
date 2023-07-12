#
#    Copyright (c) 2023 Project CHIP Authors
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
import re

import chip.interaction_model
import pytest
from chip.clusters.Objects import AccessControl
from chip.clusters.Types import NullValue
from common.utils import (connect_device, disconnect_device, discover_device, get_setup_payload, read_zcl_attribute,
                          write_zcl_attribute)

log = logging.getLogger(__name__)


@pytest.fixture(scope="session")
def binaryPath(request, rootDir):
    if request.config.getoption('binaryPath'):
        return request.config.getoption('binaryPath')
    else:
        return os.path.join(rootDir, 'examples/all-clusters-app/openiotsdk/build/chip-openiotsdk-all-clusters-app-example.elf')


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
    ret = device.wait_for_output("Open IoT SDK all-clusters-app example application start")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("Open IoT SDK all-clusters-app example application run")
    assert ret is not None and len(ret) > 0


@pytest.mark.commissioningtest
def test_commissioning(device, controller):
    assert controller is not None
    devCtrl = controller

    ret = device.wait_for_output("Open IoT SDK all-clusters-app example application start")
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


ALL_CLUSTERS_CTRL_TEST_ENDPOINT_ID = 0


@pytest.mark.ctrltest
def test_clusters_ctrl(device, controller):
    assert controller is not None
    devCtrl = controller

    ret = device.wait_for_output("Open IoT SDK all-clusters-app example application start")
    assert ret is not None and len(ret) > 0

    setupPayload = get_setup_payload(device)
    assert setupPayload is not None

    ret = device.wait_for_output("Current software version")
    assert ret is not None and len(ret) > 1

    version_app = ret[-1].split()[-2:]

    commissionable_device = discover_device(devCtrl, setupPayload)
    assert commissionable_device is not None

    nodeId = connect_device(devCtrl, setupPayload, commissionable_device)
    assert nodeId is not None

    ret = device.wait_for_output("Commissioning completed successfully")
    assert ret is not None and len(ret) > 0

    acs = AccessControl.Structs()
    ace = AccessControl.Enums()

    #  AccessControl cluster test - write entires
    entry0 = acs.AccessControlEntryStruct(privilege=ace.AccessControlEntryPrivilegeEnum.kAdminister, authMode=ace.AccessControlEntryAuthModeEnum.kCase, subjects=NullValue, targets=[acs.AccessControlTargetStruct(
        cluster=NullValue, endpoint=0, deviceType=NullValue), acs.AccessControlTargetStruct(cluster=1, endpoint=NullValue, deviceType=NullValue), acs.AccessControlTargetStruct(cluster=2, endpoint=3, deviceType=NullValue)], fabricIndex=1)
    entry1 = acs.AccessControlEntryStruct(privilege=ace.AccessControlEntryPrivilegeEnum.kView, authMode=ace.AccessControlEntryAuthModeEnum.kCase, subjects=[4, 5, 6, 7], targets=[acs.AccessControlTargetStruct(
        cluster=NullValue, endpoint=8, deviceType=NullValue), acs.AccessControlTargetStruct(cluster=9, endpoint=NullValue, deviceType=NullValue), acs.AccessControlTargetStruct(cluster=10, endpoint=11, deviceType=NullValue)], fabricIndex=1)
    entry2 = acs.AccessControlEntryStruct(privilege=ace.AccessControlEntryPrivilegeEnum.kOperate, authMode=ace.AccessControlEntryAuthModeEnum.kGroup, subjects=[12, 13, 14, 15], targets=[acs.AccessControlTargetStruct(
        cluster=NullValue, endpoint=16, deviceType=NullValue), acs.AccessControlTargetStruct(cluster=17, endpoint=NullValue, deviceType=NullValue), acs.AccessControlTargetStruct(cluster=18, endpoint=19, deviceType=NullValue)], fabricIndex=1)
    entry3 = acs.AccessControlEntryStruct(privilege=ace.AccessControlEntryPrivilegeEnum.kOperate, authMode=ace.AccessControlEntryAuthModeEnum.kCase, subjects=[20, 21, 22, 23], targets=[acs.AccessControlTargetStruct(
        cluster=NullValue, endpoint=24, deviceType=NullValue), acs.AccessControlTargetStruct(cluster=25, endpoint=NullValue, deviceType=NullValue), acs.AccessControlTargetStruct(cluster=26, endpoint=27, deviceType=NullValue)], fabricIndex=1)
    err, res = write_zcl_attribute(devCtrl, "AccessControl", "Acl", nodeId,  ALL_CLUSTERS_CTRL_TEST_ENDPOINT_ID,
                                   [entry0, entry1, entry2, entry3])
    assert err == 0
    assert res[0].Status == chip.interaction_model.Status.Success

    err, res = read_zcl_attribute(devCtrl, "AccessControl", "Acl", nodeId, ALL_CLUSTERS_CTRL_TEST_ENDPOINT_ID)
    assert err == 0
    assert res.value[0] == entry0
    assert res.value[1] == entry1
    assert res.value[2] == entry2
    assert res.value[3] == entry3

    #  BasicInformation cluster test - check vendor ID
    err, res = read_zcl_attribute(devCtrl, "BasicInformation", "VendorID", nodeId, ALL_CLUSTERS_CTRL_TEST_ENDPOINT_ID)
    assert err == 0
    assert res.value == int(setupPayload.attributes['VendorID'])

    #  BasicInformation cluster test - check product ID
    err, res = read_zcl_attribute(devCtrl, "BasicInformation", "ProductID", nodeId, ALL_CLUSTERS_CTRL_TEST_ENDPOINT_ID)
    assert err == 0
    assert res.value == int(setupPayload.attributes['ProductID'])

    #  BasicInformation cluster test - check software version
    err, res = read_zcl_attribute(devCtrl, "BasicInformation", "SoftwareVersion", nodeId, ALL_CLUSTERS_CTRL_TEST_ENDPOINT_ID)
    assert err == 0
    assert res.value == int(re.sub(r"[\[\]]", "", version_app[0]))

    #  BasicInformation cluster test - check software version string
    err, res = read_zcl_attribute(devCtrl, "BasicInformation", "SoftwareVersionString", nodeId, ALL_CLUSTERS_CTRL_TEST_ENDPOINT_ID)
    assert err == 0
    assert res.value == version_app[1]
