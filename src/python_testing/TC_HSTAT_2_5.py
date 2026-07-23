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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 0
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
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

# Auto-generated from test specification: [TC-HSTAT-2.5] Mist functionality with DUT as Server


class TC_HSTAT_2_5(MatterBaseTest):

    def pics_TC_HSTAT_2_5(self) -> list[str]:
        return [
            "HSTAT.S",
        ]

    def desc_TC_HSTAT_2_5(self) -> str:
        return "[TC-HSTAT-2.5] Mist functionality with DUT as Server"

    def steps_TC_HSTAT_2_5(self):
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", ""),
            TestStep(2, "TH sends command On to the On/Off cluster on the same endpoint as this cluster.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(3, "TH sends command SetSettings with the Mode field set to Humidifier",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(4, "TH sends command SetSettings with the Mode field set to Humidifier",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(5, "TH sends command SetSettings with only the MistWarm bit of the MistType field set",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(6, "TH reads from the DUT the MistType attribute.",
                     "Verify that the DUT response contains a value with the MistWarm bit set. Store the value as MistSetting"),
            TestStep(7, "TH writes to the DUT the MistType attribute with only the MistCold bit set",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(8, "TH reads from the DUT the MistType attribute.",
                     "Verify that the DUT response contains a value with the MistCold bit set."),
            TestStep(9, "Individually subscribe to the MistType attribute",
                     "This will receive updates when these attributes change value."),
            TestStep(10, "TH writes to the DUT the MistType attribute with only the MistWarm bit of the MistType field set",
                     "Verify that the DUT response contains a value with only the MistWarm bit set. Verify that an attribute report was received for MistType and that the value received has only the MistWarm bit set."),
            TestStep(11, "TH sends command SetSettings with the MistType field set to MistWarm",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(12, "TH sends command SetSettings with the MistType field set to MistCold",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(13, "TH sends command Off to the On/Off cluster on the same endpoint as this cluster.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(14, "TH reads from the DUT the MistType attribute.", "Verify that the DUT response contains the NULL value."),
        ]

    @async_test_body
    async def test_TC_HSTAT_2_5(self):
        self.step(1)
        # Commission DUT to TH (can be skipped if done in a preceding test).
        #

        self.step(2)
        # TH sends command On to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(3)
        # TH sends command SetSettings with the Mode field set to Humidifier
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(4)
        # TH sends command SetSettings with the Mode field set to Humidifier
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(5)
        # TH sends command SetSettings with only the MistWarm bit of the MistType field set
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(6)
        # TH reads from the DUT the MistType attribute.
        # Verify that the DUT response contains a value with the MistWarm bit set. Store the value as MistSetting

        self.step(7)
        # TH writes to the DUT the MistType attribute with only the MistCold bit set
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(8)
        # TH reads from the DUT the MistType attribute.
        # Verify that the DUT response contains a value with the MistCold bit set.

        self.step(9)
        # Individually subscribe to the MistType attribute
        # This will receive updates when these attributes change value.

        self.step(10)
        # TH writes to the DUT the MistType attribute with only the MistWarm bit of the MistType field set
        # Verify that the DUT response contains a value with only the MistWarm bit set. Verify that an attribute report was received for MistType and that the value received has only the MistWarm bit set.

        self.step(11)
        # TH sends command SetSettings with the MistType field set to MistWarm
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)

        self.step(12)
        # TH sends command SetSettings with the MistType field set to MistCold
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)

        self.step(13)
        # TH sends command Off to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(14)
        # TH reads from the DUT the MistType attribute.
        # Verify that the DUT response contains the NULL value.


if __name__ == '__main__':
    default_matter_test_main()
