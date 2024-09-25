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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===
#

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_DGSW_2_1(MatterBaseTest):

    @staticmethod
    def is_valid_uint64_value(value):
        return isinstance(value, int) and 0 <= value <= 0xFFFFFFFFFFFFFFFF

    @staticmethod
    def is_valid_uint32_value(value):
        return isinstance(value, int) and 0 <= value <= 0xFFFFFFFF

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

        endpoint = self.user_params.get("endpoint", 0)

        # STEP 1: Commission DUT (already done)
        self.step(1)

        attributes = Clusters.SoftwareDiagnostics.Attributes
        attribute_list = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        # STEP 2: TH reads from the DUT the ThreadMetrics attribute
        self.step(2)
        if self.pics_guard(Clusters.SoftwareDiagnostics.Attributes.ThreadMetrics.attribute_id in attribute_list):
            thread_metrics_list = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.ThreadMetrics)
            # the Id field is mandatory
            asserts.assert_true(self.is_valid_uint64_value(thread_metrics_list[0].id), "Id field should be a uint64 type")
            if thread_metrics_list[0].name is not None:
                asserts.assert_true(thread_metrics_list[0].name, str, "Name field should be a string type")
            if thread_metrics_list[0].stackFreeCurrent is not None:
                asserts.assert_true(self.is_valid_uint32_value(
                    thread_metrics_list[0].stackFreeCurrent), "StackFreeCurrent field should be a uint32 type")
            if thread_metrics_list[0].stackFreeMinimum is not None:
                asserts.assert_true(self.is_valid_uint32_value(
                    thread_metrics_list[0].stackFreeMinimum), "StackFreeMinimum field should be a uint32 type")
            if thread_metrics_list[0].stackSize is not None:
                asserts.assert_true(self.is_valid_uint32_value(
                    thread_metrics_list[0].stackSize), "StackSize field should be a uint32s type")

        # STEP 3: TH reads from the DUT the CurrentHeapFree attribute
        self.step(3)
        if self.pics_guard(Clusters.SoftwareDiagnostics.Attributes.CurrentHeapFree.attribute_id in attribute_list):
            current_heap_free_attr = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapFree)
            asserts.assert_true(self.is_valid_uint64_value(current_heap_free_attr), "CurrentHeapFree field should be a uint64 type")

        # STEP 4: TH reads from the DUT the CurrentHeapUsed attribute
        self.step(4)
        if self.pics_guard(Clusters.SoftwareDiagnostics.Attributes.CurrentHeapUsed.attribute_id in attribute_list):
            current_heap_used_attr = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapUsed)
            asserts.assert_true(self.is_valid_uint64_value(current_heap_used_attr), "CurrentHeapUsed field should be a uint64 type")

        # STEP 5: TH reads from the DUT the CurrentHeapHighWatermark attribute
        self.step(5)
        if self.pics_guard(Clusters.SoftwareDiagnostics.Attributes.CurrentHeapHighWatermark.attribute_id in attribute_list):
            current_heap_high_watermark_attr = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapHighWatermark)
            asserts.assert_true(self.is_valid_uint64_value(current_heap_high_watermark_attr),
                                "CurrentHeapHighWatermark field should be a uint64 type")


if __name__ == "__main__":
    default_matter_test_main()
