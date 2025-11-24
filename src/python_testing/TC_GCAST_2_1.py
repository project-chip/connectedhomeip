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
#     app: ${LIGHTING_APP_NO_UNIQUE_ID}
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

import logging
from mobly import asserts
import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GCAST_2_1(MatterBaseTest):
    def desc_TC_GCAST_2_1(self):
        return "[TC-GCAST-2.1] Attributes with DUT as Server - Provisional"

    def steps_TC_G_2_2(self):
        return [TestStep(1, "Comissioning, already done", is_commissioning=True),
                TestStep(2, "TH reads from the DUT the Membership attribute"),
                TestStep(3, "TH reads from the DUT the MaxMembershipCount attribute")]

    def pics_TC_GCAST_2_1(self) -> list[str]:
        pics = ["GCAST.S"]
        return pics

    @async_test_body
    async def test_TC_GCAST_2_1(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 1
        cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership
        max_membership_count_attribute = Clusters.Groupcast.Attributes.MaxMembershipCount

        self.step(1)

        self.step(2)
        membership = await self.read_single_attribute_check_success(cluster, membership_attribute)
        asserts.assert_is_instance(membership, list, "Membership attribute should be a list")

        self.step(3)
        M_max = await self.read_single_attribute_check_success(cluster, max_membership_count_attribute)
        asserts.assert_true(M_max >= 10)

if __name__ == "__main__":
    default_matter_test_main()
