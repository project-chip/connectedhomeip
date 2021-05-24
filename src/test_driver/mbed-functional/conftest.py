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

import pytest

pytest_plugins = ['common.fixtures']


def pytest_addoption(parser):
    """
    Function for pytest to enable own custom commandline arguments
    :param parser: argparser
    :return:
    """
    parser.addoption('--platforms', action='store', help='List of platforms that can be used to run the tests. Platforms are separated by a comma')
    parser.addoption('--binaries', action='store', help='Platform and associated binary in the form platform:binary. Multiple values are separated by a comma')
    parser.addoption('--serial_inter_byte_delay', action='store', help='Time in second between two bytes sent on the serial line (accepts floats)')
    parser.addoption('--serial_baudrate', action='store', help='Baudrate of the serial port used', default='115200')
    parser.addoption('--network', action='store', help='WiFi network credentials to which we want to connect device. Format network_ssid:network_password')
    parser.addoption('--echo_server', action='store', help='Address and port of echo server. Format server_ip:server_port')
    parser.addoption('--chip_tools_dir', action='store', help='Path to CHIP tools directory')