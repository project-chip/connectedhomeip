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


class TC_GCAST_2_5(MatterBaseTest):
    def desc_TC_GCAST_2_5(self):
        return "[TC-GCAST-2.5] ConfigureAuxiliaryACL (Listener feature) with DUT as Server - PROVISIONAL"

    def steps_TC_GCAST_2_5(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeyID on the DUT."),
            TestStep("1c", "TH subscribes to Membership attribute with min interval 0s and max interval 30s."),
            TestStep(
                "1d", "Join group G1 generating a new KeyID K1 with Key InputKey1: TH sends command JoinGroup (GroupID=G1, Endpoints=[EP1], KeyID=K1, Key=InputKey1)."),
            TestStep(2, "Enable Auxiliary ACL on group G1: TH sends command ConfigureAuxiliaryACL (GroupID=G1, UseAuxiliaryACL=true)."),
            TestStep(3, "TH awaits subscription report of new Membership within max interval."),
            TestStep(4, "Disable Auxiliary ACL on group G1: TH sends command ConfigureAuxiliaryACL (GroupID=G1, UseAuxiliaryACL=false)."),
            TestStep(5, "TH awaits subscription report of new Membership within max interval."),
            TestStep(6, "Attempt to enable Auxiliary ACL on a unknown GroupId: TH sends command ConfigureAuxiliaryACL (GroupID=G_UNKNOWN, UseAuxiliaryACL=true)."),
            TestStep(7, "If GCAST.S.F01(SD) feature is supported on the cluster, join group G2 as Sender: TH sends command JoinGroup (GroupID=G2, Endpoints=[],KeyId=K1) to join group as sender only."),
            TestStep(8, "If GCAST.S.F01(SD) feature is supported on the cluster, attempt to enable Auxiliary ACL on group G2: TH sends command ConfigureAuxiliaryACL (GroupID=G2, UseAuxiliaryACL=true) on Sender-only membership"),
        ]

    def pics_TC_GCAST_2_5(self) -> list[str]:
        return ["GCAST.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_5(self):
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        self.step("1a")
        ln_enabled, sd_enabled, pga_enabled = await get_feature_map(self)
        if not ln_enabled:
            logger.info("Listener feature is not enabled, skip remaining steps.")
            self.mark_all_remaining_steps_skipped("1b")
            return

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

        self.step(2)
        await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(
            groupID=groupID1,
            useAuxiliaryACL=True)
        )

        self.step(3)
        membership_matcher = generate_membership_entry_matcher(groupID1, has_auxiliary_acl=True)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(4)
        await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(
            groupID=groupID1,
            useAuxiliaryACL=False)
        )

        self.step(5)
        sub.reset()
        membership_matcher = generate_membership_entry_matcher(groupID1, has_auxiliary_acl=False)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

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
            self.mark_all_remaining_steps_skipped("7")
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
