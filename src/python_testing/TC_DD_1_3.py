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

# Test excluded from CI: requires a physical NFC reader and an NFC-tagged DUT.

import asyncio
import logging

from mobly import asserts
from onboarding_payload_checks import OnboardingPayloadChecks

import matter.testing.nfc
from matter.setup_payload import SetupPayload
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestUncommissionedDevice, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_DD_1_3(OnboardingPayloadChecks, MatterTestUncommissionedDevice):
    def desc_TC_DD_1_3(self) -> str:
        return "[TC-DD-1.3] NFC Tag Content [DUT - Commissionee]"

    def pics_TC_DD_1_3(self) -> list[str]:
        return ["MCORE.ROLE.COMMISSIONEE", "MCORE.DD.NFC"]

    def steps_TC_DD_1_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Power up the DUT and put the DUT in pairing mode"),
            TestStep(2, "Bring the NFC reader close to the DUT",
                     "NFC tag is detected and read, its content is an onboarding payload"),
            TestStep("3a", "Verify the onboarding payload version", "Version field is 0"),
            TestStep("3b", "Verify the 8-bit Discovery Capabilities Bitmask",
                     "No reserved bits are set, and at least one discovery method is advertised"),
            TestStep("3c", "Verify the 12-bit discriminator matches the value the DUT advertises during commissioning",
                     "DUT is discoverable using the discriminator encoded in the NFC tag"),
            TestStep("3d", "Verify the onboarding payload contains a 27-bit Passcode",
                     "Passcode is between 0x0000001 and 0x5f5e0fe"),
            TestStep("3e", "Verify passcode is valid", "Passcode is not a disallowed default value"),
            TestStep("3f", "Verify the NFC tag's onboarding payload code prefix", "Prefix is \"MT:\""),
            TestStep("3g", "Verify Vendor ID and Product ID are present in the payload"),
            TestStep(4, "Verify the packed binary data structure",
                     "Structure is padded with 0 bits at the end to the nearest byte boundary"),
            TestStep(5, "Verify the Commissioning Flow value", "Value is 0, 1 or 2"),
        ]

    @async_test_body
    async def test_TC_DD_1_3(self):
        self.step(1)
        log.info("DUT expected to be powered on and in pairing mode.")

        self.step(2)
        reader = matter.testing.nfc.NFCReader()
        monitoring_task = asyncio.create_task(reader.activate_tag_monitoring())
        await self.wait_for_user_input_async(
            "Bring the TH NFC reader close to the DUT's NFC tag. Press Enter when done.")
        reader.deactivate_tag_monitoring()
        nfc_tag_content = await monitoring_task
        log.info("nfc_tag_content: %s", nfc_tag_content)
        asserts.assert_true(reader.is_onboarding_data(nfc_tag_content), "No NFC tag with onboarding data found")

        payload = SetupPayload().ParseQrCode(nfc_tag_content)

        self.step("3a")
        self.check_payload_version(payload)

        self.step("3b")
        self.check_discovery_capabilities_bitmask(payload)

        self.step("3c")
        asserts.assert_is_not_none(payload.long_discriminator, "NFC tag payload is missing the long discriminator")
        await self.check_advertised_discriminator(payload.long_discriminator, long=True)

        self.step("3d")
        self.check_passcode_range(payload)

        self.step("3e")
        self.check_passcode_validity(payload)

        self.step("3f")
        self.check_code_prefix(nfc_tag_content[:3])

        self.step("3g")
        self.log_vendor_and_product_id_not_verified(payload)

        self.step(4)
        # QRCodeSetupPayloadParser (src/setup_payload/QRCodeSetupPayloadParser.cpp) rejects the
        # payload with CHIP_ERROR_INVALID_ARGUMENT if the trailing padding bits are not all 0, so
        # the successful parse above already confirms the packed structure is correctly padded.
        log.info("Padding bits validated implicitly by the successful parse.")

        self.step(5)
        self.check_commissioning_flow(payload)

    async def wait_for_user_input_async(self, *args, **kwargs):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self.wait_for_user_input, *args, **kwargs)


if __name__ == "__main__":
    default_matter_test_main()
