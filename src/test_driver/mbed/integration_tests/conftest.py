# SPDX-FileCopyrightText: 2009-2021 Arm Limited
# SPDX-License-Identifier: Apache-2.0

pytest_plugins = ['common.fixtures']


def pytest_addoption(parser):
    """
    Function for pytest to enable own custom commandline arguments
    :param parser: argparser
    :return:
    """
    parser.addoption('--platforms', action='store',
                     help='List of platforms that can be used to run the tests. Platforms are separated by a comma')
    parser.addoption('--serial_inter_byte_delay', action='store',
                     help='Time in second between two bytes sent on the serial line (accepts floats)')
    parser.addoption('--serial_baudrate', action='store',
                     help='Baudrate of the serial port used', default='115200')
    parser.addoption('--network', action='store',
                     help='WiFi network credentials to which we want to connect device. Format network_ssid:network_password')
