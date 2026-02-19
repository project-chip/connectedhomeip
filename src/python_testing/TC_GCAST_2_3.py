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
from TC_GCAST_common import generate_membership_entry_matcher, get_feature_map, valid_endpoints_list

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GCAST_2_3(MatterBaseTest):
    def desc_TC_GCAST_2_3(self):
        return "[TC-GCAST-2.3] UpdateGroupKey KeySetID assignment and key creation with DUT as Server - PROVISIONAL"

    def steps_TC_GCAST_2_3(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeySetID on the DUT"),
            TestStep("1c", "TH subscribes to Membership attribute with min interval 0s and max interval 30s"),
            TestStep("1d", "Join Group G1 generating a new Key with KeySetID K1 using JoinGroup"),
            TestStep("1e", "Join Group G2 generating a new Key with KeySetID K2 using JoinGroup"),
            TestStep(2, "Update Group G1 to use a new KeySetID K3 and create it by providing a Key: TH sends command UpdateGroupKey (GroupID=G1, KeySetID=K3, Key=InputKey3)"),
            TestStep(3, "TH awaits subscription report showing KeySetID=K3 for G1"),
            TestStep(4, "Update Group G2 generating a new key with an existing KeySetID K1. UpdateGroupKey (GroupID=G2, KeySetID=K1, Key=InputKey4)"),
            TestStep(5, "Update Group G2 to use an existing KeySetID K1 without providing a Key. UpdateGroupKey (GroupID=G2, KeySetID=K1, Key omitted)"),
            TestStep(6, "TH awaits subscription report showing new Membership within max interval. (G2 shows KeySetID=K1)"),
            TestStep(7, "Update Group G1 to use a non-existent KeySetID K4 without providing a Key. UpdateGroupKey (GroupID=G1, KeySetID=K4, Key omitted)"),
            TestStep(8, "Update Group G_UNKNOWN. (UpdateGroupKey GroupID=G_UNKNOWN, KeySetID=K4, Key=InputKey4)"),
            TestStep(9, "Update Group G1 with invalid Key length. (UpdateGroupKey GroupID=G1, KeySetID=K4, Key= Input with length !=16)"),
            TestStep(10, "Update Group G1 to use KeySetID already used by another group. (UpdateGroupKey (GroupID=G1, KeySetID=K1, Key omitted)"),
            TestStep(11, "TH awaits subscription report showing new Membership within max interval. (G1 shows KeySetID=K1)"),
        ]

    def pics_TC_GCAST_2_3(self) -> list[str]:
        return ["GCAST.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_3(self):
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        self.step("1a")
        ln_enabled, sd_enabled, pga_enabled = await get_feature_map(self)
        endpoints_list = await valid_endpoints_list(self, ln_enabled)
        endpoints_list = [endpoints_list[0]]

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
        sub = AttributeSubscriptionHandler(groupcast_cluster, membership_attribute)
        await sub.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        self.step("1d")
        groupID1 = 1
        keySetID1 = 1
        inputKey1 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=endpoints_list,
            keySetID=keySetID1,
            key=inputKey1)
        )

        self.step("1e")
        groupID2 = 2
        keySetID2 = 2
        inputKey2 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=endpoints_list,
            keySetID=keySetID2,
            key=inputKey2)
        )

        self.step(2)
        keySetID3 = 3
        inputKey3 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
            groupID=groupID1,
            keySetID=keySetID3,
            key=inputKey3)
        )

        self.step(3)
        membership_matcher = generate_membership_entry_matcher(groupID1, key_set_id=keySetID3)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(4)
        inputKey4 = secrets.token_bytes(16)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID2,
                keySetID=keySetID1,
                key=inputKey4)
            )
            asserts.fail(f"UpdateGroupKey command should have failed with {Status.AlreadyExists}, but it succeeded.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.AlreadyExists,
                                 f"Send UpdateGroupKey command error should be {Status.AlreadyExists} instead of {e.status}")

        self.step(5)
        await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
            groupID=groupID2,
            keySetID=keySetID1)
        )

        self.step(6)
        sub.reset()
        membership_matcher = generate_membership_entry_matcher(groupID2, key_set_id=keySetID1)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(7)
        keySetID4 = 4
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID1,
                keySetID=keySetID4)
            )
            asserts.fail(f"UpdateGroupKey command should have failed {Status.NotFound}, but it succeeded.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Send UpdateGroupKey command error should be {Status.NotFound} instead of {e.status}")

        self.step(8)
        groupIDUnknown = 100
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupIDUnknown,
                keySetID=keySetID4,
                key=inputKey4)
            )
            asserts.fail(f"UpdateGroupKey command should have failed with {Status.NotFound}, but it succeeded.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Send UpdateGroupKey command error should be {Status.NotFound} instead of {e.status}")

        self.step(9)
        inputKeyInvalidLength = secrets.token_bytes(15)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID1,
                keySetID=keySetID4,
                key=inputKeyInvalidLength)
            )
            asserts.fail(f"UpdateGroupKey command should have failed with {Status.ConstraintError}, but it succeeded.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send UpdateGroupKey command error should be {Status.ConstraintError} instead of {e.status}")

        self.step(10)
        await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
            groupID=groupID1,
            keySetID=keySetID1)
        )

        self.step(11)
        sub.reset()
        membership_matcher = generate_membership_entry_matcher(groupID1, key_set_id=keySetID1)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)


if __name__ == "__main__":
    default_matter_test_main()
