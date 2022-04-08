# Copyright (c) 2009-2021 Arm Limited
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
import re
from packaging import version
from time import sleep

from chip.setup_payload import SetupPayload
from chip import exceptions
from chip import ChipDeviceCtrl
from common.utils import *
import logging
log = logging.getLogger(__name__)

BLE_DEVICE_NAME = "MBED-shell"

SHELL_COMMAND_NAME = ["echo", "log", "rand", "ping", "send", "base64", "version",
                      "ble", "wifi", "config", "device", "onboardingcodes", "dns", "help", "exit"]
WIFI_MODE_NAME = ["disable", "ap", "sta"]


def get_shell_command(response):
    return [line.split()[0].strip() for line in response]


def parse_config_response(response):
    config = {}
    for param in response:
        param_name = param.split(":")[0].lower()
        if "discriminator" in param_name:
            value = int(param.split(":")[1].strip(), 16)
        elif "pincode" in param_name:
            value = int(param.split(":")[1].strip())
        else:
            value = int(param.split(":")[1].split()[0].strip())

        if "productrevision" in param_name:
            param_name = "productrev"

        config[param_name] = value
    return config


def parse_boarding_codes_response(response):
    codes = {}
    for param in response:
        codes[param.split(":")[0].lower()] = param.split()[1].strip()
    return codes


@pytest.mark.smoketest
def test_smoke_test(device):
    device.reset(duration=1)
    ret = device.wait_for_output("Mbed shell example application start")
    assert ret != None and len(ret) > 0
    ret = device.wait_for_output("Mbed shell example application run")
    assert ret != None and len(ret) > 0


def test_help_check(device):
    ret = device.send(command="help", expected_output="Done")
    assert ret != None and len(ret) > 1
    shell_commands = get_shell_command(ret[1:-1])
    assert set(SHELL_COMMAND_NAME) == set(shell_commands)


def test_echo_check(device):
    ret = device.send(command="echo Hello", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert "Hello" in ret[-2]


def test_log_check(device):
    ret = device.send(command="log Hello", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert "[INFO][CHIP]: [TOO]Hello" in ret[-2]


def test_rand_check(device):
    ret = device.send(command="rand", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().isdigit()


def test_base64_encode_decode(device):
    hex_string = "1234"
    ret = device.send(command="base64 encode {}".format(
        hex_string), expected_output="Done")
    assert ret != None and len(ret) > 1
    base64code = ret[-2]
    ret = device.send(command="base64 decode {}".format(
        base64code), expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip() == hex_string


def test_version_check(device):
    ret = device.send(command="version", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert "CHIP" in ret[-2].split()[0]
    app_version = ret[-2].split()[1]
    assert isinstance(version.parse(app_version), version.Version)


def test_ble_adv_check(device):
    devCtrl = ChipDeviceCtrl.ChipDeviceController()

    ret = device.send(command="ble adv start", expected_output="Done")
    assert ret != None and len(ret) > 0
    ret = device.send(command="ble adv state", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert "enabled" in ret[-2].split()[-1]

    sleep(1)

    assert check_chip_ble_devices_advertising(devCtrl, BLE_DEVICE_NAME)

    ret = device.send(command="ble adv stop", expected_output="Done")
    assert ret != None and len(ret) > 0
    ret = device.send(command="ble adv state", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert "disabled" in ret[-2].split()[-1]

    sleep(1)

    assert not check_chip_ble_devices_advertising(devCtrl, BLE_DEVICE_NAME)


def test_device_config_check(device):
    ret = device.send(command="config", expected_output="Done")
    assert ret != None and len(ret) > 2

    config = parse_config_response(ret[1:-1])

    for param_name, value in config.items():
        ret = device.send(command="config {}".format(
            param_name), expected_output="Done")
        assert ret != None and len(ret) > 1
        if "discriminator" in param_name:
            assert int(ret[-2].split()[0], 16) == value
        else:
            assert int(ret[-2].split()[0]) == value

    new_value = int(config['discriminator']) + 1
    ret = device.send(command="config discriminator {}".format(
        new_value), expected_output="Done")
    assert ret != None and len(ret) > 1
    assert "Setup discriminator set to: {}".format(new_value) in ret[-2]

    ret = device.send(command="config discriminator", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert int(ret[-2].split()[0], 16) == new_value


def test_on_boarding_codes(device):
    ret = device.send(command="onboardingcodes", expected_output="Done")
    assert ret != None and len(ret) > 2

    boarding_codes = parse_boarding_codes_response(ret[1:-1])

    for param, value in boarding_codes.items():
        ret = device.send(command="onboardingcodes {}".format(
            param), expected_output="Done")
        assert ret != None and len(ret) > 1
        assert value == ret[-2].strip()

    try:
        device_details = dict(SetupPayload().ParseQrCode(
            "VP:vendorpayload%{}".format(boarding_codes['qrcode'])).attributes)
    except exceptions.ChipStackError as ex:
        log.error(ex.msg)
        assert False
    assert device_details != None and len(device_details) != 0

    try:
        device_details = dict(SetupPayload().ParseManualPairingCode(
            boarding_codes['manualpairingcode']).attributes)
    except exceptions.ChipStackError as ex:
        log.error(ex.msg)
        assert False
    assert device_details != None and len(device_details) != 0


def test_wifi_mode(device):
    ret = device.send(command="wifi mode", expected_output="Done")
    assert ret != None and len(ret) > 1
    current_mode = ret[-2].strip()
    assert current_mode in WIFI_MODE_NAME

    for mode in [n for n in WIFI_MODE_NAME if n != current_mode]:
        print(mode)
        ret = device.send(command="wifi mode {}".format(
            mode), expected_output="Done")
        assert ret != None and len(ret) > 0

        ret = device.send(command="wifi mode", expected_output="Done")
        assert ret != None and len(ret) > 1
        assert ret[-2].strip() == mode


def test_wifi_connect(device, network):
    network_ssid = network[0]
    network_pass = network[1]

    ret = device.send(command="wifi connect {} {}".format(
        network_ssid, network_pass), expected_output="Done")
    assert ret != None and len(ret) > 0

    ret = device.wait_for_output("StationConnected", 30)
    assert ret != None and len(ret) > 0


def test_device_factory_reset(device):
    ret = device.send(command="device factoryreset")

    sleep(1)

    ret = device.wait_for_output("Mbed shell example application start")
    assert ret != None and len(ret) > 0
    ret = device.wait_for_output("Mbed shell example application run")
    assert ret != None and len(ret) > 0


def test_exit_check(device):
    ret = device.send(command="exit", expected_output="Goodbye")
    assert ret != None and len(ret) > 0
