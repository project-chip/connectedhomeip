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

pytest_plugins = ['common.fixtures']


def pytest_addoption(parser):
    """
    Function for pytest to enable own custom commandline arguments
    :param parser: argparser
    :return:
    """
    parser.addoption('--binaryPath', action='store',
                     help='Application binary path')
    parser.addoption('--fvp', action='store',
                     help='FVP instance path')
    parser.addoption('--fvpConfig', action='store',
                     help='FVP configuration file path')
    parser.addoption('--telnetPort', action='store',
                     help='Telnet terminal port number.', default="5000")
    parser.addoption('--networkInterface', action='store',
                     help='FVP network interface name')
    parser.addoption('--updateBinaryPath', action='store',
                     help='Application update binary path')
    parser.addoption('--otaProvider', action='store',
                     help='Path to OTA provider application')
    parser.addoption('--softwareVersion', action='store',
                     help='Software version of update image in the format <number>:<x.x.x> eg. 1:0.0.01')
