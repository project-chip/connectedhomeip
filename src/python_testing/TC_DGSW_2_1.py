#
#    Copyright (c) 2024 Project CHIP Authors
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
#     app-args: --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import chip.clusters as Clusters
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_DGSW_2_1(MatterBaseTest):

    async def read_dgsw_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.SoftwareDiagnostics
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_DGSW_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC-DGSW-2.1] Attributes with Server as DUT"

    def pics_TC_DGSW_2_1(self) -> list[str]:
        return ["DGSW.S"]

    def steps_TC_DGSW_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the ThreadMetrics attribute"),
            TestStep(3, "Read the CurrentHeapFree attribute"),
            TestStep(4, "Read the CurrentHeapUsed attribute"),
            TestStep(5, "Read the CurrentHeapHighWatermark attribute"),
        ]
        return steps

    @async_test_body
    async def test_TC_DGSW_2_1(self):

        endpoint = self.get_endpoint(default=0)

        # STEP 1: Commission DUT (already done)
        self.step(1)

        attributes = Clusters.SoftwareDiagnostics.Attributes
        attribute_list = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        # STEP 2: TH reads from the DUT the ThreadMetrics attribute
        self.step(2)
        if self.pics_guard(attributes.ThreadMetrics.attribute_id in attribute_list):
            thread_metrics_list = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.ThreadMetrics)

            # Validate each element in the thread_metrics_list
            for metric in thread_metrics_list:
                # The Id field is mandatory
                matter_asserts.assert_valid_uint64(metric.id, "Id")

                # Validate the optional Name field
                if metric.name is not None:
                    matter_asserts.assert_is_string(metric.name, "Name")

                # Validate the optional StackFreeCurrent field
                if metric.stackFreeCurrent is not None:
                    matter_asserts.assert_valid_uint32(metric.stackFreeCurrent, "StackFreeCurrent")

                # Validate the optional StackFreeMinimum field
                if metric.stackFreeMinimum is not None:
                    matter_asserts.assert_valid_uint32(metric.stackFreeMinimum, "StackFreeMinimum")

                # Validate the optional StackSize field
                if metric.stackSize is not None:
                    matter_asserts.assert_valid_uint32(metric.stackSize, "StackSize")

        # STEP 3: TH reads from the DUT the CurrentHeapFree attribute
        self.step(3)
        if self.pics_guard(attributes.CurrentHeapFree.attribute_id in attribute_list):
            current_heap_free_attr = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapFree)
            matter_asserts.assert_valid_uint64(current_heap_free_attr, "CurrentHeapFree")

        # STEP 4: TH reads from the DUT the CurrentHeapUsed attribute
        self.step(4)
        if self.pics_guard(attributes.CurrentHeapUsed.attribute_id in attribute_list):
            current_heap_used_attr = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapUsed)
            matter_asserts.assert_valid_uint64(current_heap_used_attr, "CurrentHeapUsed")

        # STEP 5: TH reads from the DUT the CurrentHeapHighWatermark attribute
        self.step(5)
        if self.pics_guard(attributes.CurrentHeapHighWatermark.attribute_id in attribute_list):
            current_heap_high_watermark_attr = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapHighWatermark)
            matter_asserts.assert_valid_uint64(current_heap_high_watermark_attr, "CurrentHeapHighWatermark")


if __name__ == "__main__":
    default_matter_test_main()
