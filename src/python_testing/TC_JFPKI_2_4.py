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

# This test requires a Joint Fabric Administrator app and Joint Fabric Controller app.
# Please specify with:
#   --string-arg jfa_server_app:<path_to_app>
#   --string-arg jfc_server_app:<path_to_app>

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     script-args: >
#       --string-arg jfa_server_app:${JF_ADMIN_APP}
#       --string-arg jfc_server_app:${JF_CONTROL_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_JFPKI_2_4(MatterBaseTest):
    def desc_TC_JFPKI_2_4(self) -> str:
        return "[TC-JFPKI-2.4] Validate OpenJointCommissioningWindow Command correct behavior"

    def pics_TC_JFPKI_2_4(self) -> list[str]:
        return ["JFPKI.S"]

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        # Placeholders for state that will be used once command flow is implemented.
        self.fabric_a_ctrl = None
        self.fabric_a_admin = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)

    def teardown_class(self):
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()
        if self.fabric_a_ctrl is not None:
            self.fabric_a_ctrl.terminate()

        super().teardown_class()

    def steps_TC_JFPKI_2_4(self) -> list[TestStep]:
        return [
            TestStep("1", "Commission DUT to TH."),
            TestStep("2", "TH sends OJCW command to DUT with valid parameters: CommissioningTimeout=180, PAKEPasscodeVerifier=valid_97_byte_verifier, Discriminator=3840, Iterations=2000, Salt=valid_16_byte_salt.",
                     "DUT responds with SUCCESS status and opens its commissioning window."),
            TestStep("3", "Verify commissioning window is open by checking DUT advertisement.",
                     "DUT advertises commissioning service with correct discriminator."),
            TestStep("4", "Wait for CommissioningTimeout to expire.",
                     "DUT stops advertising commissioning service."),
            TestStep("5", "TH sends OJCW command to DUT with CommissioningTimeout=0.",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("6", "TH sends OJCW command to DUT with CommissioningTimeout=65535.",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("7", "TH sends OJCW command to DUT with PAKEPasscodeVerifier of incorrect length (96 bytes).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("8", "TH sends OJCW command to DUT with PAKEPasscodeVerifier of incorrect length (98 bytes).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("9", "TH sends OJCW command to DUT with Discriminator=4096 (out of range).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("10", "TH sends OJCW command to DUT with Iterations=999 (below minimum).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("11", "TH sends OJCW command to DUT with Iterations=100001 (above maximum).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("12", "TH sends OJCW command to DUT with Salt of length 15 bytes (below minimum).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("13", "TH sends OJCW command to DUT with Salt of length 33 bytes (above maximum).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("14", "TH sends OJCW command to DUT with valid parameters: CommissioningTimeout=60, Iterations=1000 (minimum valid), Salt=valid_32_byte_salt (maximum valid).",
                     "DUT responds with SUCCESS status and opens commissioning window."),
            TestStep("15", "Verify commissioning window behavior with minimum/maximum valid parameters.",
                     "DUT advertises commissioning service correctly."),
            TestStep("16", "While commissioning window is open, TH sends another OJCW command.",
                     "DUT responds with BUSY status code."),
            TestStep("17", "Wait for commissioning window to close.",
                     "DUT stops advertising commissioning service."),
            TestStep("18", "TH sends OJCW command to DUT with Iterations=50000 (mid-range valid), Salt=valid_24_byte_salt (mid-range valid).",
                     "DUT responds with SUCCESS status and opens commissioning window."),
            TestStep("19", "Verify commissioning window opens with mid-range parameters.",
                     "DUT advertises commissioning service correctly."),
            TestStep("20", "Wait for commissioning window to close.",
                     "DUT stops advertising commissioning service."),
        ]

    @async_test_body
    async def test_TC_JFPKI_2_4(self):
        self.step("1")
        # TODO: Commission DUT to TH.
        self.step("2")
        # TODO: TH sends OJCW command to DUT with valid parameters: CommissioningTimeout=180, PAKEPasscodeVerifier=valid_97_byte_verifier, Discriminator=3840, Iterations=2000, Salt=valid_16_byte_salt.
        # Expected: DUT responds with SUCCESS status and opens its commissioning window.
        self.step("3")
        # TODO: Verify commissioning window is open by checking DUT advertisement.
        # Expected: DUT advertises commissioning service with correct discriminator.
        self.step("4")
        # TODO: Wait for CommissioningTimeout to expire.
        # Expected: DUT stops advertising commissioning service.
        self.step("5")
        # TODO: TH sends OJCW command to DUT with CommissioningTimeout=0.
        # Expected: DUT responds with INVALID_COMMAND status code.
        self.step("6")
        # TODO: TH sends OJCW command to DUT with CommissioningTimeout=65535.
        # Expected: DUT responds with INVALID_COMMAND status code.
        self.step("7")
        # TODO: TH sends OJCW command to DUT with PAKEPasscodeVerifier of incorrect length (96 bytes).
        # Expected: DUT responds with INVALID_COMMAND status code.
        self.step("8")
        # TODO: TH sends OJCW command to DUT with PAKEPasscodeVerifier of incorrect length (98 bytes).
        # Expected: DUT responds with INVALID_COMMAND status code.
        self.step("9")
        # TODO: TH sends OJCW command to DUT with Discriminator=4096 (out of range).
        # Expected: DUT responds with INVALID_COMMAND status code.
        self.step("10")
        # TODO: TH sends OJCW command to DUT with Iterations=999 (below minimum).
        # Expected: DUT responds with INVALID_COMMAND status code.
        self.step("11")
        # TODO: TH sends OJCW command to DUT with Iterations=100001 (above maximum).
        # Expected: DUT responds with INVALID_COMMAND status code.
        self.step("12")
        # TODO: TH sends OJCW command to DUT with Salt of length 15 bytes (below minimum).
        # Expected: DUT responds with INVALID_COMMAND status code.
        self.step("13")
        # TODO: TH sends OJCW command to DUT with Salt of length 33 bytes (above maximum).
        # Expected: DUT responds with INVALID_COMMAND status code.
        self.step("14")
        # TODO: TH sends OJCW command to DUT with valid parameters: CommissioningTimeout=60, Iterations=1000 (minimum valid), Salt=valid_32_byte_salt (maximum valid).
        # Expected: DUT responds with SUCCESS status and opens commissioning window.
        self.step("15")
        # TODO: Verify commissioning window behavior with minimum/maximum valid parameters.
        # Expected: DUT advertises commissioning service correctly.
        self.step("16")
        # TODO: While commissioning window is open, TH sends another OJCW command.
        # Expected: DUT responds with BUSY status code.
        self.step("17")
        # TODO: Wait for commissioning window to close.
        # Expected: DUT stops advertising commissioning service.
        self.step("18")
        # TODO: TH sends OJCW command to DUT with Iterations=50000 (mid-range valid), Salt=valid_24_byte_salt (mid-range valid).
        # Expected: DUT responds with SUCCESS status and opens commissioning window.
        self.step("19")
        # TODO: Verify commissioning window opens with mid-range parameters.
        # Expected: DUT advertises commissioning service correctly.
        self.step("20")
        # TODO: Wait for commissioning window to close.
        # Expected: DUT stops advertising commissioning service.


if __name__ == "__main__":
    default_matter_test_main()
