#
#    Copyright (c) 2026 Project CHIP Authors
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

# Test excluded from CI: verifies a QR code concatenating multiple DUTs' onboarding
# payloads, which requires a tester-supplied expected device count.

from mobly import asserts

from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import CertificationUnitTestNoDevice, TestStep
from matter.testing.runner import default_matter_test_main

# Section 5.1.5 "Concatenation" of the specification defines '*' as the delimiter
# between concatenated onboarding payloads.
_CONCATENATION_DELIMITER = "*"


class TC_DD_1_4(CertificationUnitTestNoDevice):
    def desc_TC_DD_1_4(self) -> str:
        return "[TC-DD-1.4] Concatenated QR Code Content [DUT - Commissionee]"

    def pics_TC_DD_1_4(self) -> list[str]:
        return ["MCORE.ROLE.COMMISSIONEE", "MCORE.DD.CONCATENATED_QR_CODE"]

    def steps_TC_DD_1_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Scan the DUTs' concatenated QR code and verify the number of delimiters",
                     "Number of '*' delimiters is one less than the number of onboarding DUTs"),
        ]

    @async_test_body
    async def test_TC_DD_1_4(self):
        self.step(1)
        asserts.assert_true(self.matter_test_config.qr_code_content, "This test needs to be run with the qr-code param.")
        qr_code_content = self.matter_test_config.qr_code_content[0]
        device_count_response = self.wait_for_user_input(
            "Please specify the number of devices that will be onboarded.", "0x2")
        device_count = int(device_count_response, 16)

        delimiter_count = qr_code_content.count(_CONCATENATION_DELIMITER)
        asserts.assert_equal(
            delimiter_count, device_count - 1,
            f"Invalid number of delimiters detected in QR code payload: {delimiter_count}, expected {device_count - 1}")


if __name__ == "__main__":
    default_matter_test_main()
