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
import secrets

from mobly import asserts
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, has_cluster, run_if_endpoint_matches
from matter.interaction_model import InteractionModelError, Status

from src.python_testing.TC_GCAST_common import get_feature_map, valid_endpoints_list, generate_membership_entry_matcher

logger = logging.getLogger(__name__)


class TC_GCAST_2_4(MatterBaseTest):
    def desc_TC_GCAST_2_4(self):
        return "[TC-GCAST-2.4] ExpireGracePeriod command effect with DUT as Server - PROVISIONAL"

    def steps_TC_GCAST_2_4(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeyID on the DUT"),
            TestStep("1c", "TH subscribes to Membership attribute with min interval 0s and max interval 30s"),
            TestStep("1d", "Join Group G1 generating a new Key with KeyID K1 using JoinGroup"),
            TestStep("1e", "Use JoinGroup to Update Group G1 Key while providing a grace period to K1."),
            TestStep(2, "TH awaits subscription report of new Membership within max interval."),
            TestStep(3, "Expire K1 on Group G1 immediately: TH sends command ExpireGracePeriod (GroupID=G1)"),
            TestStep(4, "TH awaits subscription report of new Membership within max interval."),
            TestStep(5, "Attempt to expire a KeyId on Group G1 where there is no ExpiringKey: TH sends command ExpireGracePeriod (GroupID=G1)"),
            TestStep(6, "Attempt to expire a KeyId for non-existent GroupID: TH sends command ExpireGracePeriod (GroupID=G_Unknown)")
        ]

    def pics_TC_GCAST_2_4(self) -> list[str]:
        pics = ["GCAST.S"]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_4(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 0
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        self.step("1a")
        ln_enabled, sd_enabled = await get_feature_map(self)
        endpoints_list = await valid_endpoints_list(self, ln_enabled)
        endpoints_list = [endpoints_list[0]]

        self.step("1b")
        await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

        self.step("1c")
        sub = AttributeSubscriptionHandler(groupcast_cluster, membership_attribute)
        await sub.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        self.step("1d")
        groupID1 = 1
        keyID1 = 1
        inputKey1 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=endpoints_list,
            keyID=keyID1,
            key=inputKey1)
        )

        self.step("1e")
        keyID2 = 2
        gracePeriodSeconds = 5
        inputKey2 = secrets.token_bytes(16)
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=endpoints_list,
            keyID=keyID2,
            gracePeriod=gracePeriodSeconds,
            key=inputKey2)
        )

        self.step(2)
        membership_matcher = generate_membership_entry_matcher(groupID1, expiring_key_id=keyID1)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(3)
        await self.send_single_cmd(Clusters.Groupcast.Commands.ExpireGracePeriod(groupID=groupID1))

        self.step(4)
        sub.reset()
        membership_matcher = generate_membership_entry_matcher(groupID1, expiring_key_id_must_not_exist=True)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(5)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.ExpireGracePeriod(groupID=groupID1))
            asserts.fail("ExpireGracePeriod command should have failed when there is no ExpiringKey, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure,
                                 f"Send ExpireGracePeriod command error should be {Status.Failure} instead of {e.status}")

        self.step(6)
        groupIDUnknown = 2
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.ExpireGracePeriod(groupID=groupIDUnknown))
            asserts.fail("ExpireGracePeriod command should have failed for a non-existent groupID, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand,
                                 f"Send ExpireGracePeriod command error should be {Status.InvalidCommand} instead of {e.status}")

if __name__ == "__main__":
    default_matter_test_main()
