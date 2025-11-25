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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md"#defining-the-ci-test-arguments
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
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging


import matter.clusters as Clusters
from matter.clusters import ClusterObjects
from matter.testing import matter_asserts
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_DISHALM_2_1(MatterBaseTest):

    def desc_TC_DISHALM_2_1(self) -> str:
        return "198.2.1. [TC-DISHALM-2.1] Attributes with DUT as Server"

    def pics_TC_DISHALM_2_1(self):
        pics = [
            "DISHALM.S"
        ]
        return pics

    def steps_TC_DISHALM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the Mask attribute", "Verify that the DUT response contains a 32-bit value"),
            TestStep(3, "TH reads from the DUT the Latch attribute", "Verify that the DUT response contains a 32-bit value"),
            TestStep(4, "TH reads from the DUT the State attribute", "Verify that the DUT response contains a 32-bit value"),
            TestStep(5, "TH reads from the DUT the Supported attribute", "Verify that the DUT response contains a 32-bit value")
        ]
        return steps

    async def read_and_check_attributes_from_dishwasher_alarm(self, attribute: ClusterObjects.ClusterAttributeDescriptor):
        resp = await self.read_single_attribute_check_success(
            cluster=self.cluster,
            attribute=attribute
        )

        matter_asserts.assert_valid_uint32(resp, attribute)

        logger.info(f"Reading attribute: {attribute}, response: {resp}")

    @async_test_body
    async def test_TC_DISHALM_2_1(self):

        self.cluster = Clusters.DishwasherAlarm
        self.endpoint = self.get_endpoint()

        self.step(1)

        self.step(2)
        mask_attribute = Clusters.DishwasherAlarm.Attributes.Mask
        await self.read_and_check_attributes_from_dishwasher_alarm(mask_attribute)

        self.step(3)
        latch_attribute = Clusters.DishwasherAlarm.Attributes.Latch
        await self.read_and_check_attributes_from_dishwasher_alarm(latch_attribute)

        self.step(4)
        state_attribute = Clusters.DishwasherAlarm.Attributes.State
        await self.read_and_check_attributes_from_dishwasher_alarm(state_attribute)

        self.step(5)
        supported_attribute = Clusters.DishwasherAlarm.Attributes.Supported
        await self.read_and_check_attributes_from_dishwasher_alarm(supported_attribute)


if __name__ == "__main__":
    default_matter_test_main()
