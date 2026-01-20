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

# Test excluded from CI

import asyncio
import logging

from mobly import asserts, signals

import matter.testing.nfc
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


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
                     "Compare QR Code and NFCTag onboarding data",
                     "Onboarding data are identical."
                     ),
            TestStep("4",
                     "Try to write a new NDEF in the tag",
                     "Writing NFC tag fails, the content of the tag is unmodified."
                     )
        ]

    @async_test_body
    async def test_TC_DD_1_5(self):

        reader = matter.testing.nfc.NFCReader()

        ###########
        self.step("1")

        monitoring_task = asyncio.create_task(reader.activate_tag_monitoring())
        user_input_task = asyncio.create_task(self.wait_for_user_input_async(
            "Bring in NFC reader at contact over all sides of the DUT packaging. Press enter when done"
        ))
        await user_input_task
        reader.deactivate_tag_monitoring()
        ndef = await monitoring_task
        asserts.assert_false(reader.is_onboarding_data(ndef), "Found NFC tag with Onboarding data when none is expected")

        ###########
        self.step("2a")

        ###########
        self.step("2b")
        monitoring_task = asyncio.create_task(reader.activate_tag_monitoring())
        user_input_task = asyncio.create_task(self.wait_for_user_input_async(
            "Press enter when steps 2a and 2b are done", ""
        ))
        await user_input_task

        reader.deactivate_tag_monitoring()
        nfc_tag_content = await monitoring_task
        log.info(f"nfc_tag_content: {nfc_tag_content}")
        asserts.assert_true(reader.is_onboarding_data(nfc_tag_content), "No NFC tag with onboarding data found")

        ###########
        self.step("3")

        if self.check_pics("MCORE.DD.NTL"):
            log.info("MCORE.DD.NTL is set")
        else:
            log.info("MCORE.DD.NTL is not set")

        if self.check_pics("MCORE.DD.QR"):
            log.info("MCORE.DD.QR is set")
        else:
            log.info("MCORE.DD.QR is not set")

        # The current test can be executed on:
        # - devices supporting NTL.
        # - devices with NFC onboarding data (not necessarily supporting NTL).
        if self.check_pics("MCORE.DD.NTL"):
            # Device supporting NTL: QRCode is mandatory
            asserts.assert_true(self.check_pics("MCORE.DD.QR"),
                                "NTL devices SHALL have a QRCode")

            asserts.assert_true(self.matter_test_config.qr_code_content,
                                "This test needs to be run with qr_code param")
            qr_code_content = self.matter_test_config.qr_code_content[0]
            log.info(f"qr_code_content: {qr_code_content}")

            asserts.assert_equal(
                qr_code_content,
                nfc_tag_content,
                f"Error! QR Code and NFC Tag onboarding data differ! "
                f"qr_code_content='{qr_code_content}', nfc_tag_content='{nfc_tag_content}'"
            )

        else:
            log.info("Device not supporting NTL: QRCode is optional")

        ###########
        self.step("4")

        with asserts.assert_raises(signals.TestFailure):
            reader.write_ndef_uri("MT:-24J029Q00OC0000000")

        # Check that NDEF content is unchanged
        nfc_tag_content_after_write = reader.read_nfc_tag_data()
        asserts.assert_equal(nfc_tag_content, nfc_tag_content_after_write, "Error! NDEF content has been changed!")

    async def wait_for_user_input_async(self, *args, **kwargs):
        loop = asyncio.get_running_loop()
        # Run the blocking function in a thread pool executor
        return await loop.run_in_executor(None, self.wait_for_user_input, *args, **kwargs)


if __name__ == "__main__":
    default_matter_test_main()
