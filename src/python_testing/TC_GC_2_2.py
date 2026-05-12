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
from TC_GC_common import generate_membership_entry_matcher, get_auxiliary_acl_equivalence_set, get_feature_map, valid_endpoints_list

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from matter.tlv import uint

logger = logging.getLogger(__name__)


class TC_GC_2_2(MatterBaseTest):
    def desc_TC_GC_2_2(self):
        return "[TC-GC-2.2] JoinGroup as Listener or Sender with DUT as Server - Provisional"

    def steps_TC_GC_2_2(self):
        return [TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
                TestStep("1b", "TH removes any existing group and KeySetID on the DUT"),
                TestStep("1c", "TH subscribes to Membership attribute of the Groupcast cluster on Endpoint 0 and to the AuxiliaryAccessUpdated event of the AccessControl cluster with a min interval of 0s and max interval of 30s. Accumulate all reports seen."),
                TestStep("1d", "TH reads OperationalCredentials cluster's CurrentFabricIndex attribute on Endpoint 0"),
                TestStep(2, "If GC.S.F00(LN) feature is not supported on the cluster, skip to step 14"),
                TestStep("3a", "Attempt to join Group G1 with a new Key with KeySetID K1 on Endpoint EP1"),
                TestStep("3b", "TH awaits subscription report of new membership within max interval"),
                TestStep("3c", "TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute"),
                TestStep("3d", "TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster"),
                TestStep("4a", "If DUT only support one non-root and non-aggregator endpoint, skip to step 6a"),
                TestStep("4b", "Attempt to add EP2 to Group G1"),
                TestStep("4c", "TH awaits subscription report of new membership within max interval"),
                TestStep("4d", "TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute"),
                TestStep("4e", "TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster"),
                TestStep("5a", "Attempt to replace endpoints with only EP1 for Group G1 using ReplaceEndpoints"),
                TestStep("5b", "TH awaits subscription report of new membership within max interval"),
                TestStep("6a", "Attempt to join Group G2 with existing Key1 and using Auxiliary ACL"),
                TestStep("6b", "TH awaits subscription report of new membership within max interval"),
                TestStep("6c", "TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster"),
                TestStep("6d", "TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute"),
                TestStep(7, "Attempt to join Group G3 using a new Key but providing existing KeySetID (result: already exists)"),
                TestStep(8, "Attempt to join Group G3 using a new KeySetID, but without providing InputKey (result: not found)"),
                TestStep(9, "Attempt to join Group G3 with invalid endpoint (result: unsupported endpoint)"),
                TestStep(10, "Attempt to join Group G3 with Root endpoint 0 (result: unsupported endpoint)"),
                TestStep(11, "If Sender is supported in DUT, skip this step. Else, attempt to join Group G3 with an empty endpoints list (result: constraint error)"),
                TestStep(12, "If DUT has more than 20 endpoints, attempt to join Group G3 with 21 endpoints (result: constraint error)"),
                TestStep(13, "If PerGroupAddress is supported on the cluster skip this step, else attempt to join Group G3 with McastAddrPolicy=PerGroup (result: constraint error)"),
                TestStep(14, "If Sender is not supported in DUT, skip to step 20"),
                TestStep("15a", "Attempt to join Group G4 as Sender (no endpoints) and a new key"),
                TestStep("15b", "If Listener is not supported, skip this step. Else TH awaits subscription report of new membership within max interval with Endpoints list empty and HasAuxiliaryACL=False"),
                TestStep("15c", "If Listener is supported, skip this step. Else, TH awaits subscription report of new membership within max interval with Endpoints list and HasAuxiliaryACL ommited"),
                TestStep("16a", "Attempt to join Group G5 as Sender using existing KeySetID"),
                TestStep("16b", "If Listener is not supported, skip this step. Else, TH awaits subscription report of new membership within max interval with Endpoints list empty and HasAuxiliaryACL=False"),
                TestStep("16c", "If Listener is supported, skip this step. Else, TH awaits subscription report of new membership within max interval with Endpoints list and HasAuxiliaryACL ommited"),
                TestStep("17a", "Attempt to join Group G5 with new Key but providing existing KeySetID (result: already exists)"),
                TestStep("17b", "Attempt to join Group G5 with new KeySetID but without providing InputKey (result: not found)"),
                TestStep(18, "If Listener feature is supported, skip this step. Else, attempt to join a group with ReplaceEndpoints=True (result: constraint error)"),
                TestStep(
                    19, "If Listener feature is supported, skip this step. Else, attempt to join a group with AuxiliaryACL true (result: constraint error)"),
                TestStep(20, "JoinGroup with invalid GroupID (result: constraint error)"),
                TestStep(21, "JoinGroup with Key length != 16 (result: constraint error)")]

    def pics_TC_GC_2_2(self) -> list[str]:
        return ["GC.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GC_2_2(self):
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership
        endpoints_list_empty = []

        # Commission DUT to TH (can be skipped if done in a preceding test)
        self.step("1a")
        ln_enabled, sd_enabled, pga_enabled = await get_feature_map(self)
        endpoints_list = await valid_endpoints_list(self, ln_enabled)
        if ln_enabled and not endpoints_list:
            self.mark_all_remaining_steps_skipped("1b")
            return

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

        # TH subscribes to Membership attribute of the Groupcast cluster on Endpoint 0 and to the AuxiliaryAccessUpdated event of the AccessControl cluster with a min interval of 0s and max interval of 30s. Accumulate all reports seen.
        self.step("1c")
        membership_sub = AttributeSubscriptionHandler(groupcast_cluster, membership_attribute)
        await membership_sub.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        event_sub = EventSubscriptionHandler(expected_cluster=Clusters.AccessControl,
                                             expected_event_id=Clusters.AccessControl.Events.AuxiliaryAccessUpdated.event_id)
        await event_sub.start(self.default_controller, self.dut_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=30)

        # TH reads OperationalCredentials cluster's CurrentFabricIndex attribute on Endpoint 0
        self.step("1d")
        fabric_index = await self.read_single_attribute_check_success(Clusters.OperationalCredentials, Clusters.OperationalCredentials.Attributes.CurrentFabricIndex, endpoint=0)

        # If GC.S.F00(LN) feature is not supported on the cluster, skip to step 12
        self.step(2)
        if not ln_enabled:
            self.mark_step_range_skipped("3a", 13)
        else:
            # Attempt to join Group G1 with a new Key with KeySetID K1 on Endpoint EP1
            self.step("3a")
            groupID1 = 1
            keySetID1 = 1
            inputKey1 = secrets.token_bytes(16)
            endpoint1 = endpoints_list[0]

            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID1,
                endpoints=[endpoint1],
                keySetID=keySetID1,
                key=inputKey1,
                useAuxiliaryACL=False)
            )

            # TH awaits subscription report of new membership within max interval
            self.step("3b")
            membership_matcher = generate_membership_entry_matcher(
                group_id=groupID1, endpoints=[endpoint1], key_set_id=keySetID1, has_auxiliary_acl=False)
            membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

            # TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute
            self.step("3c")
            aux_acl = await self.read_single_attribute_check_success(Clusters.AccessControl, Clusters.AccessControl.Attributes.AuxiliaryACL, endpoint=0)

            # Validate that value does not contain any entry that logically represents or grants access on EP1 to Group G1 for FabricIndex F1.
            actual_set = get_auxiliary_acl_equivalence_set(aux_acl, parts_list)
            asserts.assert_false((fabric_index, groupID1, endpoint1) in actual_set,
                                 f"Entry for FabricIndex {fabric_index}, Group G1 ({groupID1}) and Endpoint EP1 ({endpoint1}) should not be in AuxiliaryACL")

            # TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster
            self.step("3d")
            event_sub.wait_for_event_expect_no_report(timeout_sec=5)

            # If DUT only support one non-root and non-aggregator endpoint, skip to step 6a
            self.step("4a")
            if len(endpoints_list) < 2:
                self.mark_step_range_skipped("4b", "5b")
            else:
                # Attempt to add EP2 to Group G1
                self.step("4b")
                await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID1,
                    endpoints=[endpoints_list[1]],
                    keySetID=keySetID1,
                    useAuxiliaryACL=False)
                )

                # TH awaits subscription report of new membership within max interval
                self.step("4c")
                membership_matcher = generate_membership_entry_matcher(
                    group_id=groupID1, endpoints=endpoints_list[0:2], key_set_id=keySetID1, has_auxiliary_acl=False)
                membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

                # TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute
                self.step("4d")
                aux_acl = await self.read_single_attribute_check_success(Clusters.AccessControl, Clusters.AccessControl.Attributes.AuxiliaryACL, endpoint=0)

                # Validate that value does not contain any entry that logically represents or grants access on EP1 or EP2 to Group G1 for FabricIndex F1.
                actual_set = get_auxiliary_acl_equivalence_set(aux_acl, parts_list)
                for endpoint_to_check in [endpoints_list[0], endpoints_list[1]]:
                    asserts.assert_false((fabric_index, groupID1, endpoint_to_check) in actual_set,
                                         f"Entry for FabricIndex {fabric_index}, Group G1 ({groupID1}) and Endpoint {endpoint_to_check} should not be in AuxiliaryACL")

                # TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster
                self.step("4e")
                event_sub.wait_for_event_expect_no_report(timeout_sec=5)

                # Attempt to replace endpoints for Group G1 using ReplaceEndpoints:
                self.step("5a")
                await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID1,
                    endpoints=[endpoint1],
                    keySetID=keySetID1,
                    replaceEndpoints=True)
                )

                # TH awaits subscription report of new membership within max interval
                self.step("5b")
                membership_matcher = generate_membership_entry_matcher(
                    group_id=groupID1, endpoints=[endpoint1], key_set_id=keySetID1, has_auxiliary_acl=False)
                membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

            # Attempt to join Group G2 with existing Key1 and using Auxiliary ACL
            self.step("6a")
            groupID2 = 2

            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID2,
                endpoints=[endpoint1],
                keySetID=keySetID1,
                useAuxiliaryACL=True)
            )

            # TH awaits subscription report of new membership within max interval
            self.step("6b")
            membership_matcher = generate_membership_entry_matcher(
                group_id=groupID2, endpoints=[endpoint1], key_set_id=keySetID1, has_auxiliary_acl=True)
            membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

            # TH awaits subscription report of AuxiliaryAccessUpdated event from AccessControl cluster
            self.step("6c")
            event_data = event_sub.wait_for_event_report(Clusters.AccessControl.Events.AuxiliaryAccessUpdated, timeout_sec=60)
            asserts.assert_equal(event_data.adminNodeID, self.default_controller.nodeId,
                                 "Event adminNodeID should match TH Node ID")

            # TH reads Endpoint 0 AccessControl cluster AuxiliaryACL attribute
            self.step("6d")
            aux_acl = await self.read_single_attribute_check_success(Clusters.AccessControl, Clusters.AccessControl.Attributes.AuxiliaryACL, endpoint=0)

            # Validate value contains entry that logically represents and grants Operate (3) privilege on EP1 for Group G2 on FabricIndex F1.
            actual_set = get_auxiliary_acl_equivalence_set(aux_acl, parts_list)
            asserts.assert_true((fabric_index, groupID2, endpoint1) in actual_set,
                                f"Could not find valid AuxiliaryACL entry for FabricIndex {fabric_index}, Group G2 ({groupID2}) and Endpoint EP1 ({endpoint1})")

            # Attempt to join Group G3 using a new Key but providing existing KeySetID (result: already exists)
            self.step(7)
            groupID3 = 3
            inputKey2 = secrets.token_bytes(16)

            try:
                await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID3,
                    endpoints=[endpoint1],
                    keySetID=keySetID1,
                    key=inputKey2)
                )
                asserts.fail(
                    "JoinGroup command should have failed because Group with keySetID already exists and does not match key, but it still succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.AlreadyExists,
                                     f"Send JoinGroup command error should be {Status.AlreadyExists} instead of {e.status}")

            # Attempt to join Group G3 using a new KeySetID, but without providing InputKey (result: not found)
            self.step(8)
            keySetID2 = 2
            try:
                await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID3,
                    endpoints=[endpoint1],
                    keySetID=keySetID2)
                )
                asserts.fail("JoinGroup command should have failed because no Key found, but it still succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.NotFound,
                                     f"Send JoinGroup command error should be {Status.NotFound} instead of {e.status}")

            # Attempt to join Group G3 with invalid endpoint (result: unsupported endpoint)
            self.step(9)
            endpoint_invalid = uint(0xFFFF)
            try:
                await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID3,
                    endpoints=[endpoint_invalid],
                    keySetID=keySetID1)
                )
                asserts.fail("JoinGroup command should have failed because endpoint is invalid, but it still succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.UnsupportedEndpoint,
                                     f"Send JoinGroup command error should be {Status.UnsupportedEndpoint} instead of {e.status}")

            # Attempt to join Group G3 with root endpoint (result: unsupported endpoint)
            self.step(10)
            endpoint_root = uint(0)
            try:
                await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID3,
                    endpoints=[endpoint_root],
                    keySetID=keySetID1)
                )
                asserts.fail("JoinGroup command should have failed because endpoint is invalid, but it still succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.UnsupportedEndpoint,
                                     f"Send JoinGroup command error should be {Status.UnsupportedEndpoint} instead of {e.status}")

            # If Sender is supported in DUT, skip this step. Else, attempt to join Group G3 with an empty endpoints list (result: constraint error)
            if sd_enabled:
                self.skip_step(11)
            else:
                self.step(11)
                try:
                    await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                        groupID=groupID3,
                        endpoints=endpoints_list_empty,
                        keySetID=keySetID1)
                    )
                    asserts.fail("JoinGroup command should have failed because endpoints list is empty, but it still succeeded")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.ConstraintError,
                                         f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

            # If DUT has more than 20 endpoints, attempt to join Group G3 with 21 endpoints (result: constraint error)
            if len(endpoints_list) > 20:
                self.step(12)
                exceeding_endpoint = 21
                while exceeding_endpoint in endpoints_list:
                    exceeding_endpoint += 1
                endpoints_list_exceeds_DUT_endpoints = endpoints_list + [exceeding_endpoint]
                try:
                    await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                        groupID=groupID3,
                        endpoints=endpoints_list_exceeds_DUT_endpoints,
                        keySetID=keySetID2)
                    )
                    asserts.fail(
                        "JoinGroup command should have failed because endpoints list has more endpoints than supported by the command, but it still succeeded")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.ConstraintError,
                                         f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")
            else:
                self.skip_step(12)

            # If PerGroupAddress is supported on the cluster skip this step, else attempt to join Group G3 with McastAddrPolicy=PerGroup (result: constraint error)
            if pga_enabled:
                self.skip_step(13)
            else:
                self.step(13)
                try:
                    await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                        groupID=groupID3,
                        endpoints=[endpoint1],
                        keySetID=keySetID1,
                        mcastAddrPolicy=Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kPerGroup)
                    )
                    asserts.fail(
                        "JoinGroup command should have failed because McastAddrPolicy is not supported, but it still succeeded")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.ConstraintError,
                                         f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        # If Sender is not supported in DUT, skip to step 20
        self.step(14)
        if not sd_enabled:
            self.mark_step_range_skipped("15a", 19)
        else:
            self.step("15a")
            # Attempt to join Group G4 as Sender (no endpoints) and a new key
            groupID4 = 4
            keySetID2 = 2
            inputKey2 = secrets.token_bytes(16)
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID4,
                endpoints=endpoints_list_empty,
                keySetID=keySetID2,
                key=inputKey2)
            )

            if ln_enabled:
                # TH awaits subscription report of new membership within max interval
                self.step("15b")
                membership_matcher = generate_membership_entry_matcher(
                    group_id=groupID4, key_set_id=keySetID2, endpoints=endpoints_list_empty, has_auxiliary_acl=False)
                membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)
                self.skip_step("15c")
            else:
                self.skip_step("15b")
                self.step("15c")
                membership_matcher = generate_membership_entry_matcher(
                    group_id=groupID4, key_set_id=keySetID2, endpoints=None)
                membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

            # Attempt to join Group G5 as Sender using existing KeySetID
            self.step("16a")
            groupID5 = 5
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID5,
                endpoints=endpoints_list_empty,
                keySetID=keySetID2)
            )

            # TH awaits subscription report of new membership within max interval
            if ln_enabled:
                self.step("16b")
                membership_matcher = generate_membership_entry_matcher(
                    group_id=groupID5, key_set_id=keySetID2, endpoints=endpoints_list_empty, has_auxiliary_acl=False)
                membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)
                self.skip_step("16c")
            else:
                self.skip_step("16b")
                self.step("16c")
                membership_matcher = generate_membership_entry_matcher(
                    group_id=groupID5, key_set_id=keySetID2, endpoints=None)
                membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

            # Attempt to join Group G5 with new Key but providing existing KeySetID (result: already exists)
            self.step("17a")
            inputKey3 = secrets.token_bytes(16)
            try:
                await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID5,
                    endpoints=endpoints_list_empty,
                    keySetID=keySetID2,
                    key=inputKey3)
                )
                asserts.fail(
                    "JoinGroup command should have failed because Group with keySetID already exists and does not match key, but it still succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.AlreadyExists,
                                     f"Send JoinGroup command error should be {Status.AlreadyExists} instead of {e.status}")

            # Attempt to join Group G5 with new Key but without providing InputKey (result: not found)
            self.step("17b")
            keySetID3 = 3
            try:
                await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID5,
                    endpoints=endpoints_list_empty,
                    keySetID=keySetID3)
                )
                asserts.fail("JoinGroup command should have failed because no Key found, but it still succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.NotFound,
                                     f"Send JoinGroup command error should be {Status.NotFound} instead of {e.status}")

            if ln_enabled:
                self.skip_step(18)
            else:
                self.step(18)
                try:
                    await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                        groupID=groupID5,
                        endpoints=endpoints_list_empty,
                        keySetID=keySetID2,
                        replaceEndpoints=True)
                    )
                    asserts.fail("JoinGroup command should have failed because listener cannot do JoinGroup commands, but it still succeeded")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.ConstraintError,
                                         f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

            # If Listener feature is not supported, attempt to join a group with AuxiliaryACL (result: constraint error)
            if ln_enabled:
                self.skip_step(19)
            else:
                self.step(19)
                try:
                    await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                        groupID=groupID5,
                        endpoints=endpoints_list_empty,
                        keySetID=keySetID2,
                        useAuxiliaryACL=True)
                    )
                    asserts.fail("JoinGroup command should have failed because listener cannot do JoinGroup commands, but it still succeeded")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.ConstraintError,
                                         f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        # conditional endpoints list for step 20 and 21
        valid_endpoints = endpoints_list_empty
        if ln_enabled:
            valid_endpoints = [endpoint1]

        # JoinGroup with invalid GroupID=0 (result: constraint error)
        self.step(20)
        groupID0 = 0
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID0, endpoints=valid_endpoints, keySetID=1)
            )
            asserts.fail("JoinGroup command should have failed because GroupID cannot be 0, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        # JoinGroup with Key length != 16 (result: constraint error)
        self.step(21)
        inputKeyLong = secrets.token_bytes(17)
        keySetID4 = 4
        groupID6 = 6
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID6,
                endpoints=valid_endpoints,
                keySetID=keySetID4,
                key=inputKeyLong)
            )
            asserts.fail("JoinGroup command should have failed because Key length is not 16 bytes, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")


if __name__ == "__main__":
    default_matter_test_main()
