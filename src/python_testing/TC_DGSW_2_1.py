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
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)


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
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the ThreadMetrics attribute",
                     "The data type in each field of the struct must match the value listed in the specification(s) Table in 11.12.6.1"
                     "ThreadMetrics Struct. The Id field of the list is mandatory and shall match with Node assigned ID unique-per-thread."
                     "Name field shall be set to vendor specific strings. StackFreeCurrent field shall indicate the current amount of stack memory, in bytes, not being utilized by the respective thread."
                     "StackFreeMinimum field shall indicate the minimum amount of available stack memory in bytes. "
                     "This value can be reset by a node reboot. StackSize field shall indicate the amount of memory allocated for use by the respective thread."
                     "Verify that StackSize is greater than 0."
                     "Verify that StackFreeCurrent is less than StackSize."
                     "Verify that StackFreeMinimum is less than or equal to StackFreeCurrent."),
            TestStep(3, "Read the CurrentHeapFree attribute",
                     "The value will indicate the current amount of unutilized heap memory in bytes."
                     "Verify that CurrentHeapFree is greater than 0."),
            TestStep(4, "Read the CurrentHeapUsed attribute",
                     "The value will indicate the current amount of used heap memory in bytes."
                     "Verify that CurrentHeapUsed is greater than 0."),
            TestStep(5, "Read the CurrentHeapHighWatermark attribute",
                     "The value will indicate the maximum amount of heap memory being used in bytes. This value can be reset by a node reboot."
                     "Verify that CurrentHeapHighWatermark is greater than or equal to CurrentHeapUsed."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.SoftwareDiagnostics))
    async def test_TC_DGSW_2_1(self):

        endpoint = self.get_endpoint()

        # STEP 1: Commission DUT (already done)
        self.step(1)

        attributes = Clusters.SoftwareDiagnostics.Attributes

        # STEP 2: TH reads from the DUT the ThreadMetrics attribute
        self.step(2)
        attr_condition = await self.attribute_guard(endpoint=endpoint, attribute=attributes.ThreadMetrics)
        if attr_condition:
            thread_metrics_list = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.ThreadMetrics)

            # Validate each element in the thread_metrics_list
            for metric in thread_metrics_list:
                stackSize = None
                # The Id field is mandatory
                matter_asserts.assert_valid_uint64(metric.id, "Id")
                logger.info(f"Thread Metric ID: {metric.id}")

                # Validate the optional Name field
                if metric.name is not None:
                    matter_asserts.assert_is_string(metric.name, "Name")
                    logger.info(f"Thread Metric Name: {metric.name}")

                # Validate the optional StackSize field
                if metric.stackSize is not None:
                    stackSize = metric.stackSize
                    matter_asserts.assert_valid_uint32(stackSize, "StackSize")
                    # Verify that StackSize is greater than 0.
                    asserts.assert_greater(stackSize, 0, "StackSize is not greater than 0.")
                else:
                    logger.warning(f"Thread Metric StackSize is None for ID: {metric.id}")

                # Validate the optional StackFreeCurrent field
                if metric.stackFreeCurrent is not None:
                    matter_asserts.assert_valid_uint32(metric.stackFreeCurrent, "StackFreeCurrent")
                    # Verify that StackFreeCurrent is less than stackSize.
                    if stackSize is not None:
                        asserts.assert_less(metric.stackFreeCurrent, stackSize,
                                            "StackFreeCurrent is not less than StackSize.")
                else:
                    logger.warning(f"Thread Metric StackFreeCurrent is None for ID: {metric.id}")

                # Validate the optional StackFreeMinimum field
                if metric.stackFreeMinimum is not None:
                    matter_asserts.assert_valid_uint32(metric.stackFreeMinimum, "StackFreeMinimum")
                    # Verify that StackFreeMinimum is less than or equal to StackFreeCurrent.
                    if metric.stackFreeCurrent is not None:
                        asserts.assert_less_equal(metric.stackFreeMinimum, metric.stackFreeCurrent,
                                                  "StackFreeMinimum is not less than or equal to StackFreeCurrent.")
                else:
                    logger.warning(f"Thread Metric StackFreeMinimum is None for ID: {metric.id}")

        # STEP 3: TH reads from the DUT the CurrentHeapFree attribute
        self.step(3)
        attr_condition = await self.attribute_guard(endpoint=endpoint, attribute=attributes.CurrentHeapFree)
        if attr_condition:
            current_heap_free_attr = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapFree)
            logger.info(f"CurrentHeapFree: {current_heap_free_attr}")
            matter_asserts.assert_valid_uint64(current_heap_free_attr, "CurrentHeapFree")
            # Verify that CurrentHeapFree is greater than 0.
            asserts.assert_greater(current_heap_free_attr, 0, "CurrentHeapFree is not greater than 0.")

        # STEP 4: TH reads from the DUT the CurrentHeapUsed attribute
        self.step(4)

        current_heap_used_attr = 0
        attr_condition = await self.attribute_guard(endpoint=endpoint, attribute=attributes.CurrentHeapUsed)
        if attr_condition:
            current_heap_used_attr = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapUsed)
            logger.info(f"CurrentHeapUsed: {current_heap_used_attr}")
            matter_asserts.assert_valid_uint64(current_heap_used_attr, "CurrentHeapUsed")
            # Verify that CurrentHeapUsed is greater than 0.
            asserts.assert_greater(current_heap_used_attr, 0, "CurrentHeapUsed is not greater than 0.")

        # STEP 5: TH reads from the DUT the CurrentHeapHighWatermark attribute
        self.step(5)
        attr_condition = await self.attribute_guard(endpoint=endpoint, attribute=attributes.CurrentHeapHighWatermark)
        if attr_condition:
            current_heap_high_watermark_attr = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapHighWatermark)
            logger.info(f"CurrentHeapHighWatermark: {current_heap_high_watermark_attr}")
            matter_asserts.assert_valid_uint64(current_heap_high_watermark_attr, "CurrentHeapHighWatermark")
            # Verify that CurrentHeapHighWatermark is greater than or equal to CurrentHeapUsed.
            asserts.assert_greater_equal(current_heap_high_watermark_attr, current_heap_used_attr,
                                         "CurrentHeapHighWatermark is not greater than or equal to CurrentHeapUsed.")


if __name__ == "__main__":
    default_matter_test_main()
