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
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import secrets

from mobly import asserts
from TC_GCAST_common import generate_membership_entry_matcher, get_feature_map, valid_endpoints_list

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from matter.tlv import uint

logger = logging.getLogger(__name__)


class TC_GCAST_2_2(MatterBaseTest):
    def desc_TC_GCAST_2_2(self):
        return "[TC-GCAST-2.2] JoinGroup as Listener or Sender with DUT as Server - Provisional"

    def steps_TC_GCAST_2_2(self):
        return [TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
                TestStep("1b", "TH removes any existing group and KeySetID on the DUT"),
                TestStep("1c", "Th subscribes to Membership attribute with min interval 0s and max interval 30s"),
                TestStep(2, "If GCAST.S.F00(LN) feature is not supported on the cluster, skip to step 12"),
                TestStep("3a", "Attempt to join Group G1 with a new Key with KeySetID K1 on Endpoint EP1"),
                TestStep("3b", "TH awaits subscription report of new membership within max interval"),
                TestStep("4a", "If DUT only support one non-root and non-aggregator endpoint, skip to step 5a"),
                TestStep("4b", "Attempt to add EP2 to Group G1"),
                TestStep("4c", "TH awaits subscription report of new membership within max interval"),
                TestStep("5a", "Attempt to join Group G2 with existing Key1 and using Auxiliary ACL"),
                TestStep("5b", "TH awaits subscription report of new membership within max interval"),
                TestStep("6a", "Attempt to join Group G2 with new Key"),
                TestStep("6b", "TH awaits subscription report of new membership within max interval"),
                TestStep(7, "Attempt to join Group G3 using a new Key but providing existing KeySetID (result: already exists)"),
                TestStep(8, "Attempt to join Group G3 using a new KeySetID, but without providing InputKey (result: not found)"),
                TestStep(9, "Attempt to join Group G3 with invalid endpoint (result: unsupported endpoint)"),
                TestStep(10, "If Sender is supported in DUT, skip this step. Else, attempt to join Group G3 with an empty endpoints list (result: constraint error)"),
                TestStep(11, "If DUT has more than 20 endpoints, attempt to join Group G3 with 21 endpoints (result: constraint error)"),
                TestStep(12, "If Sender is not supported in DUT, skip to step 18"),
                TestStep("13a", "Attempt to join Group G4 as Sender (no endpoints) and a new key"),
                TestStep("13b", "TH awaits subscription report of new membership within max interval"),
                TestStep("14a", "Attempt to join Group G5 as Sender using existing KeySetID"),
                TestStep("14b", "TH awaits subscription report of new membership within max interval"),
                TestStep("15a", "Attempt to join Group G5 with new Key but providing existing KeySetID (result: already exists)"),
                TestStep("15b", "Attempt to join Group G5 with new Key but without providing InputKey (result: not found)"),
                TestStep("16a", "Attempt to join Group G5 with new Key"),
                TestStep("16b", "TH awaits subscription report of new membership within max interval"),
                TestStep(17, "Confirm If Listener featIsSupported, skip this step. Else attempt to add endpoints to Group G5 (result: constraint error)"),
                TestStep(18, "If Listener feature is not supported, attempt to join a group with AuxiliaryACL (result: constraint error)"),
                TestStep(19, "JoinGroup with invalid GroupID (result: constraint error)"),
                TestStep(20, "JoinGroup with Key length != 16 (result: constraint error)"),
                TestStep("21a", "Test JoinGroup with ReplaceEndpoints=True: Add EP1 to Group G6"),
                TestStep("21b", "TH awaits subscription report for Group G6 with EP1"),
                TestStep("21c", "Test JoinGroup with ReplaceEndpoints=True: Replace EP1 with EP2 in Group G6"),
                TestStep("21d", "TH awaits subscription report for Group G6 with only EP2"),
                TestStep(22, "JoinGroup with Endpoint 0 (result: constraint error)")]

    def pics_TC_GCAST_2_2(self) -> list[str]:
        return ["GCAST.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_2(self):
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        # Commission DUT to TH (can be skipped if done in a preceding test)
        self.step("1a")
        ln_enabled, sd_enabled, pga_enabled = await get_feature_map(self)
        endpoints_list = await valid_endpoints_list(self, ln_enabled)
        if not endpoints_list:
            self.mark_step_range_skipped("1b", 8)

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

        # Th subscribes to Membership attribute with min interval 0s and max interval 30s
        self.step("1c")
        membership_sub = AttributeSubscriptionHandler(groupcast_cluster, membership_attribute)
        await membership_sub.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        # If GCAST.S.F00(LN) feature is not supported on the cluster, skip to step 12
        self.step(2)
        if endpoints_list[0] is None:
            self.mark_step_range_skipped("3a", 8)
        if not ln_enabled:
            self.mark_step_range_skipped("3a", 11)

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

        # If DUT only support one non-root and non-aggregator endpoint, skip to step 5a
        self.step("4a")
        if len(endpoints_list) < 2:
            self.mark_step_range_skipped("4b", "4c")

        # Attempt to add EP2 to Group G1
        self.step("4b")
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=endpoints_list[0:2],
            keySetID=keySetID1,
            useAuxiliaryACL=False)
        )

        # TH awaits subscription report of new membership within max interval
        self.step("4c")
        membership_matcher = generate_membership_entry_matcher(
            group_id=groupID1, endpoints=endpoints_list[0:2], key_set_id=keySetID1, has_auxiliary_acl=False)
        membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        # Attempt to join Group G2 with existing Key1 and using Auxiliary ACL
        self.step("5a")
        groupID2 = 2

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=[endpoint1],
            keySetID=keySetID1,
            useAuxiliaryACL=True)
        )

        # TH awaits subscription report of new membership within max interval
        self.step("5b")
        membership_matcher = generate_membership_entry_matcher(
            group_id=groupID2, endpoints=[endpoint1], key_set_id=keySetID1, has_auxiliary_acl=True)
        membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        # Attempt to join Group G2 with new Key
        self.step("6a")
        keySetID2 = 2
        inputKey2 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=[endpoint1],
            keySetID=keySetID2,
            key=inputKey2,
            useAuxiliaryACL=True)
        )

        # TH awaits subscription report of new membership within max interval
        self.step("6b")
        membership_matcher = generate_membership_entry_matcher(
            group_id=groupID2, key_set_id=keySetID2, endpoints=[endpoint1], has_auxiliary_acl=True)
        membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        # Attempt to join Group G3 using a new Key but providing existing KeySetID (result: already exists)
        self.step(7)
        groupID3 = 3
        inputKey3 = secrets.token_bytes(16)

        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID3,
                endpoints=[endpoint1],
                keySetID=keySetID2,
                key=inputKey3)
            )
            asserts.fail(
                "JoinGroup command should have failed because Group with keySetID already exists and does not match key, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.AlreadyExists,
                                 f"Send JoinGroup command error should be {Status.AlreadyExists} instead of {e.status}")

        # Attempt to join Group G3 using a new KeySetID, but without providing InputKey (result: not found)
        self.step(8)
        keySetID3 = 3
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID3,
                endpoints=[uint(endpoint1)],
                keySetID=keySetID3)
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

        # If Sender is supported in DUT, skip this step. Else, attempt to join Group G3 with an empty endpoints list (result: constraint error)
        self.step(10)
        endpoints_list_empty = []
        if sd_enabled:
            self.skip_step(10)
        else:
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
        self.step(11)
        if sd_enabled:
            self.skip_step(11)
        else:
            if len(endpoints_list) > 20:
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
                        "JoinGroup command should have failed because endpoints list has more endpoints than DUT provides, but it still succeeded")
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.UnsupportedEndpoint,
                                         f"Send JoinGroup command error should be {Status.UnsupportedEndpoint} instead of {e.status}")

        # If Sender is not supported in DUT, skip to step 18
        self.step(12)
        if not sd_enabled:
            self.mark_step_range_skipped(13, 17)

        # Attempt to join Group G4 as Sender (no endpoints) and a new key
        self.step("13a")
        groupID4 = 4
        keySetID4 = 4
        inputKey4 = secrets.token_bytes(16)
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID4,
            endpoints=endpoints_list_empty,
            keySetID=keySetID4,
            key=inputKey4)
        )

        # TH awaits subscription report of new membership within max interval
        self.step("13b")
        membership_matcher = generate_membership_entry_matcher(
            group_id=groupID4, key_set_id=keySetID4, endpoints=endpoints_list_empty)
        membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        # Attempt to join Group G5 as Sender using existing KeySetID
        self.step("14a")
        groupID5 = 5
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID5,
            endpoints=endpoints_list_empty,
            keySetID=keySetID4)
        )

        # TH awaits subscription report of new membership within max interval
        self.step("14b")
        membership_matcher = generate_membership_entry_matcher(
            group_id=groupID5, key_set_id=keySetID4, endpoints=endpoints_list_empty)
        membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        # Attempt to join Group G5 with new Key but providing existing KeySetID (result: already exists)
        self.step("15a")
        inputKey6 = secrets.token_bytes(16)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID5,
                endpoints=endpoints_list_empty,
                keySetID=keySetID4,
                key=inputKey6)
            )
            asserts.fail(
                "JoinGroup command should have failed because Group with keySetID already exists and does not match key, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.AlreadyExists,
                                 f"Send JoinGroup command error should be {Status.AlreadyExists} instead of {e.status}")

        # Attempt to join Group G5 with new Key but without providing InputKey (result: not found)
        self.step("15b")
        keySetID5 = 5
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID5,
                endpoints=endpoints_list_empty,
                keySetID=keySetID5)
            )
            asserts.fail("JoinGroup command should have failed because no Key found, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Send JoinGroup command error should be {Status.NotFound} instead of {e.status}")

        # Attempt to join Group G5 with new Key
        self.step("16a")
        inputKey5 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID5,
            endpoints=endpoints_list_empty,
            keySetID=keySetID5,
            key=inputKey5)
        )

        # TH awaits subscription report of new membership within max interval
        self.step("16b")
        membership_matcher = generate_membership_entry_matcher(
            group_id=groupID5, key_set_id=keySetID5, endpoints=endpoints_list_empty)
        membership_sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        # Confirm If Listener featIsSupported, skip this step. Else attempt to add endpoints to Group G5 (result: constraint error)
        self.step(17)
        if ln_enabled:
            self.skip_step(17)  # go to step 18

        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID5,
                endpoints=[endpoint1],
                keySetID=keySetID5,
                key=inputKey5)
            )
            asserts.fail("JoinGroup command should have failed because listener cannot do JoinGroup commands, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        # If Listener feature is not supported, attempt to join a group with AuxiliaryACL (result: constraint error)
        self.step(18)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID5,
                endpoints=endpoints_list_empty,
                keySetID=keySetID5,
                useAuxiliaryACL=True)
            )
            asserts.fail("JoinGroup command should have failed because listener cannot do JoinGroup commands, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        # JoinGroup with invalid GroupID (result: constraint error)
        self.step(19)
        groupID0 = 0
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID0, keySetID=1, key=secrets.token_bytes(16))
            )
            asserts.fail("JoinGroup command should have failed because GroupID cannot be 0, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        # JoinGroup with Key length != 16 (result: constraint error)
        self.step(20)
        inputKeyLong = secrets.token_bytes(17)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID5,
                key=inputKeyLong)
            )
            asserts.fail("JoinGroup command should have failed because Key length is not 16 bytes, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        # Test JoinGroup with ReplaceEndpoints=True: Add EP1 to Group G6
        self.step("21a")
        if not ln_enabled:
            self.mark_step_range_skipped("21a", "21d")

        groupID6 = 6
        keySetID6 = 6
        key6 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID6,
            endpoints=[endpoint1],
            keySetID=keySetID6,
            key=key6)
        )

        # TH awaits subscription report for Group G6 with EP1
        self.step("21b")
        membership_sub.await_all_expected_report_matches([generate_membership_entry_matcher(
            group_id=groupID6, key_set_id=keySetID6, endpoints=[endpoint1])], timeout_sec=60)

        # Test JoinGroup with ReplaceEndpoints=True: Replace EP1 with EP2 in Group G6
        self.step("21c")
        endpoint2 = endpoints_list[1]
        if endpoint2 is None:
            self.mark_step_range_skipped("21c", "21d")
        if not (endpoint2 and endpoint1 != endpoint2):
            self.mark_step_range_skipped("21c", "21d")

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID6,
            endpoints=[endpoint2],
            keySetID=keySetID6,
            replaceEndpoints=True)
        )

        # TH awaits subscription report for Group G6 with only EP2
        self.step("21d")
        membership_sub.await_all_expected_report_matches([generate_membership_entry_matcher(
            group_id=groupID6, key_set_id=keySetID6, endpoints=[endpoint2])], timeout_sec=60)

        # JoinGroup with Endpoint 0 (result: constraint error)
        self.step(22)
        if not ln_enabled:
            self.skip_step(22)

        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=1,
                endpoints=[0],
                keySetID=1)
            )
            asserts.fail("JoinGroup command should have failed for Endpoint 0, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")


if __name__ == "__main__":
    default_matter_test_main()
