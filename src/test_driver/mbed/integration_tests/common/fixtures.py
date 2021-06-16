# Copyright (c) 2009-2020 Arm Limited
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from typing import List, Optional, Any, Mapping

import mbed_lstools
import pytest
from time import sleep
from mbed_flasher.flash import Flash

from .device import Device

from .serial_connection import SerialConnection
from .serial_device import SerialDevice
from .utils import is_network_visible

from chip import ChipDeviceCtrl

import logging
log = logging.getLogger(__name__)


@pytest.fixture(scope="session")
def platforms(request):
    if request.config.getoption('platforms'):
        return request.config.getoption('platforms').split(',')
    else:
        return [
            'DISCO_L475VG_IOT01A',
            'CY8CPROTO_062_4343W'
        ]


@pytest.fixture(scope="session")
def binaries(request):
    if request.config.getoption('binaries'):
        platform_and_binaries = request.config.getoption('binaries').split(',')
        result = {}
        for pb in platform_and_binaries:
            pb = pb.split(':')
            print(pb)
            result[pb[0]] = pb[1]
        return result
    return {}


@pytest.fixture(scope="session")
def serial_inter_byte_delay(request):
    if request.config.getoption('serial_inter_byte_delay'):
        return float(request.config.getoption('serial_inter_byte_delay'))
    return None


@pytest.fixture(scope="session")
def serial_baudrate(request):
    if request.config.getoption('serial_baudrate'):
        return int(request.config.getoption('serial_baudrate'))
    return 115200

@pytest.fixture(scope="session")
def network(request):
    if request.config.getoption('network'):
        credentials = request.config.getoption('network')
        params = credentials.split(':')
        return (params[0], params[1])
    return None

@pytest.fixture(scope="session")
def echo_server(request):
    if request.config.getoption('echo_server'):
        echo_server = request.config.getoption('echo_server')
        params = echo_server.split(':')
        return (params[0], params[1])
    return None

@pytest.fixture(scope="session")
def chip_tools_dir(request):
    if request.config.getoption('chip_tools_dir'):
        chip_tools_dir = request.config.getoption('chip_tools_dir')
        return chip_tools_dir
    return None


class BoardAllocation:
    def __init__(self, description: Mapping[str, Any]):
        self.description = description
        self.device = None
        self.flashed = False


class BoardAllocator:
    def __init__(self, platforms_supported: List[str], binaries: Mapping[str, str], serial_inter_byte_delay: float, baudrate: int):
        mbed_ls = mbed_lstools.create()
        boards = mbed_ls.list_mbeds(filter_function=lambda m: m['platform_name'] in platforms_supported)
        self.board_description = boards
        self.binaries = binaries
        self.allocation = []
        self.flasher = None
        self.serial_inter_byte_delay = serial_inter_byte_delay
        self.baudrate = baudrate
        for desc in boards:
            self.allocation.append(BoardAllocation(desc))

    def allocate(self, name: str = None):
        for alloc in self.allocation:
            if alloc.device is None:

                # Flash if a binary is provided and the board hasn't been flashed yet
                platform = alloc.description['platform_name']
                log.info('Start {} board allocation'.format(platform))
                binary = self.binaries.get(platform)
                if alloc.flashed is False and binary:
                    if self.flasher is None:
                        self.flasher = Flash(log)
                    log.info('Flash {} board with {} ...'.format(platform, binary))
                    self.flasher.flash(build=binary, target_id=alloc.description["target_id"])
                    alloc.flashed = True
                    log.info('Flashing completed')

                # Create the serial connection
                connection = SerialConnection(
                    port=alloc.description["serial_port"],
                    baudrate=self.baudrate,
                    inter_byte_delay=self.serial_inter_byte_delay
                )
                connection.open()

                # Create the serial device
                alloc.device = SerialDevice(connection, name)
                alloc.device.reset(duration=1)
                alloc.device.flush(1)

                sleep(2)

                log.info('Allocate {} board as serial device'.format(platform))

                return alloc.device
        return None

    def release(self, device: Device) -> None:
        for alloc in self.allocation:
            if alloc.device == device and alloc.device is not None:

                # Stop activities
                alloc.device.stop()
                alloc.device.serial.close()

                # Cleanup
                alloc.device = None

                log.info('Release {} board'.format(alloc.description['platform_name']))


@pytest.fixture(scope="session")
def board_allocator(
        platforms: List[str],
        binaries: Mapping[str, str],
        serial_inter_byte_delay: float,
        serial_baudrate: int,
):
    yield BoardAllocator(platforms, binaries, serial_inter_byte_delay, serial_baudrate)


@pytest.fixture(scope="function")
def device(board_allocator):
    device = board_allocator.allocate(name='DUT')
    yield device
    board_allocator.release(device)

@pytest.fixture(scope="function")
def connected_device(device, network):
    network_ssid = network[0]
    network_pass = network[1]

    # Check connection status
    ret = device.send(command="device sta connected", expected_output="Done")
    assert ret != None and len(ret) > 1
    device_connected = (ret[-2].rstrip().lower() == "true")

    if not device_connected:
        # Check if device support STA mode
        ret = device.send(command="device sta mode", expected_output="Done")
        assert ret != None and len(ret) > 1
        assert "NotSupported" != ret[-2].rstrip()

        # Check if network is visible
        ret = device.send(command="device scan", expected_output="Done", wait_before_read=5)
        assert ret != None and len(ret) > 1
        assert is_network_visible(ret, network_ssid)

        # Connect to network
        ret = device.send(command="device connect {} {}".format(network_ssid, network_pass), expected_output="Done")
        assert ret != None

        ret = device.wait_for_output("Internet connectivity ESTABLISHED")
        assert ret != None

        # Check connection status
        ret = device.send(command="device sta connected", expected_output="Done")
        assert ret != None and len(ret) > 1
        assert ret[-2].rstrip().lower() == "true"

    yield device

    if not device_connected:
        # Network disconnect
        ret = device.send(command="device sta clear_provision", expected_output="Done")
        assert ret != None

        ret = device.wait_for_output("Internet connectivity LOST")
        assert ret != None

        # Check connection status
        ret = device.send(command="device sta connected", expected_output="Done")
        assert ret != None and len(ret) > 1
        assert ret[-2].rstrip().lower() == "false"


@pytest.fixture(scope="function")
def device_controller():
    devCtrl = ChipDeviceCtrl.ChipDeviceController()
    yield devCtrl
    devCtrl.__del__