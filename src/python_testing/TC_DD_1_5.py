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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#    run1:
#      app: ${TYPE_OF_APP}
#      app-args:
#      script-args: >
#      factory-reset: true
#      quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio

import nfc_helpers
from mobly import asserts

from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body
from matter.testing.runner import default_matter_test_main


class TC_DD_1_5(MatterBaseTest):
    def desc_TC_DD_1_5(self) -> str:
        return "[TC-DD-1.5] NFC Rules of Advertisement and Onboarding [DUT - Commissionee]"

    def pics_TC_DD_1_5(self) -> list[str]:
        """This test case verifies that the NFC Tag setup experience follows guidance in section 5.1.8"""
        return ["MCORE.ROLE.COMMISSIONEE", "MCORE.DD.NFC"]

    def steps_TC_DD_1_5(self) -> list[TestStep]:
        return [
            TestStep("1",
                     "Keep the DUT in packaging, if needed, activate tag detection in TH NFC reader, then bring in TH NFC reader at contact over all sides of the DUT packaging.",
                     "Onboarding Payload is not readable from the tag: Either the tag is not detected, or the content read is not Onboarding Payload.",
                     is_commissioning=False
                     ),
            TestStep("2a",
                     "Unpack DUT from its packaging, locate NFC tag, and, if needed, follow instructions to activate the NFC tag.",
                     "NFC tag is ready (will be verified in next step)"
                     ),
            TestStep("2b",
                     "If needed, activate tag detection in TH NFC reader, then bring in TH NFC reader close to the DUT’s NFC tag.",
                     "NFC tag is detected and read, it contains a Record of type URI whose data starts with \"MT:\""
                     ),
            TestStep("3",
                     "Try to write a new NDEF in the tag",
                     "Writing NFC tag failed, the content of the tag is unmodified."
                     )
        ]

    @async_test_body
    async def test_TC_DD_1_5(self):

        reader = nfc_helpers.NFCReaderHelper()
        asserts.assert_is_not_none(reader, "No connected NFC reader found")

        ###########
        self.step("1")
        if not self.is_pics_sdk_ci_only:
            monitoring_task = asyncio.create_task(reader.activate_tag_monitoring())
            user_input_task = asyncio.create_task(self.wait_for_user_input_async(
                "Press enter when done"
            ))
            await user_input_task
            reader.deactivate_tag_monitoring()
            tag = await monitoring_task
            asserts.assert_false(
                reader.is_onboarding_data(tag), "Found NFC tag with Onboarding data"
            )

        ###########
        self.step("2a")

        ###########
        self.step("2b")
        monitoring_task = asyncio.create_task(reader.activate_tag_monitoring())
        if not self.is_pics_sdk_ci_only:
            user_input_task = asyncio.create_task(self.wait_for_user_input_async(
                "Press enter when steps 2a and 2b are done", ""
            ))
            await user_input_task
        reader.deactivate_tag_monitoring()
        ndef = await monitoring_task
        asserts.assert_true(reader.is_onboarding_data(ndef), "No NFC tag with onboarding data found")

        ###########
        self.step("3")
        asserts.assert_false(reader.write_t4t_ndef_uri("MT:-24J029Q00OC0000000"))
        ndef_after = reader.read_t4t_ndef()
        asserts.assert_equal(ndef, ndef_after)

    async def wait_for_user_input_async(self, *args, **kwargs):
        loop = asyncio.get_running_loop()
        # Run the blocking function in a thread pool executor
        return await loop.run_in_executor(None, self.wait_for_user_input, *args, **kwargs)


if __name__ == "__main__":
    default_matter_test_main()
