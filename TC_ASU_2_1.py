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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_DEVICES_APP}
#     app-args: --device ambient-context-sensor:1 --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/asu_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --app-pipe /tmp/asu_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# NodeId is uint64, EndpointId is uint16
MAX_UINT16 = 0xFFFF
MAX_UINT64 = 0xFFFFFFFFFFFFFFFF


# Script Function Call Example
# rm -rf admin_storage1.json && rm -rf kvs1 &&
# python3 ./scripts/tests/run_python_test.py --app out/linux-x64-all-devices-clang/all-devices-app
# --factory-reset --app-args "--device ambient-context-sensor:1 --KVS kvs1 --discriminator 1234 --app-pipe /tmp/asu_fifo"
# --script src/python_testing/TC_ASU_2_1.py
# --script-args "--storage-path admin_storage1.json --discriminator 1234 --passcode 20202021
#   --commissioning-method on-network --endpoint 1 --app-pipe /tmp/asu_fifo"


class TC_ASU_2_1(MatterBaseTest):
    def desc_TC_ASU_2_1(self) -> str:
        return "[TC-ASU-2.1] Attributes with DUT as a server"

    def pics_TC_ASU_2_1(self):
        return ["ASU.S"]

    def steps_TC_ASU_2_1(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads the UnionName attribute.",
                     "DUT response contains string characters."),
            TestStep("3", "TH reads the UnionHealth attribute.",
                     "DUT response contains a UnionHealthEnum type data."),
            TestStep("4", "TH reads the UnionContributorList attribute.",
                     "DUT response contains UnionContributorStruct data containing ContributorNodeID, ContributorEndpointID, ContributorName, and ContributorStatus.")
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AmbientSensingUnion))
    async def test_TC_ASU_2_1(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.AmbientSensingUnion
        attr = Clusters.AmbientSensingUnion.Attributes

        self.step("1")

        self.step("2")
        unionName_read = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attr.UnionName
        )
        asserts.assert_true(isinstance(unionName_read, str), "UnionName shall be a string.")
        log.info("Rx'd UnionName: %s", unionName_read)
        string_size = len(unionName_read)
        # checkvalue between 1 and 128
        asserts.assert_greater_equal(string_size, 1, "The string size needs to be betwween 1 and 128.")
        asserts.assert_less_equal(string_size, 128, "The string size needs to be betwween 1 and 128.")

        self.step("3")
        unionHealth_read = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attr.UnionHealth
        )
        log.info("Rx'd UnionHealth: %s", unionHealth_read)
        valid_union_health_values = [
            Clusters.AmbientSensingUnion.Enums.UnionHealthEnum.kFullyFunctional,
            Clusters.AmbientSensingUnion.Enums.UnionHealthEnum.kLimitedDegraded,
            Clusters.AmbientSensingUnion.Enums.UnionHealthEnum.kNonFunctional,
        ]
        asserts.assert_in(unionHealth_read, valid_union_health_values,
                          "UnionHealth shall be a valid UnionHealthEnum value.")

        self.step("4")
        unionlist_read = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=cluster,
            attribute=attr.UnionContributorList
        )

        list_size = len(unionlist_read)
        # checkvalue between 1 and 128
        asserts.assert_greater_equal(list_size, 1, "The string size needs to be betwween 1 and 128.")
        asserts.assert_less_equal(list_size, 128, "The string size needs to be betwween 1 and 128.")

        valid_contributor_status_values = [
            Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOnline,
            Clusters.AmbientSensingUnion.Enums.UnionContributorStatusEnum.kUnionContributorOffline,
        ]

        for contributor in unionlist_read:

            asserts.assert_true(hasattr(contributor, 'contributorNodeID'), "ContributorNodeID missing")
            asserts.assert_true(hasattr(contributor, 'contributorEndpointID'), "ContributorEndpointID missing")
            asserts.assert_true(hasattr(contributor, 'contributorName'), "ContributorName missing")
            asserts.assert_true(hasattr(contributor, 'contributorStatus'), "ContributorStatus missing")

            if contributor.contributorNodeID == NullValue:
                # Non-Matter contributor: NodeID and EndpointID are null, Name is mandatory
                asserts.assert_equal(
                    contributor.contributorEndpointID,
                    NullValue,
                    "ContributorEndpointID shall be NullValue when ContributorNodeID is NullValue"
                )
                asserts.assert_false(
                    contributor.contributorName == NullValue,
                    "ContributorName shall not be NullValue when ContributorNodeID is NullValue"
                )
                asserts.assert_true(
                    isinstance(contributor.contributorName, str),
                    "ContributorName shall be a non-empty string when ContributorNodeID is NullValue"
                )
                string_size = len(contributor.contributorName)
                # checkvalue between 1 and 128
                asserts.assert_greater_equal(string_size, 1, "The string size needs to be betwween 1 and 128.")
                asserts.assert_less_equal(string_size, 128, "The string size needs to be betwween 1 and 128.")

            else:
                # Matter contributor: NodeID and EndpointID are valid integers
                asserts.assert_greater_equal(contributor.contributorNodeID, 0,
                                             "ContributorNodeID shall be >= 0")
                asserts.assert_less_equal(contributor.contributorNodeID, MAX_UINT64,
                                          "ContributorNodeID shall be <= MAX_UINT64")
                asserts.assert_greater_equal(contributor.contributorEndpointID, 0,
                                             "ContributorEndpointID shall be >= 0")
                asserts.assert_less_equal(contributor.contributorEndpointID, MAX_UINT16,
                                          "ContributorEndpointID shall be <= MAX_UINT16")
                if contributor.contributorName == NullValue:
                    string_size = len(contributor.contributorName)
                    # checkvalue between 1 and 128
                    asserts.assert_greater_equal(string_size, 1, "The string size needs to be betwween 1 and 128.")
                    asserts.assert_less_equal(string_size, 128, "The string size needs to be betwween 1 and 128.")

            asserts.assert_in(
                contributor.contributorStatus,
                valid_contributor_status_values,
                "ContributorStatus shall be a valid UnionContributorStatusEnum value."
            )


if __name__ == "__main__":
    default_matter_test_main()
