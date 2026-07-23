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

# Auto-generated from test specification: [TC-HSTAT-2.6] Negative mist test cases with DUT as Server


class TC_HSTAT_2_6(MatterBaseTest):

    def pics_TC_HSTAT_2_6(self) -> list[str]:
        return [
            "HSTAT.S",
        ]

    def desc_TC_HSTAT_2_6(self) -> str:
        return "[TC-HSTAT-2.6] Negative mist test cases with DUT as Server"

    def steps_TC_HSTAT_2_6(self):
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", ""),
            TestStep(2, "TH sends command On to the On/Off cluster on the same endpoint as this cluster.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(3, "TH sends command SetSettings with the Mode field set to Dehumidifier",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(4, "TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(5, "TH sends command SetSettings with the MistType field set to MistCold",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(6, "TH sends command SetSettings with the MistType field set to MistWarm",
                     "Verify DUT responds w/ status INVALID_IN_STATE(0xcb)"),
            TestStep(7, "TH reads from the DUT the MistType attribute.", "Verify that the DUT response contains the NULL value."),
        ]

    @async_test_body
    async def test_TC_HSTAT_2_6(self):
        self.step(1)
        # Commission DUT to TH (can be skipped if done in a preceding test).
        #

        self.step(2)
        # TH sends command On to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(3)
        # TH sends command SetSettings with the Mode field set to Dehumidifier
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(4)
        # TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(5)
        # TH sends command SetSettings with the MistType field set to MistCold
        # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)

        self.step(6)
        # TH sends command SetSettings with the MistType field set to MistWarm
        # Verify DUT responds w/ status INVALID_IN_STATE(0xcb)

        self.step(7)
        # TH reads from the DUT the MistType attribute.
        # Verify that the DUT response contains the NULL value.


if __name__ == '__main__':
    default_matter_test_main()
