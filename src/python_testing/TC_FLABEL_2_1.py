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

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, run_if_endpoint_matches, has_attribute, has_cluster, default_matter_test_main
from mobly import asserts


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

    @run_if_endpoint_matches(has_attribute(Clusters.FixedLabel.Attributes.LabelList) and has_cluster(Clusters.FixedLabel))
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

        # Step 3: Attempt to write LabelList (should fail)
        self.step(3)
        test_label = Clusters.Objects.FixedLabel.Attributes.LabelList(
            [Clusters.Objects.FixedLabel.Structs.LabelStruct(
                label="Test_Label",
                value="Test_Value"
            )]
        )

        # Use write_single_attribute with expect_success=False since we expect it to fail
        write_status = await self.write_single_attribute(
            attribute_value=test_label,
            expect_success=False
        )
        asserts.assert_equal(write_status, Status.UnsupportedWrite, "Expected UNSUPPORTED_WRITE status")

        # Step 4: Verify LabelList hasn't changed
        self.step(4)
        final_labels = await self.read_single_attribute_check_success(
            cluster=Clusters.Objects.FixedLabel,
            attribute=Clusters.Objects.FixedLabel.Attributes.LabelList
        )
        asserts.assert_equal(initial_labels, final_labels,
                             "LabelList should remain unchanged after write attempt")


if __name__ == "__main__":
    default_matter_test_main()
