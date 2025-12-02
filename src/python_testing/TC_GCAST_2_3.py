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
import time
from mobly import asserts
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
import matter.clusters as Clusters
from matter.testing.matter_testing import AttributeMatcher, MatterBaseTest, TestStep, async_test_body, default_matter_test_main, has_cluster, run_if_endpoint_matches
from matter.interaction_model import InteractionModelError, Status

from src.python_testing.TC_GCAST_common import get_feature_map, valid_endpoints_list, generate_membership_entry_matcher

logger = logging.getLogger(__name__)


class TC_GCAST_2_3(MatterBaseTest):
    def desc_TC_GCAST_2_3(self):
        return "[TC-GCAST-2.3] Key rotation using UpdateGroupKey with GracePeriod with DUT as Server - PROVISIONAL"

    def steps_TC_GCAST_2_3(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeyID on the DUT"),
            TestStep("1c", "TH subscribes to Membership attribute with min interval 0s and max interval 30s"),
            TestStep("1d", "Join Group G1 generating a new Key with KeyID K1 using JoinGroup"),
            TestStep("1e", "Join Group G2 generating a new Key with KeyID K2 using JoinGroup"),
            TestStep(2, "Update Group G1 with a new KeyID K3 with a grace period for K1 using UpdateGroupKey"),
            TestStep(3, "TH awaits subscription report showing KeyID=K3, ExpiringKeyID=K1 for G1"),
            TestStep(4, "Update Group G2 generating a new Key with KeyId=ExpiringKeyID K1 (should fail)"),
            TestStep(5, "TH Wait GracePeriod(GP) + tolerance (+10%)"),
            TestStep(6, "TH awaits subscription report showing no ExpiringKeyID; only KeyID=K3 remains for G1"),
            TestStep(7, "Repeat Step 4, It SHALL now succeed: Update Group G2 with KeyID=K1"),
            TestStep(8, "Update Group G1 generating a new Key for an existing KeyID K2 (should fail)"),
            TestStep(9, "Update Group G1 with a new KeyID K4 without providing an InputKey (should fail)"),
            TestStep(10, "Update Group G1 generating a new KeyID K4 with invalid Key length (should fail)"),
            TestStep(11, "Update Group G1 generating a new KeyID K4 with GracePeriod exceeding limit (should fail)"),
            TestStep(12, "Update Group G1 with KeyID already used by another group. "),
        ]

    def pics_TC_GCAST_2_3(self) -> list[str]:
        pics = ["GCAST.S"]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_3(self):
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
        groupID2 = 2
        keyID2 = 2
        inputKey2 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=endpoints_list,
            keyID=keyID2,
            key=inputKey2)
        )

        self.step(2)
        keyID3 = 3
        gracePeriodSeconds = 5
        inputKey3 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
            groupID=groupID1,
            keyID=keyID3,
            gracePeriod=gracePeriodSeconds,
            key=inputKey3)
        )

        self.step(3)
        membership_matcher = generate_membership_entry_matcher(groupID1, key_id=keyID3, expiring_key_id=keyID1)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(4)
        inputKey4 = secrets.token_bytes(16)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID2,
                keyID=keyID1,
                key=inputKey4)
            )
            asserts.fail("UpdateGroupKey command should have failed with as keyID1 still exists, but it succeeded.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.AlreadyExists,
                                 f"Send UpdateGroupKey command error should be {Status.AlreadyExists} instead of {e.status}")

        self.step(5)
        gracePeriodWaitingTime = max(gracePeriodSeconds * 1.1, 30)
        logger.info(f"Waiting for {gracePeriodWaitingTime:.1f} seconds for grace period to make sure it expired.")
        time.sleep(gracePeriodWaitingTime)

        self.step(6)
        sub.reset()
        membership_matcher = generate_membership_entry_matcher(groupID1, key_id=keyID3, expiring_key_id_must_not_exist=True)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(7)
        await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
            groupID=groupID2,
            keyID=keyID1,
            key=inputKey4)
        )

        self.step(8)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID1,
                keyID=keyID2,
                key=inputKey3)
            )
            asserts.fail("UpdateGroupKey command should have failed with already existing keyID K2, but it succeeded.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.AlreadyExists,
                                 f"Send UpdateGroupKey command error should be {Status.AlreadyExists} instead of {e.status}")

        self.step(9)
        keyID4 = 4
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID1,
                keyID=keyID4)
            )
            asserts.fail("UpdateGroupKey command should have failed, but it succeeded.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure,
                                 f"Send UpdateGroupKey command error should be {Status.Failure} instead of {e.status}")

        self.step(10)
        inputKey4InvalidLength = secrets.token_bytes(15)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID1,
                keyID=keyID4,
                key=inputKey4InvalidLength)
            )
            asserts.fail("UpdateGroupKey command should have failed because of Key with invalid length, but it succeeded.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send UpdateGroupKey command error should be {Status.ConstraintError} instead of {e.status}")

        self.step(11)
        gracePeriodSecondsInvalidLimit = 86400 + 1
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID1,
                keyID=keyID4,
                gracePeriod=gracePeriodSecondsInvalidLimit,
                key=inputKey4)
            )
            asserts.fail("Unexpected success returned from sending UpdateGroupKey command.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send UpdateGroupKey command error should be {Status.ConstraintError} instead of {e.status}")

        self.step(12)
        await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
            groupID=groupID1,
            keyID=keyID2)
        )

if __name__ == "__main__":
    default_matter_test_main()
