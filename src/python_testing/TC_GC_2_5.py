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
#     app: ${ALL_DEVICES_APP}
#     app-args: --device on-off-light:1 --discriminator 1234 --KVS kvs1 --groupcast
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
from TC_GC_common import generate_membership_entry_matcher, get_auxiliary_acl_equivalence_set, get_feature_map, valid_endpoints_list

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GC_2_5(MatterBaseTest):
    def desc_TC_GC_2_5(self):
        return "[TC-GC-2.5] ConfigureAuxiliaryACL (Listener feature) with DUT as Server - PROVISIONAL"

    def steps_TC_GC_2_5(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeySetID on the DUT."),
            TestStep("1c", "TH subscribes to Membership attribute of the Groupcast cluster on Endpoint 0 and to the AuxiliaryAccessUpdated event of the AccessControl cluster with a min interval of 0s and max interval of 30s. Accumulate all reports seen."),
            TestStep("1d", "TH reads OperationalCredentials cluster's CurrentFabricIndex attribute on Endpoint 0"),
            TestStep(
                "1e", "Join group G1 generating a new KeySetID K1 with Key InputKey1: TH sends command JoinGroup (GroupID=G1, Endpoints=[EP1], KeySetID=K1, Key=InputKey1)."),
            TestStep(2, "Enable Auxiliary ACL on group G1: TH sends command ConfigureAuxiliaryACL (GroupID=G1, UseAuxiliaryACL=true)."),
            TestStep("3a", "TH awaits subscription report of new Membership within max interval. (G1 entry with HasAuxiliaryACL=true)"),
            TestStep("3b", "TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster"),
            TestStep("3c", "TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute and verifies G1 entry is present"),
            TestStep(4, "Disable Auxiliary ACL on group G1: TH sends command ConfigureAuxiliaryACL (GroupID=G1, UseAuxiliaryACL=false)."),
            TestStep("5a", "TH awaits subscription report of new Membership within max interval. (G1 entry with HasAuxiliaryACL=false)"),
            TestStep("5b", "TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster"),
            TestStep("5c", "TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute and verifies G1 entry is removed"),
            TestStep(6, "Attempt to enable Auxiliary ACL on a unknown GroupId: TH sends command ConfigureAuxiliaryACL (GroupID=G_UNKNOWN, UseAuxiliaryACL=true)."),
            TestStep(7, "If GC.S.F01(SD) feature is supported on the cluster, join group G2 as Sender: TH sends command JoinGroup (GroupID=G2, Endpoints=[],KeyId=K1) to join group as sender only."),
            TestStep(8, "If GC.S.F01(SD) feature is supported on the cluster, attempt to enable Auxiliary ACL on group G2: TH sends command ConfigureAuxiliaryACL (GroupID=G2, UseAuxiliaryACL=true) on Sender-only membership"),
        ]

    def pics_TC_GC_2_5(self) -> list[str]:
        return ["GC.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GC_2_5(self):
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        self.step("1a")
        ln_enabled, sd_enabled, pga_enabled = await get_feature_map(self)
        if not ln_enabled:
            logger.info("Listener feature is not enabled, skip remaining steps.")
            self.mark_all_remaining_steps_skipped("1b")
            return

        endpoints_list = await valid_endpoints_list(self, ln_enabled)
        if len(endpoints_list) > 1:
            endpoints_list = [endpoints_list[0]]

        # Get the Root Node PartsList to use for wildcard expansion in ACL checks.
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

        self.step(2)
        await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(
            groupID=groupID1,
            useAuxiliaryACL=True)
        )

        self.step("3a")
        membership_matcher = generate_membership_entry_matcher(groupID1, has_auxiliary_acl=True)
        membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step("3b")
        event_data = event_sub.wait_for_event_report(Clusters.AccessControl.Events.AuxiliaryAccessUpdated, timeout_sec=60)
        asserts.assert_equal(event_data.adminNodeID, self.default_controller.nodeId, "Event adminNodeID should match TH Node ID")

        self.step("3c")
        aux_acl = await self.read_single_attribute_check_success(Clusters.AccessControl, Clusters.AccessControl.Attributes.AuxiliaryACL, endpoint=0)
        actual_set = get_auxiliary_acl_equivalence_set(aux_acl, parts_list)
        asserts.assert_true((fabric_index, groupID1, endpoints_list[0]) in actual_set,
                            f"Could not find valid AuxiliaryACL entry for FabricIndex {fabric_index}, Group G1 ({groupID1}) and Endpoint {endpoints_list[0]}")

        self.step(4)
        await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(
            groupID=groupID1,
            useAuxiliaryACL=False)
        )

        self.step("5a")
        membership_sub.reset()
        membership_matcher = generate_membership_entry_matcher(groupID1, has_auxiliary_acl=False)
        membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step("5b")
        event_data = event_sub.wait_for_event_report(Clusters.AccessControl.Events.AuxiliaryAccessUpdated, timeout_sec=60)
        asserts.assert_equal(event_data.adminNodeID, self.default_controller.nodeId, "Event adminNodeID should match TH Node ID")

        self.step("5c")
        aux_acl = await self.read_single_attribute_check_success(Clusters.AccessControl, Clusters.AccessControl.Attributes.AuxiliaryACL, endpoint=0)
        actual_set = get_auxiliary_acl_equivalence_set(aux_acl, parts_list)
        asserts.assert_false((fabric_index, groupID1, endpoints_list[0]) in actual_set,
                             f"Entry for FabricIndex {fabric_index}, Group G1 ({groupID1}) and Endpoint {endpoints_list[0]} should have been removed from AuxiliaryACL")

        self.step(6)
        try:
            groupIDUnknown = 2
            await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(
                groupID=groupIDUnknown,
                useAuxiliaryACL=True)
            )
            asserts.fail("ConfigureAuxiliaryACL command should have failed with an unknown GroupID, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Send ConfigureAuxiliaryACL command error should be {Status.NotFound} instead of {e.status}")

        if not sd_enabled:
            self.mark_all_remaining_steps_skipped(7)
            return

        self.step(7)
        groupID2 = 2
        endpoints = []
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=endpoints,
            keySetID=keySetID1)
        )

        self.step(8)
        await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(
            groupID=groupID2,
            useAuxiliaryACL=True)
        )


if __name__ == "__main__":
    default_matter_test_main()
