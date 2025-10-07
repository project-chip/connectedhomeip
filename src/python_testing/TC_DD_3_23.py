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
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

from matter.setup_payload import SetupPayload
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.testing.matter_nfc_interaction import connect_read_nfc_tag_data

logger = logging.getLogger(__name__)


class TC_DD_3_23(MatterBaseTest):
    def desc_TC_DD_3_23(self) -> str:
        return " [TC-DD-3.23] NFC-based commissioning [DUT as Commissionee]"

    def steps_TC_DD_3_23(self) -> list[TestStep]:
        return [
            TestStep(1, "Detecting the NFC Tag and reading Payload", is_commissioning=False),
            TestStep(2, 'Check "Custom Flow" field of Onboarding data corresponds to "User-Intent commissioning flow",'
                        'perform user action required to enter commissioning mode'),
            TestStep(3, 'If "Custom Flow" field of Onboarding data corresponds to "Custom commissioning flow",'
                        'perform vendor-specified interactions to enter commissioning mode'),
            TestStep(4, "Bring or keep TH NFC reader close to the DUT’s NFC tag,"
                        "and initiate commissioning through NFC interface (NTL)"),
            TestStep(5, "Th Starts to re-check the DUT power state.")
        ]

    @async_test_body
    async def test_TC_DD_3_23(self):

        # Step 1: Begin commissioning with PASE and failsafe
        self.step(1)
        nfc_tag_data = connect_read_nfc_tag_data()
        self.matter_test_config.qr_code_content.append(nfc_tag_data)
        self.step(2)
        payload = SetupPayload().ParseQrCode(nfc_tag_data)
        logging.info(f"NFC Tag data read:{self.matter_test_config.qr_code_content}, length: {len(self.matter_test_config.qr_code_content)}")
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        print(self.matter_test_config.thread_operational_dataset)
        await self.commission_devices()


if __name__ == "__main__":
    default_matter_test_main()
