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
# === BEGIN CI TEST ARGUMENTS === priyamal, check what to put as the test arguments
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


class TC_GCAST_2_2(MatterBaseTest):
    def desc_TC_GCAST_2_2(self):
        return "[TC-GCAST-2.2] JoinGroup as Listener or Sender with DUT as Server - Provisional"

    def steps_TC_GCAST_2_2(self):
        return [TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
                TestStep("1b", "TH removes any existing group and KeyID on the DUT"),
                TestStep("1c", "Th subscribes to Membership attribute with min interval 0s and max interval 30s"),
                TestStep(2, "If GCAST.S.F00(LN) feature is not supported on the cluster, skip to step 12"),
                TestStep("3a", "Attempt to join Group G1 with a new Key with KeyID K1 on Endpoint EP1"),
                TestStep("3b", "TH awaits subscription report of new membership within max interval"),
                TestStep("4a", "If DUT only support one non-root and non-aggregator endpoint, skip to step 5a"),
                TestStep("4b", "Attempt to add EP2 to Group G1"),
                TestStep("4c", "TH awaits subscription report of new membership within max interval"),
                TestStep("5a", "Attempt to join Group G2 with existing Key1 and using Auxiliary ACL"),
                TestStep("5b", "TH awaits subscription report of new membership within max interval"),
                TestStep("6a", "Attempt to join Group G2 with new Key and providing a grace period for K1"),
                TestStep("6b", "TH awaits subscription report of new membership within max interval"),
                TestStep("6c", "TH waits grace period plus tolerance"),
                TestStep("6d", "TH awaits subscription report of new membership within max interval"),
                TestStep(7, "Attempt to join Group G3 using a new Key but providing existing KeyID (result: already exists)"),
                TestStep(8, "Attempt to join Group G3 using a new KeyId, but without providing InputKey (result: not found)"),
                TestStep(9, "Attempt to join Group G3 with invalid endpoint (result: unsupported endpoint)"),
                TestStep(10, "If Sender is supported in DUT, skip this step. Else, attempt to join Group G3 with an empty endpoints list (result: constraint error)"),
                TestStep(11, "If DUT has more than 20 endpoints, attempt to join Group G3 with 21 endpoints (result: constraint error)"),
                TestStep(12, "If Sender is not supported in DUT, skip to step 18"),
                TestStep("13a", "Attempt to join Group G4 as Sender (no endpoints) and a new key"),
                TestStep("13b", "TH awaits subscription report of new membership within max interval"),
                TestStep("14a", "Attempt to join Group G5 as Sender using existing KeyId"),
                TestStep("14b", "TH awaits subscription report of new membership within max interval"),
                TestStep("15a", "Attempt to join Group G5 with new Key but providing existing KeyID (result: already exists)"),
                TestStep("15b", "Attempt to join Group G5 with new Key but without providing InputKey (result: not found)"),
                TestStep("16a", "Attempt to join Group G5 with new Key and providing grace period for K4"),
                TestStep("16b", "TH awaits subscription report of new membership within max interval"),
                TestStep("16c", "TH waits grace period plus tolerance"),
                TestStep("16d", "TH awaits subscription report of new membership within max interval"),
                TestStep(17, "confirm If Listener featIsSupported, skip this step. Else attempt to add endpoints to Group G5 (result: constraint error)"),
                TestStep(18, "confirm If Listener featIsSupported, skip this step. Else attempt to add endpoints to ?? (result: constraint error)"),
                TestStep(19, "JoinGroup with invalid GroupID (result: constraint error)"),
                TestStep(20, "JoinGroup with Key length != 16 (result: constraint error)"),
                TestStep(21, "JoinGroup with GracePeriod > 86400 (result: constraint error)")]

    def pics_TC_GCAST_2_2(self) -> list[str]:
        pics = ["GCAST.S"]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_2(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 0
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership
        max_membership_count_attribute = Clusters.Groupcast.Attributes.MaxMembershipCount

        # commissioning step
        self.step("1a")
        ln_enabled, sd_enabled = await get_feature_map(self)  # from TC_GCAST_common.py
        endpoints_list = await valid_endpoints_list(self, ln_enabled)
        endpoints_list = [endpoints_list[0]]

        # TH removes existing group and KeyID on the DUT
        self.step("1b")
        await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

        self.step("1c")
        sub = AttributeSubscriptionHandler(groupcast_cluster, membership_attribute)
        await sub.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        self.step(2)
        if not ln_enabled:
            self.mark_step_range_skipped("3a", 11)

        # Attempt to join a Group G1 with a new key using the following configuration:
        # {THcommand} JoinGroup (GroupID=G1, Endpoints=[EP1], KeyID=K1, Key=InputKey1)
        # success

        self.step("3a")
        # priyamal clean up unnecessary redeclarations throughout file
        groupID1 = 1
        keyID1 = 1
        inputKey1 = secrets.token_bytes(16)
        endpoint1 = endpoints_list[0]

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=endpoint1,
            keyID=keyID1,
            key=inputKey1,
            useAuxiliaryACL="false")
        )

        self.step("3b")
        membership_matcher = generate_membership_entry_matcher(
            groupID1, endpoints=endpoint1, key_id=keyID1, has_auxiliary_acl="false")
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        # If DUT only support one non-root and non-aggregator endpoint, skip to step 5a.
        self.step("4a")
        endpoint_1 = [endpoints_list[0]]
        endpoint_2 = None
        if len(endpoints_list) == 1:
            self.mark_step_range_skipped("4b", "4c")
        else:
            endpoint_2 = [endpoints_list[1]]

        # Attempt to add EP2 to Group G1. {THcommand} JoinGroup (G1,[EP2],K1,Key omitted)
        self.step("4b")

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            # groupID=groupID1,
            endpoints=endpoints_list[0:2],
            keyID=keyID1,
            key=inputKey1,
            useAuxiliaryACL="false")
        )

        self.step("4c")
        membership_matcher = generate_membership_entry_matcher(
            groupID1, endpoints=endpoints_list[0:2], key_id=keyID1, has_auxiliary_acl="false")
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step("5a")
        groupID2 = 2
        keyID1 = 2

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=endpoint1,
            keyID=keyID1,
            useAuxiliaryACL="true")
        )

        self.step("5b")
        membership_matcher = generate_membership_entry_matcher(
            groupID2, endpoints=endpoint1, key_id=keyID1, has_auxiliary_acl="true")
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        # grace period
        # Attempt to join Group G2 using a new Key and providing a grace period for K1:
        # {THcommand} JoinGroup (GroupID=G2, Endpoints=[EP1], KeyID=K2, Key=InputKey2, GracePeriod=GP)
        self.step("6a")
        # groupID2 = 2
        keyID2 = 2
        inputKey2 = secrets.token_bytes(16)
        gracePeriodSeconds = 5

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=endpoint1,
            keyID=keyID2,
            gracePeriod=gracePeriodSeconds,
            key=inputKey2,
            useAuxiliaryACL="true")
        )

        self.step("6b")
        membership_matcher = generate_membership_entry_matcher(
            groupID2, key_id=keyID2, expiring_key_id=keyID1, has_auxiliary_acl="true")
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step("6c")
        gracePeriodWaitingTime = max(gracePeriodSeconds * 1.1, 30)
        logger.info(f"Waiting for {gracePeriodWaitingTime:.1f} seconds for grace period to make sure it expired.")
        time.sleep(gracePeriodWaitingTime)

        self.step("6d")
        # sub.reset()  priyamal ?
        membership_matcher = generate_membership_entry_matcher(
            groupID2, key_id=keyID2, expiring_key_id_must_not_exist=True, has_auxiliary_acl="true")
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(7)
        groupID3 = 3
        inputKey3 = secrets.token_bytes(16)

        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID3,
                endpoints=endpoint1,
                keyID=keyID2,
                key=inputKey3)
            )
            asserts.fail(
                "JoinGroup command should have failed because Group with keyID already exists and does not match key, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.AlreadyExists,
                                 f"Send JoinGroup command error should be {Status.AlreadyExists} instead of {e.status}")

        self.step(8)
        keyID3 = 3
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID3,
                endpoints=endpoint1,
                keyID=keyID3)
            )
            asserts.fail("JoinGroup command should have failed because no Key found, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Send JoinGroup command error should be {Status.NotFound} instead of {e.status}")

        self.step(9)
        endpoint_invalid = -1
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID3,
                endpoints=endpoint_invalid,
                keyID=keyID1)
            )
            asserts.fail("JoinGroup command should have failed because endpoint is invalid (not in uint16 format), but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedEndpoint,
                                 f"Send JoinGroup command error should be {Status.UnsupportedEndpoint} instead of {e.status}")

        self.step(10)
        if sd_enabled:
            self.mark_step_range_skipped(10, 10)  # How to just skip to 11?

        endpoints_list_empty = []
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID3,
                endpoints=endpoints_list_empty,
                keyID=keyID1)
            )
            asserts.fail("JoinGroup command should have failed because endpoints list is empty, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        self.step(11)
        if len(endpoints_list) > 20:
            endpoint21 = 21
            # length of this list is now one more than the length of the list of endpoints in DUT
            endpoints_list_exceeds_DUT_endpoints = endpoints_list.append(endpoint21)
            try:
                await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID3,
                    endpoints=endpoints_list_exceeds_DUT_endpoints,
                    keyID=keyID2)
                )
                asserts.fail(
                    "JoinGroup command should have failed because endpoints list has more endpoints than DUT provides, but it still succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError,
                                     f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        self.step(12)
        if not sd_enabled:
            self.mark_step_range_skipped(13, 17)

        self.step("13a")
        groupID4 = 4
        keyID4 = 4
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID4,
            endpoints=endpoints_list_empty,
            keyID=keyID4)
        )

        self.step("13b")
        membership_matcher = generate_membership_entry_matcher(
            groupID4, key_id=keyID4, endpoints=[], expiring_key_id_must_not_exist=true)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step("14a")
        groupID5 = 5
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID5,
            endpoints=endpoints_list_empty,
            keyID=keyID4)
        )

        self.step("14b")
        membership_matcher = generate_membership_entry_matcher(
            groupID5, key_id=keyID4, endpoints=[], expiring_key_id_must_not_exist=true)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step("15a")
        inputKey6 = secrets.token_bytes(16)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID5,
                endpoints=endpoints_list_empty,
                keyID=keyID4,
                key=inputKey6)
            )
            asserts.fail(
                "JoinGroup command should have failed because Group with keyID already exists and does not match key, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.AlreadyExists,
                                 f"Send JoinGroup command error should be {Status.AlreadyExists} instead of {e.status}")

        self.step("15b")
        keyID5 = 5
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID5,
                endpoints=endpoints_list_empty,
                keyID=keyID5)
            )
            asserts.fail("JoinGroup command should have failed because no Key found, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Send JoinGroup command error should be {Status.NotFound} instead of {e.status}")

        self.step("16a")
        inputKey5 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID5,
            endpoints=[],
            keyID=keyID5,
            gracePeriod=gracePeriodSeconds,
            key=inputKey5)
        )

        self.step("16b")
        membership_matcher = generate_membership_entry_matcher(
            groupID5, key_id=keyID5, endpoints=[], expiring_key_id=keyID4)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step("16c")
        gracePeriodWaitingTime = max(gracePeriodSeconds * 1.1, 30)
        logger.info(f"Waiting for {gracePeriodWaitingTime:.1f} seconds for grace period to make sure it expired.")
        time.sleep(gracePeriodWaitingTime)

        self.step("16d")  # priyamal doc says G4, confirm should be G5
        membership_matcher = generate_membership_entry_matcher(
            groupID5, key_id=keyID5, expiring_key_id_must_not_exist=True)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(17)
        if ln_enabled:
            self.mark_step_range_skipped(18, 18)  # priyamal confirm This should skip to 19 right?
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID5,
                endpoints=endpoint1,
                keyID=keyID5,
                key=inputKey5)
            )
            asserts.fail("JoinGroup command should have failed because listener cannot do JoinGroup commands, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        self.step(18)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID5,
                endpoints=endpoints_list_empty,  # priyamal check if preference to use [] or the empty_list
                keyID=keyID5,
                useAuxiliaryACL="true")
            )
            asserts.fail("JoinGroup command should have failed because listener cannot do JoinGroup commands, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

        self.step(19)
        groupID0 = 0
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID0)
            )
            asserts.fail("JoinGroup command should have failed because GroupID cannot be 0, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")

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

        self.step(21)
        gracePeriodSeconds = 86401
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID2,
                endpoints=endpoint1,
                keyID=keyID2,
                gracePeriod=gracePeriodSeconds,
                key=inputKey2)
            )
            asserts.fail("JoinGroup command should have failed because GracePeriod is too long, but it still succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send JoinGroup command error should be {Status.ConstraintError} instead of {e.status}")
