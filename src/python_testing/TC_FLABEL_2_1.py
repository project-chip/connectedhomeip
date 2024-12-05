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

import logging
import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.interaction_model import Status
from mobly import asserts

logger = logging.getLogger(__name__)


class Test_TC_FLABEL_2_1(MatterBaseTest):
    def pics_TC_FLABEL_2_1(self) -> list[str]:
        return ["FLABEL.S"]

    def steps_TC_FLABEL_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads LabelList from the DUT", "Read is successful"),
            TestStep(3, "TH tries to write LabelList attribute", "Write fails with UNSUPPORTED_WRITE"),
            TestStep(4, "Verify LabelList hasn't changed", "LabelList matches initial read")
        ]

    @async_test_body
    async def test_TC_FLABEL_2_1(self):
        # Step 1: Commission DUT (already done)
        self.step(1)

        # Step 2: Read LabelList attribute
        self.step(2)
        initial_labels = await self.read_single_attribute_check_success(
            cluster=Clusters.Objects.FixedLabel,
            attribute=Clusters.Objects.FixedLabel.Attributes.LabelList
        )
        asserts.assert_true(isinstance(initial_labels, list), "LabelList should be a list type")
        logger.info(f"Initial LabelList: {initial_labels}")

        # Step 3: Attempt to write LabelList (should fail)
        self.step(3)
        test_label = [Clusters.Objects.FixedLabel.Structs.LabelStruct(
            label="Test_Label",
            value="Test_Value"
        )]
        logger.info(f"Attempting to write LabelList: {test_label}")

        try:
            result = await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(1, Clusters.Objects.FixedLabel.Attributes.LabelList(test_label))]
            )
            logger.info(f"Write result: {result}")
            logger.info(f"Write status: {result[0]}")
            asserts.assert_equal(result[0].Status, Status.UnsupportedWrite, "Expected UNSUPPORTED_WRITE status")
        except Exception as e:
            logger.error(f"Unexpected error during write: {str(e)}")
            asserts.fail(f"Unexpected error during write: {str(e)}")

        # Step 4: Verify LabelList hasn't changed
        self.step(4)
        final_labels = await self.read_single_attribute_check_success(
            cluster=Clusters.Objects.FixedLabel,
            attribute=Clusters.Objects.FixedLabel.Attributes.LabelList
        )
        logger.info(f"Final LabelList: {final_labels}")

        asserts.assert_equal(initial_labels, final_labels,
                             "LabelList should remain unchanged after write attempt")


if __name__ == "__main__":
    default_matter_test_main()
