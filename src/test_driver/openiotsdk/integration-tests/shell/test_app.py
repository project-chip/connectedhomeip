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

import chip.native
import pytest
from chip import exceptions
from chip.setup_payload import SetupPayload
from common.utils import get_shell_commands_from_help_response
from packaging import version

log = logging.getLogger(__name__)


@pytest.fixture(scope="session")
def binaryPath(request, rootDir):
    if request.config.getoption('binaryPath'):
        return request.config.getoption('binaryPath')
    else:
        return os.path.join(rootDir, 'examples/shell/openiotsdk/build/chip-openiotsdk-shell-example.elf')


SHELL_COMMAND_NAME = ["base64", "exit", "help", "version",
                      "config", "device", "onboardingcodes", "dns",
                      "echo", "log", "rand", "stat"]


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

        if "hardwareversion" in param_name:
            param_name = "hardwarever"

        config[param_name] = value
    return config


def parse_boarding_codes_response(response):
    codes = {}
    for param in response:
        codes[param.split(":")[0].lower()] = param.split()[1].strip()
    return codes


@pytest.mark.smoketest
def test_smoke_test(device):
    ret = device.wait_for_output("Open IoT SDK shell example application start")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("Open IoT SDK shell example application run")
    assert ret is not None and len(ret) > 0


@pytest.mark.ctrltest
def test_command_check(device):
    try:
        chip.native.Init()
    except exceptions.ChipStackException as ex:
        log.error("CHIP initialization failed {}".format(ex))
        assert False
    except Exception:
        log.error("CHIP initialization failed")
        assert False

    ret = device.wait_for_output("Open IoT SDK shell example application start")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("Open IoT SDK shell example application run")
    assert ret is not None and len(ret) > 0

    # Wait for printing prompt
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0

    # Help
    ret = device.send(command="help", expected_output="Done")
    assert ret is not None and len(ret) > 1
    shell_commands = get_shell_commands_from_help_response(ret[1:-1])
    assert set(SHELL_COMMAND_NAME) == set(shell_commands)

    # Echo
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="echo Hello", expected_output="Done")
    assert ret is not None and len(ret) > 1
    assert "Hello" in ret[-2]

    # Log
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="log Hello", expected_output="Done")
    assert ret is not None and len(ret) > 1
    assert "[INF] [TOO] Hello" in ret[-2]

    # Rand
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="rand", expected_output="Done")
    assert ret is not None and len(ret) > 1
    assert ret[-2].rstrip().isdigit()

    # Base64
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    hex_string = "1234"
    ret = device.send(command="base64 encode {}".format(
        hex_string), expected_output="Done")
    assert ret is not None and len(ret) > 1
    base64code = ret[-2]
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="base64 decode {}".format(
        base64code), expected_output="Done")
    assert ret is not None and len(ret) > 1
    assert ret[-2].rstrip() == hex_string

    # Version
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="version", expected_output="Done")
    assert ret is not None and len(ret) > 1
    assert "CHIP" in ret[-2].split()[0]
    app_version = ret[-2].split()[1]
    assert isinstance(version.parse(app_version), version.Version)

    # Config
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="config", expected_output="Done")
    assert ret is not None and len(ret) > 2

    config = parse_config_response(ret[1:-1])

    for param_name, value in config.items():
        ret = device.wait_for_output("Enter command")
        assert ret is not None and len(ret) > 0
        ret = device.send(command="config {}".format(
            param_name), expected_output="Done")
        assert ret is not None and len(ret) > 1
        if "discriminator" in param_name:
            assert int(ret[-2].split()[0], 16) == value
        else:
            assert int(ret[-2].split()[0]) == value

    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    new_value = int(config['discriminator']) + 1
    ret = device.send(command="config discriminator {}".format(
        new_value), expected_output="Done")
    assert ret is not None and len(ret) > 1
    assert "Setup discriminator set to: {}".format(new_value) in ret[-2]

    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="config discriminator", expected_output="Done")
    assert ret is not None and len(ret) > 1
    assert int(ret[-2].split()[0], 16) == new_value

    # Onboardingcodes
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="onboardingcodes none", expected_output="Done")
    assert ret is not None and len(ret) > 2

    boarding_codes = parse_boarding_codes_response(ret[1:-1])

    for param, value in boarding_codes.items():
        ret = device.wait_for_output("Enter command")
        assert ret is not None and len(ret) > 0
        ret = device.send(command="onboardingcodes none {}".format(
            param), expected_output="Done")
        assert ret is not None and len(ret) > 1
        assert value == ret[-2].strip()

    try:
        device_details = dict(SetupPayload().ParseQrCode(
            "VP:vendorpayload%{}".format(boarding_codes['qrcode'])).attributes)
    except exceptions.ChipStackError as ex:
        log.error(ex.msg)
        assert False
    assert device_details is not None and len(device_details) != 0

    try:
        device_details = dict(SetupPayload().ParseManualPairingCode(
            boarding_codes['manualpairingcode']).attributes)
    except exceptions.ChipStackError as ex:
        log.error(ex.msg)
        assert False
    assert device_details is not None and len(device_details) != 0

    # Exit - should be the last check
    ret = device.wait_for_output("Enter command")
    assert ret is not None and len(ret) > 0
    ret = device.send(command="exit", expected_output="Goodbye")
    assert ret is not None and len(ret) > 0
