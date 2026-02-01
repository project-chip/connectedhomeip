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


import logging

from mobly import asserts

import matter.testing.nfc
from matter.setup_payload import SetupPayload
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_DD_3_23(MatterBaseTest):
    def desc_TC_DD_3_23(self) -> str:
        return "[TC-DD-3.23] NFC-based commissioning [DUT as Commissionee]"

    def steps_TC_DD_3_23(self) -> list[TestStep]:
        return [
            TestStep(1, "Detecting the NFC Tag and reading the Payload", is_commissioning=False),
            TestStep(2, 'Validate the NFC bit in payload and Perform the commissioning')
        ]

    @async_test_body
    async def test_TC_DD_3_23(self):

        # Step 1: Here we check if the Tag is connected to the Host machine and read the NFC Tag data
        self.step(1)

        nfc_reader_index = self.user_params.get("NFC_Reader_index", 0)
        reader = matter.testing.nfc.NFCReader(nfc_reader_index)

        nfc_tag_data = reader.read_nfc_tag_data()
        log.info(f"NFC Tag data : '{nfc_tag_data}'")
        asserts.assert_true(
            reader.is_onboarding_data(nfc_tag_data),
            f"'{nfc_tag_data}' is not a valid Matter URI"
        )
        self.matter_test_config.qr_code_content.append(nfc_tag_data)

        # Step 2: the NFC tag data is parsed and checked if the device supports NFC commissioning and commission begins
        self.step(2)
        payload = SetupPayload().ParseQrCode(nfc_tag_data)
        asserts.assert_true(payload.supports_nfc_commissioning, "Device does not Support NFC Commissioning")
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        commissioning_success = await self.commission_devices()
        asserts.assert_true(commissioning_success, "Device Commissioning using nfc transport has failed")


if __name__ == "__main__":
    default_matter_test_main()
