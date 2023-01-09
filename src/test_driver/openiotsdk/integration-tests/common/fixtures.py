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
import pathlib
import shutil
from time import sleep

import chip.CertificateAuthority
import chip.native
import pytest
from chip import ChipDeviceCtrl, exceptions
from chip.ChipStack import *

from .fvp_device import FvpDevice
from .telnet_connection import TelnetConnection

log = logging.getLogger(__name__)


@pytest.fixture(scope="session")
def rootDir():
    return pathlib.Path(__file__).parents[5].absolute()


@pytest.fixture(scope="session")
def fvp(request):
    if request.config.getoption('fvp'):
        return request.config.getoption('fvp')
    else:
        return shutil.which('FVP_Corstone_SSE-300_Ethos-U55')


@pytest.fixture(scope="session")
def fvpConfig(request, rootDir):
    if request.config.getoption('fvpConfig'):
        return request.config.getoption('fvpConfig')
    else:
        return os.path.join(rootDir, 'config/openiotsdk/fvp/cs300.conf')


@pytest.fixture(scope="session")
def telnetPort(request):
    return request.config.getoption('telnetPort')


@pytest.fixture(scope="session")
def networkInterface(request):
    if request.config.getoption('networkInterface'):
        return request.config.getoption('networkInterface')
    else:
        return None


@pytest.fixture(scope="function")
def device(fvp, fvpConfig, binaryPath, telnetPort, networkInterface):
    connection = TelnetConnection('localhost', telnetPort)
    device = FvpDevice(fvp, fvpConfig, binaryPath, connection, networkInterface, "FVPdev")
    device.start()
    yield device
    device.stop()


@pytest.fixture(scope="session")
def vendor_id():
    return 0xFFF1


@pytest.fixture(scope="session")
def fabric_id():
    return 1


@pytest.fixture(scope="session")
def node_id():
    return 1


@pytest.fixture(scope="function")
def controller(vendor_id, fabric_id, node_id):
    try:
        chip.native.Init()
        chipStack = chip.ChipStack.ChipStack(
            persistentStoragePath='/tmp/openiotsdk-test-storage.json', enableServerInteractions=False)
        certificateAuthorityManager = chip.CertificateAuthority.CertificateAuthorityManager(
            chipStack, chipStack.GetStorageManager())
        certificateAuthorityManager.LoadAuthoritiesFromStorage()
        if (len(certificateAuthorityManager.activeCaList) == 0):
            ca = certificateAuthorityManager.NewCertificateAuthority()
            ca.NewFabricAdmin(vendorId=vendor_id, fabricId=fabric_id)
        elif (len(certificateAuthorityManager.activeCaList[0].adminList) == 0):
            certificateAuthorityManager.activeCaList[0].NewFabricAdmin(vendorId=vendor_id, fabricId=fabric_id)

        caList = certificateAuthorityManager.activeCaList

        devCtrl = caList[0].adminList[0].NewController()

    except exceptions.ChipStackException as ex:
        log.error("Controller initialization failed {}".format(ex))
        return None
    except:
        log.error("Controller initialization failed")
        return None

    yield devCtrl
