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

import chip.CertificateAuthority
import chip.native
import pytest
from chip import exceptions

from .fvp_device import FvpDevice
from .telnet_connection import TelnetConnection
from .terminal_device import TerminalDevice

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


@pytest.fixture(scope="session")
def otaProvider(request, rootDir):
    if request.config.getoption('otaProvider'):
        return request.config.getoption('otaProvider')
    else:
        return os.path.join(rootDir, 'out/chip-ota-provider-app')


@pytest.fixture(scope="session")
def softwareVersion(request):
    if request.config.getoption('softwareVersion'):
        version = request.config.getoption('softwareVersion')
        params = version.split(':')
        return (params[0], params[1])
    else:
        return ("1", "0.0.1")


@pytest.fixture(scope="function")
def device(fvp, fvpConfig, binaryPath, telnetPort, networkInterface):
    connection = TelnetConnection('localhost', telnetPort)
    device = FvpDevice(fvp, fvpConfig, binaryPath, connection, networkInterface, "FVPdev")
    device.start()
    yield device
    device.stop()


@pytest.fixture(scope="session")
def controller(controllerConfig):
    try:
        chip.native.Init()
        chipStack = chip.ChipStack.ChipStack(
            persistentStoragePath=controllerConfig['persistentStoragePath'], enableServerInteractions=False)
        certificateAuthorityManager = chip.CertificateAuthority.CertificateAuthorityManager(
            chipStack, chipStack.GetStorageManager())
        certificateAuthorityManager.LoadAuthoritiesFromStorage()
        if (len(certificateAuthorityManager.activeCaList) == 0):
            ca = certificateAuthorityManager.NewCertificateAuthority()
            ca.NewFabricAdmin(vendorId=controllerConfig['vendorId'], fabricId=controllerConfig['fabricId'])
        elif (len(certificateAuthorityManager.activeCaList[0].adminList) == 0):
            certificateAuthorityManager.activeCaList[0].NewFabricAdmin(
                vendorId=controllerConfig['vendorId'], fabricId=controllerConfig['fabricId'])

        caList = certificateAuthorityManager.activeCaList

        devCtrl = caList[0].adminList[0].NewController()

    except exceptions.ChipStackException as ex:
        log.error("Controller initialization failed {}".format(ex))
        return None
    except Exception:
        log.error("Controller initialization failed")
        return None

    yield devCtrl

    devCtrl.Shutdown()
    certificateAuthorityManager.Shutdown()
    chipStack.Shutdown()
    os.remove(controllerConfig['persistentStoragePath'])


@pytest.fixture(scope="session")
def ota_provider(otaProvider, otaProviderConfig):
    args = [
        '--discriminator', otaProviderConfig['discriminator'],
        '--secured-device-port',  otaProviderConfig['port'],
        '-c',
        '--KVS', otaProviderConfig['persistentStoragePath'],
        '--filepath', otaProviderConfig['filePath'],
    ]

    device = TerminalDevice(otaProvider, args, "OTAprovider")
    device.start()

    yield device

    device.stop()
    os.remove(otaProviderConfig['persistentStoragePath'])
