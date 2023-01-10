#!/usr/bin/env python3

# Copyright (c) 2023 Project CHIP Authors
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

import os
import re
import sys
import random
import subprocess

CHIP_TOPDIR = os.path.dirname(os.path.realpath(__file__))[:-len(os.path.join('src', 'setup_payload', 'tests'))]
sys.path.insert(0, os.path.join(CHIP_TOPDIR, 'src', 'setup_payload', 'python'))
from generate_setup_payload import SetupPayload, CommissioningFlow, INVALID_PASSCODES  # noqa: E402


def payload_param_dict():
    return {
        'Version': None,
        'VendorID': None,
        'ProductID': None,
        'Custom flow': None,
        'Discovery Bitmask': None,
        'Short discriminator': None,
        'Long discriminator': None,
        'Passcode': None
    }


def remove_escape_sequence(data):
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    result = ansi_escape.sub('', data)
    return result


def parse_setup_payload(chip_tool, payload):
    cmd_args = [chip_tool, 'payload', 'parse-setup-payload', payload]
    data = subprocess.check_output(cmd_args).decode('utf-8')
    data = remove_escape_sequence(data)
    parsed_params = payload_param_dict()
    for key in parsed_params:
        k_st = data.find(key)
        if k_st == -1:
            continue

        # 1 is for ":"
        k_end = k_st + len(key) + 1

        k_nl = data.find('\n', k_end)
        parsed_params[key] = data[k_end:k_nl].split()[0]

    return parsed_params


def generate_payload_params():
    payload_params = payload_param_dict()
    payload_params['Version'] = 0
    payload_params['VendorID'] = random.randint(0, 0xFFFF)
    payload_params['ProductID'] = random.randint(0, 0xFFFF)
    payload_params['Custom flow'] = random.randint(0, 2)
    payload_params['Discovery Bitmask'] = random.randint(0, 7)
    payload_params['Long discriminator'] = random.randint(0, 0x0FFF)
    payload_params['Short discriminator'] = (payload_params['Long discriminator'] >> 8)
    payload_params['Passcode'] = random.randint(1, 0x5F5E0FE)

    # Rather than regenerating, just subtracting 5
    if payload_params['Passcode'] in INVALID_PASSCODES:
        payload_params['Passcode'] -= 5

    return payload_params


def generate_payloads(in_params):
    payloads = SetupPayload(in_params['Long discriminator'], in_params['Passcode'],
                            in_params['Discovery Bitmask'], CommissioningFlow(in_params['Custom flow']),
                            in_params['VendorID'], in_params['ProductID'])
    manualcode = payloads.generate_manualcode()
    qrcode = payloads.generate_qrcode()
    return manualcode, qrcode


def verify_payloads(chip_tool):
    in_params = generate_payload_params()
    manualcode, qrcode = generate_payloads(in_params)
    manualcode_params = parse_setup_payload(chip_tool, manualcode)
    qrcode_params = parse_setup_payload(chip_tool, qrcode)

    print("Input parameters:", in_params)
    print("Manualcode:", manualcode)
    print("QRCode:", qrcode)
    print("Manualcode parsed by chip-tool:", manualcode_params)
    print("QRCode parsed by chip-tool:", qrcode_params)

    assert in_params['Version'] == int(manualcode_params['Version'], 0)
    assert in_params['Passcode'] == int(manualcode_params['Passcode'], 0)
    assert in_params['Short discriminator'] == int(manualcode_params['Short discriminator'], 0)
    if in_params['Custom flow'] != 0:
        assert in_params['VendorID'] == int(manualcode_params['VendorID'], 0)
        assert in_params['ProductID'] == int(manualcode_params['ProductID'], 0)

    assert in_params['Version'] == int(qrcode_params['Version'], 0)
    assert in_params['VendorID'] == int(qrcode_params['VendorID'], 0)
    assert in_params['ProductID'] == int(qrcode_params['ProductID'], 0)
    assert in_params['Custom flow'] == int(qrcode_params['Custom flow'], 0)
    assert in_params['Discovery Bitmask'] == int(qrcode_params['Discovery Bitmask'], 0)
    assert in_params['Passcode'] == int(qrcode_params['Passcode'], 0)
    assert in_params['Long discriminator'] == int(qrcode_params['Long discriminator'], 0)


def main():
    if len(sys.argv) == 2:
        chip_tool = sys.argv[1]
        for i in range(0, 10):
            verify_payloads(chip_tool)


if __name__ == '__main__':
    main()
