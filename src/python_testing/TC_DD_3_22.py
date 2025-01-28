#
#    Copyright (c) 2025 Project CHIP Authors
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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#       quiet: True
# === END CI TEST ARGUMENTS ===

from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_DD_3_22(MatterBaseTest):
    def desc_TC_DD_3_22(self) -> str:
        return "[TC-DD-3.22] Commissioning Flow - Enhanced Setup Flow Terms and Conditions [DUT - Commissioner]"

    def pics_TC_DD_3_22(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["MCORE.ROLE.COMMISSIONER", "MCORE.DD.ESF_TC_COMMISSIONER"]

    def steps_TC_DD_3_22(self) -> list[TestStep]:
        return [
            TestStep(0, description="", expectation="", is_commissioning=False),
            TestStep(1, "Commissioner has regulatory and fabric information available and has accurate date, time and timezone"),
            TestStep(2, "Input TH's onboarding payload (QR or numeric code) using the DUT Commissioner."),
            TestStep(3, "DUT parses TH's onboarding payload and retrieves DeviceModel for TH from DCL."),
            TestStep(4, "Commissioner SHALL present the Terms and Conditions located at EnhancedSetupFlowTCUrl to the user and prompt for input."),
            TestStep(5, "Commissioner and Commissionee discover each other and connect via the discovery mode applicable for the DUT."),
            TestStep(6, "Establish encryption keys with Password Authenticated Session Establishment on the commissioning channel"),
            TestStep(7, "Commissioner SHALL re-arm Fail-safe timer on Commissionee within 60s (the autonomously Fail-safe timer length set by Commissionee)"),
            TestStep(8, "Commissioner SHALL configure regulatory information in the Commissionee."),
            TestStep(9, "Commissioner SHALL configure the ESF TC acknowledgements in the Commissionee."),
            TestStep(10, "Commissioner proceeds through the rest of the commissioning process."),
        ]

    @async_test_body
    async def test_TC_DD_3_22(self):
        self.step(0)
        if not self.pics_guard(self.check_pics("MCORE.ROLE.COMMISSIONER") and self.check_pics("MCORE.DD.ESF_TC_COMMISSIONER")):
            self.skip_all_remaining_steps(1)
            return

        self.step(1)
        self.wait_for_user_input("Does the commissioner have regulatory, fabric information, and an accurate time source?")

        self.step(2)
        self.wait_for_user_input("Using the commissioner, manually enter the onboarding payload.")

        self.step(3)
        self.wait_for_user_input(
            "Was the commissioner able to decode the onboarding payload then retrieve the DeviceModel from DCL?")

        self.step(4)
        self.wait_for_user_input(
            "Does the commissioner present the required Terms and Conditions defined within the published enhanced setup flow payload's TC URL?")

        self.step(5)
        self.wait_for_user_input("Was the commissioner able to establish encryption keys using PASE?")

        self.step(6)
        self.wait_for_user_input("Using the commissioner, manually accept the presented Terms and Conditions, then continue.")

        self.step(7)
        self.wait_for_user_input("Did the commissioner re-arm the fail-safe timer?")

        self.step(8)
        self.wait_for_user_input("Did the commissioner set the regulatory, fabric information, time information?")

        self.step(9)
        self.wait_for_user_input("Did the commissioner set the user's accepted Terms and Conditions?")

        self.step(10)
        self.wait_for_user_input("Did the commissionee complete commissioning successfully?")


if __name__ == "__main__":
    default_matter_test_main()
