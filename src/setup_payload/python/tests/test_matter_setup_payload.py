#!/usr/bin/env python3
#
#    Copyright (c) 2024 Project CHIP Authors
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

import unittest

from matter_setup_payload import CommissioningFlow, SetupPayload

test_dataset = [
    {
        "qrcode": "MT:00000CQM00KA0648G00", "manualcode": "34970112332",
        "payload": SetupPayload(3840, 20202021),
    },
    {
        "qrcode": "MT:-8VJ0OMV172PX813210", "manualcode": "32140830464",
        "payload": SetupPayload(3431, 49910688, rendezvous=2, vid=0x1317, pid=0x0002),
    },
    {
        "qrcode": "MT:0A.T0RTM00Y0OJ0.510", "manualcode": "00210433428",
        "payload": SetupPayload(80, 54757432, flow=CommissioningFlow.Standard, vid=0x2345, pid=0x1023),
    },
    {
        "qrcode": "MT:KSNK4SIC13-JPR4UY00", "manualcode": "702871264504387000187",
        "payload": SetupPayload(3091, 43338551, flow=CommissioningFlow.Custom, vid=0x1123, pid=0x0012),
    },
    {
        "qrcode": "MT:A3L90ARR15G6N57Y900", "manualcode": "745492075300001000013",
        "payload": SetupPayload(3781, 12349876, flow=CommissioningFlow.UserIntent, vid=1, pid=1),
    },
    {
        "qrcode": "MT:KSNK4M5113-JPR4UY00", "manualcode": "702871264504387000187",
        "payload": SetupPayload(3091, 43338551, rendezvous=2, flow=CommissioningFlow.Custom, vid=0x1123, pid=0x0012),
    },
    {
        "qrcode": "MT:EPX0482F26DAVY09R10", "manualcode": "403732495800069000166",
        "payload": SetupPayload(174, 81235604, rendezvous=7, flow=CommissioningFlow.UserIntent, vid=0x45, pid=0x10),
    },
    {
        "qrcode": "MT:MZWA6G6026O2XP0II00", "manualcode": "619156140465523329207",
        "payload": SetupPayload(2310, 23005908, flow=CommissioningFlow.Custom, vid=0xFFF3, pid=0x8098),
    },
]


class TestSetupPayload(unittest.TestCase):
    def test_generated_payload(self):
        for test_case in test_dataset:
            self.assertEqual(SetupPayload.generate_qrcode(test_case["payload"]), test_case["qrcode"])
            self.assertEqual(SetupPayload.generate_manualcode(test_case["payload"]), test_case["manualcode"])


def verify_generated_payloads(in_params, manualcode_params, qrcode_params):
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

    def test_parse_onboarding_code(self):
        for test_case in test_dataset:
            parsed_qrcode_obj = SetupPayload.parse(test_case["qrcode"])
            parsed_manualcode_obj = SetupPayload.parse(test_case["manualcode"])
            expected_payload = test_case["payload"]

            self.assertEqual(parsed_manualcode_obj.passcode, expected_payload.passcode)
            self.assertEqual(parsed_manualcode_obj.short_discriminator, expected_payload.short_discriminator)
            if expected_payload.flow != CommissioningFlow.Standard:
                self.assertEqual(parsed_manualcode_obj.vid, expected_payload.vid)
                self.assertEqual(parsed_manualcode_obj.pid, expected_payload.pid)

            self.assertEqual(parsed_qrcode_obj.passcode, expected_payload.passcode)
            self.assertEqual(parsed_qrcode_obj.short_discriminator, expected_payload.short_discriminator)
            self.assertEqual(parsed_qrcode_obj.long_discriminator, expected_payload.long_discriminator)
            self.assertEqual(parsed_qrcode_obj.discovery, expected_payload.discovery)
            self.assertEqual(parsed_qrcode_obj.flow, expected_payload.flow)
            self.assertEqual(parsed_qrcode_obj.vid, expected_payload.vid)
            self.assertEqual(parsed_qrcode_obj.pid, expected_payload.pid)


if __name__ == "__main__":
    unittest.main()
