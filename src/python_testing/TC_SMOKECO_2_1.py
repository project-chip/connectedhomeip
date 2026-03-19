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
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

from TC_SMOKECOTestBase import SmokeCoBaseTest

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_SMOKECO_2_1(SmokeCoBaseTest):

    def desc_TC_SMOKECO_2_1(self) -> str:
        return "[TC-SMOKECO-2.7] Unmount Attribute with DUT as Server"

    def steps_TC_SMOKECO_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH"),
            TestStep(2, "TH reads from the DUT the ExpressedState attribute.",
                     "Verify that the DUT response contains a value between 0 and 9"),
            TestStep(3, "TH reads from the DUT the SmokeState attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(4, "TH reads from the DUT the COState attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(5, "TH reads from the DUT the BatteryAlert attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(6, "TH reads from the DUT the DeviceMuted attribute.",
                     "Verify that the DUT response contains a value between 0 and 1"),
            TestStep(7, "TH reads from the DUT the TestInProgress attribute.",
                     "Verify that the DUT response contains a bool value"),
            TestStep(8, "TH reads from the DUT the HardwareFaultAlert attribute.",
                     "Verify that the DUT response contains a bool value"),
            TestStep(9, "TH reads from the DUT the EndOfServiceAlert attribute.",
                     "Verify that the DUT response contains a value between 0 and 1"),
            TestStep(10, "TH reads from the DUT the InterconnectSmokeAlarm attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(11, "TH reads from the DUT the InterconnectCOAlarm attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(12, "TH reads from the DUT the ContaminationState attribute.",
                     "Verify that the DUT response contains a value between 0 and 3"),
            TestStep(13, "TH reads from the DUT the SmokeSensitivityLevel attribute.",
                     "Verify that the DUT response contains a value between 0 and 2"),
            TestStep(14, "TH reads from the DUT the ExpiryDate attribute.", "Verify that the DUT response contains epoch-s format"),
            TestStep(15, "TH reads from the DUT the Unmounted attribute.", "Verify that the DUT response contains a bool value"),
        ]

    def pics_TC_SMOKECO_2_1(self) -> list[str]:
        return [
            "SMOKECO.S.A000d",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.SmokeCoAlarm))
    async def test_TC_SMOKECO_2_1(self):

        # Step 1, "Commission DUT to TH."
        self.step(1)  # Commissioning already done

        self.step(2)
        await self.read_attribute_check_range(self.smokeco_cluster.Attributes.ExpressedState, 0, 9)

        self.step(3)
        await self.read_attribute_check_range(self.smokeco_cluster.Attributes.SmokeState, 0, 2)

        self.step(4)
        await self.read_attribute_check_range(self.smokeco_cluster.Attributes.COState, 0, 2)

        self.step(5)
        await self.read_attribute_check_range(self.smokeco_cluster.Attributes.BatteryAlert, 0, 2)

        self.step(6)
        if self.attribute_guard(endpoint=self.get_endpoint(), attribute=self.smokeco_cluster.Attributes.DeviceMuted):
            await self.read_attribute_check_range(self.smokeco_cluster.Attributes.DeviceMuted, 0, 2)

        self.step(7)
        await self.read_attribute_check_bool(self.smokeco_cluster.Attributes.TestInProgress)

        self.step(8)
        await self.read_attribute_check_bool(self.smokeco_cluster.Attributes.HardwareFaultAlert)

        self.step(9)
        await self.read_attribute_check_range(self.smokeco_cluster.Attributes.EndOfServiceAlert, 0, 1)

        self.step(10)
        if self.attribute_guard(endpoint=self.get_endpoint(), attribute=self.smokeco_cluster.Attributes.InterconnectSmokeAlarm):
            await self.read_attribute_check_range(self.smokeco_cluster.Attributes.InterconnectSmokeAlarm, 0, 2)

        self.step(11)
        if self.attribute_guard(endpoint=self.get_endpoint(), attribute=self.smokeco_cluster.Attributes.InterconnectCOAlarm):
            await self.read_attribute_check_range(self.smokeco_cluster.Attributes.InterconnectCOAlarm, 0, 2)

        self.step(12)
        await self.read_attribute_check_range(self.smokeco_cluster.Attributes.ContaminationState, 0, 3)

        self.step(13)
        await self.read_attribute_check_range(self.smokeco_cluster.Attributes.SmokeSensitivityLevel, 0, 2)

        self.step(14)
        if self.attribute_guard(endpoint=self.get_endpoint(), attribute=self.smokeco_cluster.Attributes.ExpiryDate):
            await self.read_attribute_check_epoch(self.smokeco_cluster.Attributes.ExpiryDate)

        self.step(15)
        if self.attribute_guard(endpoint=self.get_endpoint(), attribute=self.smokeco_cluster.Attributes.Unmounted):
            await self.read_attribute_check_bool(self.smokeco_cluster.Attributes.Unmounted)


if __name__ == "__main__":
    default_matter_test_main()
