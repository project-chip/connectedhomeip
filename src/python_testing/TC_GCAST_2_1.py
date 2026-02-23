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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 0
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GCAST_2_1(MatterBaseTest):
    def desc_TC_GCAST_2_1(self):
        return "[TC-GCAST-2.1] Attributes with DUT as Server - Provisional"

    def steps_TC_GCAST_2_1(self):
        return [TestStep(1, "Commissioning, already done", is_commissioning=True),
                TestStep(2, "TH reads from the DUT the Membership attribute"),
                TestStep(3, "TH reads from the DUT the MaxMembershipCount attribute"),
                TestStep(4, "TH reads from the DUT the MaxMcastAddrCount attribute"),
                TestStep(5, "TH reads from the DUT the UsedMcastAddrCount attribute"),
                TestStep(6, "TH reads from the DUT the FabricUnderTest attribute")]

    def pics_TC_GCAST_2_1(self) -> list[str]:
        return ["GCAST.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_1(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 0
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership
        max_membership_count_attribute = Clusters.Groupcast.Attributes.MaxMembershipCount
        max_mcast_addr_count_attribute = Clusters.Groupcast.Attributes.MaxMcastAddrCount
        used_mcast_addr_count_attribute = Clusters.Groupcast.Attributes.UsedMcastAddrCount
        fabric_under_test_attribute = Clusters.Groupcast.Attributes.FabricUnderTest

        self.step(1)

        self.step(2)
        membership = await self.read_single_attribute_check_success(groupcast_cluster, membership_attribute)
        asserts.assert_is_instance(membership, list, "Membership attribute list should be a list.")
        asserts.assert_equal(membership, [], "Membership attribute list should be empty.")

        self.step(3)
        M_max = await self.read_single_attribute_check_success(groupcast_cluster, max_membership_count_attribute)
        asserts.assert_true(M_max >= 10, "MaxMembershipCount attribute should be >= 10")

        self.step(4)
        A_max = await self.read_single_attribute_check_success(groupcast_cluster, max_mcast_addr_count_attribute)
        asserts.assert_true(A_max >= 1, "MaxMcastAddrCount attribute should be >= 1")

        self.step(5)
        usedMcastAddrCount = await self.read_single_attribute_check_success(groupcast_cluster, used_mcast_addr_count_attribute)
        asserts.assert_true(usedMcastAddrCount <= A_max,
                            f"UsedMcastAddrCount ({usedMcastAddrCount}) should be <= MaxMcastAddrCount ({A_max})")

        self.step(6)
        fabricUnderTest = await self.read_single_attribute_check_success(groupcast_cluster, fabric_under_test_attribute)
        asserts.assert_equal(fabricUnderTest, 0, "FabricUnderTest attribute should be 0 (testing disabled by default)")


if __name__ == "__main__":
    default_matter_test_main()
