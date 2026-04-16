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
import secrets

from mobly import asserts
from TC_GC_common import (generate_membership_empty_matcher, generate_membership_entry_matcher, get_auxiliary_acl_equivalence_set,
                          get_feature_map, valid_endpoints_list)

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GC_2_4(MatterBaseTest):
    def desc_TC_GC_2_4(self):
        return "[TC-GC-2.4] LeaveGroup partial & full removal with DUT as Server - PROVISIONAL"

    def steps_TC_GC_2_4(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeySetID on the DUT"),
            TestStep("1c", "TH subscribes to Membership attribute and to the AuxiliaryAccessUpdated event of the AccessControl cluster with a min interval of 0s and max interval of 30s. Accumulate all reports seen."),
            TestStep("1d", "TH reads OperationalCredentials cluster's CurrentFabricIndex attribute on Endpoint 0"),
            TestStep("1e", "Join Group G1 generating new KeySetID K1 with Key InputKey1 using JoinGroup"),
            TestStep("1f", "TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster (expect no report)"),
            TestStep("1g", "TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute and verifies G1 entry is not present"),
            TestStep("1h", "Join Group G2 with existing KeySetID K1 and UseAuxiliaryACL=True"),
            TestStep("1i", "TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster"),
            TestStep("1j", "TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute and verifies G2 entry is present"),
            TestStep("2a", "Completely Leave Group G2 by omitting the endpoint list parameters. LeaveGroup (GroupID=G2, Endpoints omitted)"),
            TestStep("2b", "TH awaits subscription report of new Membership within max interval. (G2 entry removed)"),
            TestStep("2c", "TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster"),
            TestStep("2d", "TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute and verifies G2 entry is removed"),
            TestStep(3, "If LN feature is not supported skip to step 5"),
            TestStep("4a", "Join Group G3 with existing KeySetID K1 using JoinGroup"),
            TestStep("4b", "If DUT only support one non-root and non-aggregator endpoint, skip to step 4e"),
            TestStep("4c", "Remove EP2 from Group G3. LeaveGroup (GroupID=G3, Endpoints=[EP2])"),
            TestStep(
                "4d", "TH awaits subscription report of new Membership within max interval. (G3 entry with endpoints list [EP1])"),
            TestStep("4e", "Remove EP1 from Group G3. LeaveGroup (GroupID=G3, Endpoints=[EP1])"),
            TestStep("4f", "TH awaits subscription report of new Membership within max interval. (If SD supported: G3 entry with empty endpoints list, else G3 entry removed)"),
            TestStep(5, "Attempt to Leave a non-existing group. LeaveGroup (GroupID=NonExisting)"),
            TestStep(6, "Leave all groups. LeaveGroup with GroupID=0"),
            TestStep(7, "TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster (expect no report)"),
            TestStep(8, "TH awaits subscription report of new Membership within max interval. (Empty list)"),
            TestStep(9, "Leave all groups without being part of any group on this fabric. LeaveGroup with GroupID=0"),
        ]

    def pics_TC_GC_2_4(self) -> list[str]:
        return ["GC.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GC_2_4(self):
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        self.step("1a")
        ln_enabled, sd_enabled, pga_enabled = await get_feature_map(self)
        endpoints_list = await valid_endpoints_list(self, ln_enabled)
        if len(endpoints_list) > 1:
            endpoints_list = endpoints_list[:2]

        # Get the Root Node PartsList to use for potential wildcard expansion in ACL checks.
        parts_list = await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            endpoint=0
        )

        self.step("1b")
        # Check if there are any groups on the DUT.
        membership = await self.read_single_attribute_check_success(groupcast_cluster, membership_attribute)
        if membership:
            # LeaveGroup with groupID 0 will leave all groups on the fabric.
            await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

        # remove any existing KeySetID on the DUT, except KeySetId 0 (IPK).
        resp: Clusters.GroupKeyManagement.Commands.KeySetReadAllIndicesResponse = await self.send_single_cmd(Clusters.GroupKeyManagement.Commands.KeySetReadAllIndices())

        read_group_key_ids: list[int] = resp.groupKeySetIDs
        for key_set_id in read_group_key_ids:
            if key_set_id != 0:
                await self.send_single_cmd(Clusters.GroupKeyManagement.Commands.KeySetRemove(key_set_id))

        self.step("1c")
        membership_sub = AttributeSubscriptionHandler(groupcast_cluster, membership_attribute)
        await membership_sub.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        event_sub = EventSubscriptionHandler(expected_cluster=Clusters.AccessControl,
                                             expected_event_id=Clusters.AccessControl.Events.AuxiliaryAccessUpdated.event_id)
        await event_sub.start(self.default_controller, self.dut_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=30)

        self.step("1d")
        fabric_index = await self.read_single_attribute_check_success(Clusters.OperationalCredentials, Clusters.OperationalCredentials.Attributes.CurrentFabricIndex, endpoint=0)

        self.step("1e")
        groupID1 = 1
        keySetID1 = 1
        inputKey1 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=endpoints_list,
            keySetID=keySetID1,
            key=inputKey1)
        )

        self.step("1f")
        event_sub.wait_for_event_expect_no_report(timeout_sec=5)

        self.step("1g")
        aux_acl = await self.read_single_attribute_check_success(Clusters.AccessControl, Clusters.AccessControl.Attributes.AuxiliaryACL, endpoint=0)
        actual_set = get_auxiliary_acl_equivalence_set(aux_acl, parts_list)
        for endpoint in endpoints_list:
            asserts.assert_false((fabric_index, groupID1, endpoint) in actual_set,
                                 f"Entry for FabricIndex {fabric_index}, Group G1 ({groupID1}) and Endpoint {endpoint} should not be in AuxiliaryACL")

        self.step("1h")
        groupID2 = 2
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=endpoints_list,
            keySetID=keySetID1,
            useAuxiliaryACL=True)
        )

        self.step("1i")
        event_data = event_sub.wait_for_event_report(Clusters.AccessControl.Events.AuxiliaryAccessUpdated, timeout_sec=60)
        asserts.assert_equal(event_data.adminNodeID, self.default_controller.nodeId, "Event adminNodeID should match TH Node ID")

        self.step("1j")
        aux_acl = await self.read_single_attribute_check_success(Clusters.AccessControl, Clusters.AccessControl.Attributes.AuxiliaryACL, endpoint=0)
        actual_set = get_auxiliary_acl_equivalence_set(aux_acl, parts_list)
        for endpoint in endpoints_list:
            asserts.assert_true((fabric_index, groupID2, endpoint) in actual_set,
                                f"Could not find valid AuxiliaryACL entry for FabricIndex {fabric_index}, Group G2 ({groupID2}) and Endpoint {endpoint}")

        self.step("2a")
        resp: Clusters.Groupcast.Commands.LeaveGroupResponse = await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=groupID2))
        asserts.assert_is_not_none(resp.endpoints, "LeaveGroupResponse endpoints should not be None")
        asserts.assert_equal(resp.endpoints, endpoints_list,
                             f"LeaveGroupResponse cmd endpoints list {resp.endpoints} is not equal to the endpoints list provided in step 1h {endpoints_list}")

        self.step("2b")
        membership_matcher = generate_membership_entry_matcher(groupID2, test_for_exists=False)
        membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step("2c")
        event_data = event_sub.wait_for_event_report(Clusters.AccessControl.Events.AuxiliaryAccessUpdated, timeout_sec=60)
        asserts.assert_equal(event_data.adminNodeID, self.default_controller.nodeId, "Event adminNodeID should match TH Node ID")

        self.step("2d")
        aux_acl = await self.read_single_attribute_check_success(Clusters.AccessControl, Clusters.AccessControl.Attributes.AuxiliaryACL, endpoint=0)
        actual_set = get_auxiliary_acl_equivalence_set(aux_acl, parts_list)
        for endpoint in endpoints_list:
            asserts.assert_false((fabric_index, groupID2, endpoint) in actual_set,
                                 f"Entry for FabricIndex {fabric_index}, Group G2 ({groupID2}) and Endpoint {endpoint} should have been removed from AuxiliaryACL")

        self.step(3)
        if not ln_enabled:
            self.mark_step_range_skipped("4a", "4f")
        else:
            self.step("4a")
            groupID3 = 3
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID3,
                endpoints=endpoints_list,
                keySetID=keySetID1)
            )

            self.step("4b")
            if len(endpoints_list) == 1:
                self.mark_step_range_skipped("4c", "4d")
            else:
                self.step("4c")
                endpoint_2 = [endpoints_list[1]]
                resp: Clusters.Groupcast.Commands.LeaveGroupResponse = await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(
                    groupID=groupID3,
                    endpoints=endpoint_2)
                )
                asserts.assert_is_not_none(resp.endpoints, "LeaveGroupResponse endpoints should not be None")
                asserts.assert_equal(resp.endpoints, endpoint_2,
                                     f"LeaveGroupResponse cmd endpoints list {resp.endpoints} is not equal to {endpoint_2}")

                self.step("4d")
                membership_sub.reset()
                membership_matcher = generate_membership_entry_matcher(groupID3, endpoints=[endpoints_list[0]])
                membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

            self.step("4e")
            endpoint_1 = [endpoints_list[0]]
            resp: Clusters.Groupcast.Commands.LeaveGroupResponse = await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(
                groupID=groupID3,
                endpoints=endpoint_1)
            )
            asserts.assert_is_not_none(resp.endpoints, "LeaveGroupResponse endpoints should not be None")
            asserts.assert_equal(resp.endpoints, endpoint_1,
                                 f"LeaveGroupResponse cmd endpoints list {resp.endpoints} is not equal to {endpoint_1}")

            self.step("4f")
            if sd_enabled:
                membership_matcher = generate_membership_entry_matcher(groupID3, endpoints=None)
                membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)
            else:
                membership_matcher = generate_membership_entry_matcher(groupID3, test_for_exists=False)
                membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(5)
        groupIDUnknown = 100
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=groupIDUnknown))
            asserts.fail("LeaveGroup command should have failed the groupID does not exist, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Send LeaveGroup command error should be {Status.NotFound} instead of {e.status}")

        self.step(6)
        resp: Clusters.Groupcast.Commands.LeaveGroupResponse = await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=0))
        asserts.assert_is_not_none(resp.endpoints, "LeaveGroupResponse endpoints should not be None")
        asserts.assert_equal(resp.endpoints, [],
                             f"LeaveGroupResponse cmd endpoints list {resp.endpoints} is not equal to an empty list")

        self.step(7)
        membership_sub.reset()
        event_sub.wait_for_event_expect_no_report(timeout_sec=5)

        self.step(8)
        membership_matcher = generate_membership_empty_matcher()
        membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(9)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=0))
            asserts.fail("LeaveGroup command should have failed, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Send LeaveGroup command error should be {Status.NotFound} instead of {e.status}")


if __name__ == "__main__":
    default_matter_test_main()
