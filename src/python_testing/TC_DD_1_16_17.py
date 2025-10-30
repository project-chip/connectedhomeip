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
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --custom-flow 1
#       --app-pipe /tmp/tmp_pipe
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --manual-code 500549123365521327696
#       --qr-code MT:-24J0MH312-10648G00
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --app-pipe /tmp/tmp_pipe
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
#   run2:
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
#       --qr-code MT:-24J0Q1212-10648G00
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --app-pipe /tmp/tmp_pipe
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#
# Test runs twice - once with standard flow, once with user-intent flow

from mobly import asserts

import matter.discovery
from matter.setup_payload import SetupPayload
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

# TODO: set the default timeout to be larger because there's a manual step
# TODO: force the linux app to actually not advertise when it's in user intent mode and add a trigger to start advertising


class TC_DD_1_16_17(MatterBaseTest):
    ''' TC-DD-1.16 and TC-DD-1.17

        TC-DD-1.16
        This is the test for the QR code. This test does NOT include a request to put the device into commissionable
        mode since devices with a QR code will necessarily include a manual code as well per the spec guidelines. This
        test DOES include a check that user-intent commissioning flow devices do not advertise on startup.

        TC-DD-1.17
        This is the test for manual code. This test includes a request to put the custom and user-intent
        commissioning devices into a commissionable mode because this test needs to be run for every device
        with a setup code. This test does NOT verify that user-intent commissioning flow devices do not
        advertise out of box because they cannot be differentiated from custom commissioning flow devices
        from the manual pairing code. That check is done in TC-DD-1.16
    '''

    def steps_TC_DD_1_16(self):
        return [TestStep(1, "TH parses the QR code"),
                TestStep(2, "If the Custom Flow field is set to 0, this device uses standard flow. Verify that the DUT is advertising as Commissionable",
                         "Device is advertising as commissionable"),
                TestStep(3, "If the Custom Flow field is set to 1, this device uses user-intent flow. Verify that the DUT is NOT advertising as Commissionable",
                         "Device is not advertising as commissionable")
                ]

    def pics_TC_DD_1_16(self):
        return ['MCORE.DD.QR']

    @async_test_body
    async def test_TC_DD_1_16(self):

        self.step(1)
        asserts.assert_true(self.matter_test_config.qr_code_content,
                            "This test needs to be run with the qr setup code.")
        parsed = SetupPayload().ParseQrCode(self.matter_test_config.qr_code_content[0])

        self.step(2)
        if parsed.commissioning_flow == 0:
            # Standard commissioning flow - this should be advertising
            await self.ensure_advertising(filter_type=matter.discovery.FilterType.LONG_DISCRIMINATOR, filter=parsed.long_discriminator)
        else:
            self.mark_current_step_skipped()

        self.step(3)
        if parsed.commissioning_flow == 1:
            responses = await self.default_controller.DiscoverCommissionableNodes(filterType=matter.discovery.FilterType.LONG_DISCRIMINATOR, filter=parsed.long_discriminator)
            asserts.assert_equal(responses, [], "Device with user-intent commissioning flow should not be advertising")
        else:
            self.mark_current_step_skipped()

    def pics_TC_DD_1_17(self):
        return ['MCORE.DD.MANUAL_PC']

    def steps_TC_DD_1_17(self):
        return [TestStep(1, "TH parses the manual code"),
                TestStep(2, "If the VID_PID_PRESENT field is set to 0, this device uses standard flow. Verify that the DUT is advertising as Commissionable",
                         "Device is advertising as commissionable"),
                TestStep(3, "If the device uses custom flow or user-intent commissioning, ask the tester to place the device into commissionable mode"),
                TestStep(4, "If the device uses custom flow or user-intent commissioning, verify that the DUT is advertising as commissionable",
                         "Device is advertising as commissionable"),
                ]

    async def ensure_advertising(self, filter_type: matter.discovery.FilterType, filter: int):
        responses = await self.default_controller.DiscoverCommissionableNodes(filterType=filter_type, filter=filter, stopOnFirst=True)
        asserts.assert_greater_equal(len(responses), 1, "Device should be advertising as commissionable")

    @async_test_body
    async def test_TC_DD_1_17(self):
        self.step(1)
        asserts.assert_true(self.matter_test_config.manual_code,
                            "This test needs to be run with the manual setup code.")
        parsed = SetupPayload().ParseManualPairingCode(self.matter_test_config.manual_code[0])
        has_product_id = int(parsed.product_id) != 0
        has_vendor_id = int(parsed.vendor_id) != 0
        asserts.assert_equal(has_product_id, has_vendor_id, "Product and vendor ID must either both be included or both be omitted")
        standard_flow = not has_vendor_id and not has_product_id

        self.step(2)
        if standard_flow:
            # Device should be advertising
            await self.ensure_advertising(filter_type=matter.discovery.FilterType.SHORT_DISCRIMINATOR, filter=parsed.short_discriminator)
        else:
            self.mark_current_step_skipped()

        self.step(3)
        if not standard_flow:
            if self.is_pics_sdk_ci_only:
                command_dict = {"Name": 'UserIntentCommissioningStart'}
                self.write_to_app_pipe(command_dict)
            else:
                self.wait_for_user_input("Please perform any actions needed to put the device into commissionable mode")
        else:
            self.mark_current_step_skipped()

        self.step(4)
        if not standard_flow:
            await self.ensure_advertising(filter_type=matter.discovery.FilterType.SHORT_DISCRIMINATOR, filter=parsed.short_discriminator)
        else:
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
