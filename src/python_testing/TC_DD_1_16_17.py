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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --bool-arg post_cert_test:true
#       --qr-code MT:-24J0KCZ16N71648G00
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter import discovery
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_DD_1_16_17(MatterBaseTest):
    ''' TC-DD-1.16 and TC-DD-1.17

        TC-DD-1.16
        This is the test for manual code. This test includes a request to put the custom and user-intent
        commissioning devices into a commissionable mode because this test needs to be run for every device
        with a setup code. This test does NOT verify that user-intent commissioning flow devices do not
        advertise out of box because they cannot be differentiated from custom commissioning flow devices
        from the manual pairing code. That check is done in TC-DD-1.17

        TC-DD-1.17
        This is the test for the QR code. This test does NOT include a request to put the device into commissionable
        mode since devices with a QR code will necessarily include a manual code as well per the spec guidelines. This
        test DOES include a check that user-intent commissioning flow devices do not advertise on startup.
    '''

    def pics_TC_DD_1_16(self):
        return ['MCORE.DD.MANUAL_PC']

    def steps_TC_DD_1_16(self):
        return [TestStep(1, "TH parses the manual code"),
                TestStep(2, "If the VID_PID_PRESENT field is set to 0, this device uses standard flow. Verify that the DUT is advertising as Commissionable",
                         "Device is advertising as commissionable"),
                TestStep(3, "If the device uses custom flow or user-intent commissioning, ask the tester to place the device into commissionable mode"),
                TestStep(4, "If the device uses custom flow or user-intent commissioning, verify that the DUT is advertising as commisionable"),
                ]

    @async_test_body
    async def test_TC_DD_1_16(self):
        asserts.assert_true(self.matter_test_config.manual_code,
                            "This test needs to be run with the manual setup code.")

        self.step(1)

        # need to use establish pase session here so we check over BLE and mdns
        # Check with the NFC folks how this is supposed to work
        self.mark_all_remaining_steps_skipped(2)

    def steps_TC_DD_1_17(self):
        return [TestStep(1, "TH parses the QR code"),
                TestStep(2, "If the Custom Flow field is set to 0, this device uses standard flow. Verify that the DUT is advertising as Commissionable",
                         "Device is advertising as commissionable"),
                TestStep(2, "If the Custom Flow field is set to 1, this device uses user-intent flow. Verify that the DUT is NOT advertising as Commissionable",
                         "Device is not advertising as commissionable")
                ]


if __name__ == "__main__":
    default_matter_test_main()
