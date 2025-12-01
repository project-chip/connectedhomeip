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

from src.python_testing.TC_GCAST_2_1 import is_groupcast_supporting_cluster

logger = logging.getLogger(__name__)


def membership_entry_matcher(
        group_id: int, key_id: int = None,
        expiring_key_id: int = None,
        has_auxiliary_acl: str = None,
        endpoints: list = None,
        should_not_exist: bool = False
) -> AttributeMatcher:
    """Create a matcher that checks if Membership attribute contains (or does not contain) an entry matching the specified criteria.
    
    Args:
        group_id: The groupID to match (required)
        key_id: The keyID to match (optional)
        expiring_key_id: The expiringKeyID to match (optional)
        has_auxiliary_acl: The HasAuxiliaryACL value to match (optional)
        endpoints: The endpoints list to match (optional)
        should_not_exist: If True, returns True when the entry is NOT found (default: False)

    Returns:
        An AttributeMatcher that returns True when:
        - should_not_exist=False: A Membership entry matches all specified criteria
        - should_not_exist=True: No Membership entry matches the specified criteria
    """
    def predicate(report) -> bool:
        if report.attribute != Clusters.Groupcast.Attributes.Membership:
            return False
        
        found_match = False
        for entry in report.value:
            if entry.groupID != group_id:
                continue
            if key_id is not None and entry.keyID != key_id:
                continue
            if expiring_key_id is not None:
                if entry.expiringKeyID is None or entry.expiringKeyID != expiring_key_id:
                    continue
            if has_auxiliary_acl is not None:
                if entry.hasAuxiliaryACL is None or entry.hasAuxiliaryACL != has_auxiliary_acl:
                    continue
            if endpoints is not None:
                if entry.endpoints is None or entry.endpoints != endpoints:
                    continue
            found_match = True
            break
        return found_match if not should_not_exist else not found_match
    
    desc_parts = [f"groupID={group_id}"]
    if key_id is not None:
        desc_parts.append(f"keyID={key_id}")
    if expiring_key_id is not None:
        desc_parts.append(f"expiringKeyID={expiring_key_id}")
    if has_auxiliary_acl is not None:
        desc_parts.append(f"hasAuxiliaryACL={has_auxiliary_acl}")
    if endpoints is not None:
        desc_parts.append(f"endpoints={endpoints}")
    
    if should_not_exist:
        description = f"Membership does NOT have entry with {', '.join(desc_parts)}"
    else:
        description = f"Membership has entry with {', '.join(desc_parts)}"
    
    return AttributeMatcher.from_callable(description=description, matcher=predicate)


def membership_empty_matcher() -> AttributeMatcher:
    """Create a matcher that checks if Membership attribute is empty (no groups present).
    
    Returns:
        An AttributeMatcher that returns True when the Membership list is empty.
    """
    def predicate(report) -> bool:
        if report.attribute != Clusters.Groupcast.Attributes.Membership:
            return False
        return len(report.value) == 0
    
    description = "Membership list is empty (no groups present)"
    return AttributeMatcher.from_callable(description=description, matcher=predicate)


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

    async def get_feature_map(self):
        """Get supported features."""
        feature_map = await self.read_single_attribute_check_success(
            Clusters.Groupcast,
            Clusters.Groupcast.Attributes.FeatureMap,
            endpoint=0
        )
        ln_enabled = bool(feature_map & Clusters.Groupcast.Bitmaps.Feature.kListener)
        sd_enabled = bool(feature_map & Clusters.Groupcast.Bitmaps.Feature.kSender)
        asserts.assert_true(sd_enabled or ln_enabled,
                            "At least one of the following features must be enabled: Listener or Sender.")
        logger.info(f"FeatureMap: {feature_map} : LN supported: {ln_enabled} | SD supported: {sd_enabled}")
        return ln_enabled, sd_enabled

    async def valid_endpoints_list(self, ln_enabled, sd_enabled):
        """
        Get the JoinGroup cmd endpoints list based on enabled features such as Listener/Sender.
        If only Sender is enabled, endpoints list is empty. If listener is enabled, the endpoint list is the
        first valid endpoint (excluding root and aggregator), [EP1].
        """
        endpoints_list = []
        if ln_enabled:
            device_type_list = await self.read_single_attribute_all_endpoints(
                cluster=Clusters.Descriptor,
                attribute=Clusters.Descriptor.Attributes.DeviceTypeList)
            logging.info(f"Device Type List: {device_type_list}")
            for endpoint, device_types in device_type_list.items():
                if endpoint == 0:
                    continue
                for device_type in device_types:
                    if device_type.deviceType == 14:  # Aggregator
                        continue
                    else:
                        server_list = await self.read_single_attribute_check_success(
                            cluster=Clusters.Descriptor,
                            attribute=Clusters.Descriptor.Attributes.ServerList,
                            endpoint=endpoint)
                        logging.info(f"Server List: {server_list}")
                        for cluster in server_list:
                            if is_groupcast_supporting_cluster(cluster):
                                endpoints_list.append(endpoint)
                                break
            asserts.assert_true(len(endpoints_list) > 0,
                                "Listener feature is enabled. Endpoint list should not be empty. There should be a valid endpoint for the GroupCast JoinGroup Command.")
        return endpoints_list

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_3(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 0
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        self.step("1a")
        ln_enabled, sd_enabled = await self.get_feature_map()
        endpoints_list = await self.valid_endpoints_list(ln_enabled, sd_enabled)

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
        membership_matcher = membership_entry_matcher(groupID1, key_id=keyID3, expiring_key_id=keyID1)
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
        membership_matcher = membership_entry_matcher(groupID1, key_id=keyID3)
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
        gracePeriodSecondsInvalidLimit = 86401
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
