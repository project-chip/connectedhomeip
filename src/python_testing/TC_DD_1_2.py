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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --custom-flow 0
#       --app-pipe /tmp/tmp_pipe
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --manual-code 10054912339
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --app-pipe /tmp/tmp_pipe
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from mobly import asserts

from matter.exceptions import ChipStackError
from matter.setup_payload import SetupPayload
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestUncommissionedDevice, TestStep
from matter.testing.runner import default_matter_test_main


class TC_DD_1_2(MatterTestUncommissionedDevice):
    def desc_TC_DD_1_2(self) -> str:
        return "[TC-DD-1.2] Manual Pairing Code Content [DUT - Commissionee]"

    def pics_TC_DD_1_2(self) -> list[str]:
        return ["MCORE.ROLE.COMMISSIONEE", "MCORE.DD.MANUAL_PC"]

    def steps_TC_DD_1_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Verify the first digit of the pairing code",
                     "First digit is 0-7. 0-3 requires an 11-digit code (VID/PID not present); "
                     "4-7 requires a 21-digit code (VID/PID present)"),
            TestStep(2, "If the pairing code is 11 digits, verify the encoded elements",
                     "Only run when the code is 11 digits"),
            TestStep("2b", "If the pairing code is 21 digits, verify the encoded elements",
                     "Only run when the code is 21 digits"),
            TestStep(3, "Verify the check digit of the pairing code",
                     "TH successfully parses the code, confirming a valid Verhoeff check digit"),
        ]

    @async_test_body
    async def test_TC_DD_1_2(self):
        self.step(1)
        asserts.assert_true(self.matter_test_config.manual_code, "This test needs to be run with the manual-code param.")
        manual_pairing_code = self.matter_test_config.manual_code[0]
        digits_length = len(manual_pairing_code)
        first_digit = int(manual_pairing_code[0])
        asserts.assert_true(0 <= first_digit <= 7, f"First digit must be between 0 and 7: {first_digit}")
        if 0 <= first_digit <= 3:
            asserts.assert_equal(digits_length, 11, f"First digit {first_digit} (0-3) requires an 11-digit code, got "
                                 f"{digits_length} digits")
        else:
            asserts.assert_equal(digits_length, 21, f"First digit {first_digit} (4-7) requires a 21-digit code, got "
                                 f"{digits_length} digits")

        self.step(2)
        if digits_length == 11:
            self._check_encoded_elements(manual_pairing_code)
        else:
            self.mark_current_step_skipped()

        self.step("2b")
        if digits_length == 21:
            self._check_encoded_elements(manual_pairing_code)
            digits_eleven_fifteen = int(manual_pairing_code[10:15])
            digits_sixteen_twenty = int(manual_pairing_code[15:20])
            asserts.assert_true(0 <= digits_eleven_fifteen <= 65535, "Digits 11 through 15 are not within valid range")
            asserts.assert_true(0 <= digits_sixteen_twenty <= 65535, "Digits 16 through 20 are not within valid range")
        else:
            self.mark_current_step_skipped()

        self.step(3)
        try:
            SetupPayload().ParseManualPairingCode(manual_pairing_code)
        except ChipStackError as e:
            asserts.fail(f"Validation failed for check digit of the manual pairing code: {e}")

    def _check_encoded_elements(self, manual_pairing_code: str) -> None:
        digits_two_six = int(manual_pairing_code[1:6])
        digits_seven_ten = int(manual_pairing_code[6:10])
        asserts.assert_true(0 <= digits_two_six <= 65535, "Digits 2 through 6 are not within valid range")
        asserts.assert_true(0 <= digits_seven_ten <= 8191, "Digits 7 through 10 are not within valid range")


if __name__ == "__main__":
    default_matter_test_main()
