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
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)


class TC_DGSW_2_3(MatterBaseTest):

    async def read_dgsw_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.SoftwareDiagnostics
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_DGSW_2_3(self) -> str:
        """Returns a description of this test"""
        return "[TC-DGSW-2.3] Command Received with Server as DUT"

    def pics_TC_DGSW_2_3(self) -> list[str]:
        return ["DGSW.S", "DGSW.S.F00"]

    def steps_TC_DGSW_2_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH if not already commissioned", is_commissioning=True),
            TestStep(2, "TH reads the ThreadMetrics attribute from DUT and saves as thread_metrics_original",
                     "Receive SUCCESS response from the DUT"),
            TestStep(3, "TH reads the CurrentHeapUsed attribute from the DUT and saves as current_heap_used_original.",
                     "Receive SUCCESS response from the DUT"),
            TestStep(4, "TH reads the CurrentHeapHighWatermark attribute from the DUT and saves as high_watermark_original.",
                     "Receive SUCCESS response from the DUT. "
                     "Verify that high_watermark_original is >= current_heap_used_original"),
            TestStep(5, "TH sends ResetWatermarks command to the DUT.",
                     "Receive SUCCESS response from the DUT"),
            TestStep("6a", "TH reads the CurrentHeapUsed attribute from the DUT and saves as current_heap_used.",
                     "Receive SUCCESS response from the DUT"),
            TestStep("6b", "TH reads the CurrentHeapHighWatermark attribute from the DUT and saves as high_watermark_current.",
                     "Receive SUCCESS response from the DUT. "
                     "Verify that high_watermark_current is >= current_heap_used"),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.SoftwareDiagnostics))
    async def test_TC_DGSW_2_3(self):

        endpoint = self.get_endpoint()
        attributes = Clusters.SoftwareDiagnostics.Attributes

        # STEP 1: Commission DUT to TH (already done)
        self.step(1)

        # STEP 2: TH reads the ThreadMetrics attribute from DUT
        self.step(2)
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ThreadMetrics):
            thread_metrics_original = await self.read_dgsw_attribute_expect_success(
                endpoint=endpoint, attribute=attributes.ThreadMetrics)
            matter_asserts.assert_list(thread_metrics_original, "ThreadMetrics")
            logger.info("ThreadMetrics (original): %s", thread_metrics_original)

        # STEP 3: TH reads the CurrentHeapUsed attribute from the DUT
        self.step(3)
        current_heap_used_original = None
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.CurrentHeapUsed):
            current_heap_used_original = await self.read_dgsw_attribute_expect_success(
                endpoint=endpoint, attribute=attributes.CurrentHeapUsed)
            matter_asserts.assert_valid_uint64(current_heap_used_original, "CurrentHeapUsed")
            logger.info("CurrentHeapUsed (original): %s", current_heap_used_original)

        # STEP 4: TH reads the CurrentHeapHighWatermark attribute from the DUT
        self.step(4)
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.CurrentHeapHighWatermark) and \
                await self.attribute_guard(endpoint=endpoint, attribute=attributes.CurrentHeapUsed):
            high_watermark_original = await self.read_dgsw_attribute_expect_success(
                endpoint=endpoint, attribute=attributes.CurrentHeapHighWatermark)
            matter_asserts.assert_valid_uint64(high_watermark_original, "CurrentHeapHighWatermark")
            logger.info("CurrentHeapHighWatermark (original): %s", high_watermark_original)
            asserts.assert_greater_equal(high_watermark_original, current_heap_used_original,
                                         "high_watermark_original is not greater than or equal to current_heap_used_original.")

        # STEP 5: TH sends ResetWatermarks command to the DUT
        self.step(5)
        await self.send_single_cmd(cmd=Clusters.SoftwareDiagnostics.Commands.ResetWatermarks(), endpoint=endpoint)

        # STEP 6a: TH reads the CurrentHeapUsed attribute from the DUT
        self.step("6a")
        current_heap_used = None
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.CurrentHeapUsed):
            current_heap_used = await self.read_dgsw_attribute_expect_success(
                endpoint=endpoint, attribute=attributes.CurrentHeapUsed)
            matter_asserts.assert_valid_uint64(current_heap_used, "CurrentHeapUsed")
            logger.info("CurrentHeapUsed (after ResetWatermarks): %s", current_heap_used)

        # STEP 6b: TH reads the CurrentHeapHighWatermark attribute from the DUT
        self.step("6b")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.CurrentHeapHighWatermark) and \
                await self.attribute_guard(endpoint=endpoint, attribute=attributes.CurrentHeapUsed):
            high_watermark_current = await self.read_dgsw_attribute_expect_success(
                endpoint=endpoint, attribute=attributes.CurrentHeapHighWatermark)
            matter_asserts.assert_valid_uint64(high_watermark_current, "CurrentHeapHighWatermark")
            logger.info("CurrentHeapHighWatermark (after ResetWatermarks): %s", high_watermark_current)
            asserts.assert_greater_equal(high_watermark_current, current_heap_used,
                                         "high_watermark_current is not greater than or equal to current_heap_used.")


if __name__ == "__main__":
    default_matter_test_main()
