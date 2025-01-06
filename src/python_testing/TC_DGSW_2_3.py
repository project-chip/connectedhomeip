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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_DGSW_2_3(MatterBaseTest):

    @staticmethod
    def is_valid_uint64_value(value):
        return isinstance(value, int) and 0 <= value <= 0xFFFFFFFFFFFFFFFF

    @staticmethod
    def is_valid_uint32_value(value):
        return isinstance(value, int) and 0 <= value <= 0xFFFFFFFF

    async def read_dgsw_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.SoftwareDiagnostics
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_reset_watermarks_command(self):
        endpoint = 0
        diags_cluster = Clusters.Objects.SoftwareDiagnostics

        try:
            response = await self.send_single_cmd(cmd=diags_cluster.Commands.ResetWatermarks(), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        return response

    def desc_TC_DGSW_2_3(self) -> str:
        """Returns a description of this test"""
        return "[TC-DGSW-2.1] Attributes with Server as DUT"

    def pics_TC_DGSW_2_3(self) -> list[str]:
        return ["DGSW.S"]

    def steps_TC_DGSW_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the AttributeList attribute"),
            TestStep(3, "Read the ThreadMetrics attribute"),
            TestStep(4, "Read the CurrentHeapHighWatermark attribute"),
            TestStep(5, "Read the CurrentHeapUsed attribute"),
            TestStep(6, "Send ResetWatermarks command"),
            TestStep(7, "Read the CurrentHeapHighWatermark attribute"),
            TestStep(8, "Read the ThreadMetrics attribute"),
        ]
        return steps

    @async_test_body
    async def test_TC_DGSW_2_3(self):

        endpoint = self.get_endpoint(default=0)

        # STEP 1: Commission DUT (already done)
        self.step(1)

        # STEP 2: TH reads from the DUT the AttributeList attribute
        self.step(2)
        attributes = Clusters.SoftwareDiagnostics.Attributes
        attribute_list = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        # STEP 3: TH reads from the DUT the ThreadMetrics attribute
        self.step(3)
        if self.pics_guard(attributes.ThreadMetrics.attribute_id in attribute_list):
            thread_metrics_original = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.ThreadMetrics)
            # the Id field is mandatory
            asserts.assert_true(self.is_valid_uint64_value(thread_metrics_original[0].id), "Id field should be a uint64 type")
            if thread_metrics_original[0].name is not None:
                asserts.assert_true(thread_metrics_original[0].name, str, "Name field should be a string type")
            if thread_metrics_original[0].stackFreeCurrent is not None:
                asserts.assert_true(self.is_valid_uint32_value(
                    thread_metrics_original[0].stackFreeCurrent), "StackFreeCurrent field should be a uint32 type")
            if thread_metrics_original[0].stackFreeMinimum is not None:
                asserts.assert_true(self.is_valid_uint32_value(
                    thread_metrics_original[0].stackFreeMinimum), "StackFreeMinimum field should be a uint32 type")
            if thread_metrics_original[0].stackSize is not None:
                asserts.assert_true(self.is_valid_uint32_value(
                    thread_metrics_original[0].stackSize), "StackSize field should be a uint32s type")

        # STEP 4: TH reads from the DUT the CurrentHeapHighWatermark attribute
        self.step(4)
        if self.pics_guard(attributes.CurrentHeapHighWatermark.attribute_id in attribute_list):
            high_watermark_original = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapHighWatermark)
            asserts.assert_true(self.is_valid_uint64_value(high_watermark_original),
                                "CurrentHeapHighWatermark field should be a uint64 type")

        # STEP 5: TH reads from the DUT the CurrentHeapUsed attribute
        self.step(5)
        if self.pics_guard(attributes.CurrentHeapUsed.attribute_id in attribute_list):
            current_heap_used_original = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapUsed)
            asserts.assert_true(self.is_valid_uint64_value(current_heap_used_original),
                                "CurrentHeapUsed field should be a uint64 type")
            if high_watermark_original is not None:
                asserts.assert_true(current_heap_used_original <= high_watermark_original,
                                    "CurrentHeapUsed should be less than or equal to CurrentHeapHighWatermark")

        # STEP 6: TH sends to the DUT the  command
        self.step(6)
        response = await self.send_reset_watermarks_command()
        logging.info(f"ResetWatermarks response: {response}")

        # STEP 7: TH reads from the DUT the CurrentHeapHighWatermark attribute
        self.step(7)
        if self.pics_guard(attributes.CurrentHeapHighWatermark.attribute_id in attribute_list):
            current_heap_high_watermark = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentHeapHighWatermark)
            asserts.assert_true(self.is_valid_uint64_value(current_heap_high_watermark),
                                "CurrentHeapHighWatermark field should be a uint64 type")

            # Verify that the returned value is <= high_watermark_original
            asserts.assert_true(current_heap_high_watermark <= high_watermark_original,
                                f"CurrentHeapHighWatermark ({current_heap_high_watermark}) should be <= high_watermark_original ({high_watermark_original})")

            # If CurrentHeapUsed is supported
            if current_heap_used_original is not None and current_heap_used_original < high_watermark_original:
                # Verify that the returned value is < high_watermark_original
                asserts.assert_true(current_heap_high_watermark < high_watermark_original,
                                    f"CurrentHeapHighWatermark ({current_heap_high_watermark}) should be < high_watermark_original ({high_watermark_original}) when CurrentHeapUsed ({current_heap_used_original}) is less than high_watermark_original")

        # STEP 8: TH reads from the DUT the ThreadMetrics attribute
        self.step(8)
        if self.pics_guard(Clusters.SoftwareDiagnostics.Attributes.ThreadMetrics.attribute_id in attribute_list):
            thread_metrics_reset = await self.read_dgsw_attribute_expect_success(endpoint=endpoint, attribute=attributes.ThreadMetrics)
            # the Id field is mandatory
            asserts.assert_true(self.is_valid_uint64_value(thread_metrics_reset[0].id), "Id field should be a uint64 type")
            if thread_metrics_reset[0].name is not None:
                asserts.assert_true(thread_metrics_reset[0].name, str, "Name field should be a string type")
            if thread_metrics_reset[0].stackFreeCurrent is not None:
                asserts.assert_true(self.is_valid_uint32_value(
                    thread_metrics_reset[0].stackFreeCurrent), "StackFreeCurrent field should be a uint32 type")
            if thread_metrics_reset[0].stackFreeMinimum is not None:
                asserts.assert_true(self.is_valid_uint32_value(
                    thread_metrics_reset[0].stackFreeMinimum), "StackFreeMinimum field should be a uint32 type")
            if thread_metrics_reset[0].stackSize is not None:
                asserts.assert_true(self.is_valid_uint32_value(
                    thread_metrics_reset[0].stackSize), "StackSize field should be a uint32s type")

            # Ensure the list length matches thread_metrics_original to simplify matching
            asserts.assert_equal(len(thread_metrics_reset), len(thread_metrics_original),
                                 "Mismatch in the number of items between thread_metrics_reset and thread_metrics_original")

            # Compare each item in the list
            for reset_metric in thread_metrics_reset:
                # Find the corresponding item in thread_metrics_original by ID
                original_metric = next((m for m in thread_metrics_original if m.id == reset_metric.id), None)
                asserts.assert_is_not_none(original_metric,
                                           f"No matching ThreadMetrics entry found in thread_metrics_original for ID {reset_metric.id}")

                # Compare StackFreeMinimum if present in both
                if original_metric.stackFreeMinimum is not None and reset_metric.stackFreeMinimum is not None:
                    asserts.assert_true(
                        reset_metric.stackFreeMinimum >= original_metric.stackFreeMinimum,
                        f"StackFreeMinimum for ID {reset_metric.id} in thread_metrics_reset ({reset_metric.stackFreeMinimum}) "
                        f"should be >= corresponding value in thread_metrics_original ({original_metric.stackFreeMinimum})"
                    )


if __name__ == "__main__":
    default_matter_test_main()
