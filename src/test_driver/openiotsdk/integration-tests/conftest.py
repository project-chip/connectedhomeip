#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
