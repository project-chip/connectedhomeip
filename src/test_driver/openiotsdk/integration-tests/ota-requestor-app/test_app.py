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
from chip.clusters.Objects import OtaSoftwareUpdateRequestor
from chip.clusters.Types import NullValue
from common.utils import (connect_device, disconnect_device, discover_device, get_setup_payload, send_zcl_command,
                          write_zcl_attribute)

log = logging.getLogger(__name__)


@pytest.fixture(scope="session")
def binaryPath(request, rootDir):
    if request.config.getoption('binaryPath'):
        return request.config.getoption('binaryPath')
    else:
        return os.path.join(rootDir, 'examples/ota-requestor-app/openiotsdk/build/chip-openiotsdk-ota-requestor-app-example.elf')


@pytest.fixture(scope="session")
def updateBinaryPath(request, rootDir):
    if request.config.getoption('updateBinaryPath'):
        return request.config.getoption('updateBinaryPath')
    else:
        return os.path.join(rootDir, 'examples/ota-requestor-app/openiotsdk/build/chip-openiotsdk-ota-requestor-app-example.ota')


@pytest.fixture(scope="session")
def controllerConfig(request):
    config = {
        'vendorId': 0xFFF1,
        'fabricId': 1,
        'persistentStoragePath': '/tmp/openiotsdk-test-storage.json'
    }
    return config


@pytest.fixture(scope="session")
def otaProviderConfig(request, updateBinaryPath):
    config = {
        'discriminator': '3841',
        'port': '5580',
        'filePath': f'{updateBinaryPath}',
        'persistentStoragePath': '/tmp/openiotsdk-test-ota-provider.json'
    }
    return config


@pytest.mark.smoketest
def test_smoke_test(device):
    ret = device.wait_for_output("Open IoT SDK ota-requestor-app example application start")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("Open IoT SDK ota-requestor-app example application run")
    assert ret is not None and len(ret) > 0


@pytest.mark.commissioningtest
def test_commissioning(device, controller):
    assert controller is not None
    devCtrl = controller

    ret = device.wait_for_output("Open IoT SDK ota-requestor-app example application start")
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


OTA_REQUESTOR_CTRL_TEST_ENDPOINT_ID = 0


@pytest.mark.ctrltest
def test_update_ctrl(device, controller, ota_provider, softwareVersion):
    assert controller is not None
    devCtrl = controller
    version_number, version_str = softwareVersion

    log.info("Setup OTA provider...")

    # Get OTA provider setup payload
    setupPayloadProvider = get_setup_payload(ota_provider)
    assert setupPayloadProvider is not None

    # Discover and commission the OTA provider
    commissionable_provider_device = discover_device(devCtrl, setupPayloadProvider)
    assert commissionable_provider_device is not None

    providerNodeId = connect_device(devCtrl, setupPayloadProvider, commissionable_provider_device)
    assert providerNodeId is not None

    ret = ota_provider.wait_for_output("Commissioning completed successfully")
    assert ret is not None and len(ret) > 0

    log.info("OTA provider ready")
    log.info("Setup OTA requestor...")

    # Get OTA requestor setup payload
    setupPayload = get_setup_payload(device)
    assert setupPayload is not None

    # Discover and commission the OTA requestor
    commissionable_requestor_device = discover_device(devCtrl, setupPayload)
    assert commissionable_requestor_device is not None

    requestorNodeId = connect_device(devCtrl, setupPayload, commissionable_requestor_device)
    assert requestorNodeId is not None

    ret = device.wait_for_output("Commissioning completed successfully")
    assert ret is not None and len(ret) > 0

    log.info("OTA requestor ready")
    log.info("Install ACL entries")

    #  Install necessary ACL entries in OTA provider to enable access by OTA requestor
    err, res = write_zcl_attribute(devCtrl, "AccessControl", "Acl", providerNodeId,  OTA_REQUESTOR_CTRL_TEST_ENDPOINT_ID,
                                   [{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [requestorNodeId], "targets": NullValue},
                                    {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": NullValue, "targets": [{"cluster": 41, "endpoint": NullValue, "deviceType": NullValue}]}])
    assert err == 0
    assert res[0].Status == chip.interaction_model.Status.Success

    ota_provider.set_verbose(False)

    log.info("Announce the OTA provider and start the firmware update process")

    # Announce the OTA provider and start the firmware update process
    err, res = send_zcl_command(devCtrl, "OtaSoftwareUpdateRequestor", "AnnounceOTAProvider", requestorNodeId, OTA_REQUESTOR_CTRL_TEST_ENDPOINT_ID,
                                dict(providerNodeID=providerNodeId, vendorID=int(setupPayloadProvider.attributes['VendorID']),
                                     announcementReason=OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUrgentUpdateAvailable,
                                     metadataForNode=None, endpoint=0))

    ret = device.wait_for_output("New version of the software is available")
    assert ret is not None and len(ret) > 1

    version = ret[-1].split()[-1]
    assert version_number == version

    device.set_verbose(False)

    log.info("New software image downloading and installing...")

    ret = device.wait_for_output("Open IoT SDK ota-requestor-app example application start", timeout=1200)
    assert ret is not None and len(ret) > 0

    device.set_verbose(True)

    ret = device.wait_for_output("Current software version")
    assert ret is not None and len(ret) > 1

    version_app = ret[-1].split()[-2:]
    assert version_number == re.sub(r"[\[\]]", "", version_app[0])
    assert version_str == version_app[1]

    assert disconnect_device(devCtrl, requestorNodeId)
    assert disconnect_device(devCtrl, providerNodeId)
