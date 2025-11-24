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
import time
from mobly import asserts
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.interaction_model import InteractionModelError, Status

logger = logging.getLogger(__name__)


class TC_GCAST_2_3(MatterBaseTest):
    def desc_TC_GCAST_2_3(self):
        return "[TC-GCAST-2.3] Key rotation using UpdateGroupKey with GracePeriod with DUT as Server - PROVISIONAL"

    def steps_TC_G_2_3(self):
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
        pics = ["GCAST.S", "GCAST.S.C00.Rsp", "GCAST.S.C02.Rsp"]
        return pics

    async def retrieve_join_group_cmd_endpoints(self):
        feature_map = await self.read_single_attribute_check_success(
            Clusters.Groupcast,
            Clusters.Groupcast.Attributes.FeatureMap)
        listener_bitmask = Clusters.Groupcast.Bitmaps.Feature.kListener
        sender_bitmask = Clusters.Groupcast.Bitmaps.Feature.kSender

        ln_enabled = feature_map & listener_bitmask
        sd_enabled = feature_map & sender_bitmask
        if sd_enabled and not ln_enabled:
            join_group_cmd_endpoints = []
        elif ln_enabled:
            device_type_list = await self.read_single_attribute_all_endpoints(
                cluster=Clusters.Descriptor,
                attribute=Clusters.Descriptor.Attributes.DeviceTypeList)
            logging.info(f"Device Type List: {device_type_list}") # Example output -> device_type_list = {0: [Descriptor.Structs.DeviceTypeStruct(deviceType=18, revision=1), Descriptor.Structs.DeviceTypeStruct(deviceType=22, revision=3)], 1: [Descriptor.Structs.DeviceTypeStruct(deviceType=14, revision=2)]}
            join_group_cmd_endpoints = []
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
                        logging.info(f"Server List: {server_list}") # Example output -> [63, 62, 60, 51, 48, 44, 43, 40, 31, 29, 49, 42, 53]
                        # TODO: will need to check if a cluster supports groupcast commands, then add it as a valid endpoint. Which clusters should be here apart from OnOff?
                        if Clusters.OnOff.id in server_list:
                            join_group_cmd_endpoints.append(endpoint)
                            break # TODO: 1 endpoint is enough here?
            asserts.assert_true(len(join_group_cmd_endpoints),
                                "Endpoint list should not be empty. There should be a valid endpoint for the GroupCast JoinGroup Command.")
        return join_group_cmd_endpoints

    @async_test_body
    async def test_TC_GCAST_2_3(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 1
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        self.step("1a")
        join_group_cmd_endpoints = self.retrieve_join_group_cmd_endpoints()

        self.step("1b")
        await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

        self.step("1c")
        sub = AttributeSubscriptionHandler(groupcast_cluster, membership_attribute)
        await sub.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        self.step("1d")
        groupID1 = 1
        keyID1 = 1
        inputKey1 = bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf")

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=join_group_cmd_endpoints,
            keyID=keyID1,
            key=inputKey1)
        )

        self.step("1e")
        groupID2 = 2
        keyID2 = 2
        inputKey2 = bytes.fromhex("d0d1e2d3d4d5d6d7d8d9dadbdcdddedf")
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=join_group_cmd_endpoints,
            keyID=keyID2,
            key=inputKey2)
        )

        self.step(2)
        keyID3 = 3
        gracePeriod = 5
        inputKey3 = bytes.fromhex("d0d1e2d3d4d5d6d7d8d9dadbdcdddedf")
        await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
            groupID=groupID1,
            keyID=keyID3,
            gracePeriod=gracePeriod,
            key=inputKey3)
        )

        self.step(3)
        sub.wait_for_attribute_report()
        membership_reports = sub.attribute_reports.get(membership_attribute, [])
        asserts.assert_greater(len(membership_reports), 0, "No membership reports received")
        latest_membership = membership_reports[-1].value
        group1_entry = None
        for entry in latest_membership:
            if entry.groupID == groupID1:
                group1_entry = entry
                break
        asserts.assert_is_not_none(group1_entry, f"Group {groupID1} not found in membership report")
        asserts.assert_equal(group1_entry.keyID, keyID3, f"Expected KeyID={keyID3}, got {group1_entry.keyID}")
        asserts.assert_is_not_none(group1_entry.expiringKeyID, "ExpiringKeyID should be present")
        asserts.assert_equal(group1_entry.expiringKeyID, keyID1, f"Expected ExpiringKeyID={keyID1}, got {group1_entry.expiringKeyID}")

        self.step(4)
        inputKey4 = bytes.fromhex("d0d1e2d3d4d5d6d7bed9dadbdcdddedf")
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID2,
                keyID=keyID1,
                key=inputKey4)
            )
            asserts.fail("Unexpected success returned from sending UpdateGroupKey command.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.AlreadyExists,
                                 f"Send UpdateGroupKey command error should be {Status.AlreadyExists} instead of {e.status}")

        self.step(5)
        time.sleep(gracePeriod * 1.1)

        self.step(6)
        sub.wait_for_attribute_report()
        membership_reports = sub.attribute_reports.get(membership_attribute, [])
        asserts.assert_greater(len(membership_reports), 0, "No membership reports received")
        latest_membership = membership_reports[-1].value
        group1_entry = None
        for entry in latest_membership:
            if entry.groupID == groupID1:
                group1_entry = entry
                break
        asserts.assert_is_not_none(group1_entry, f"Group {groupID1} not found in membership report")
        asserts.assert_equal(group1_entry.keyID, keyID3, f"Expected KeyID={keyID3}, got {group1_entry.keyID}")
        asserts.assert_is_none(group1_entry.expiringKeyID, "ExpiringKeyID should not be present")

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
            asserts.fail("Unexpected success returned from sending UpdateGroupKey command.")
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
            asserts.fail("Unexpected success returned from sending UpdateGroupKey command.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure,
                                 f"Send UpdateGroupKey command error should be {Status.Failure} instead of {e.status}")

        self.step(10)
        inputKey4InvalidLength = bytes.fromhex("d0d1e2d3d4d5d6d7bed9dadbdcddde")
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID1,
                keyID=keyID4,
                key=inputKey4InvalidLength)
            )
            asserts.fail("Unexpected success returned from sending UpdateGroupKey command.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Send UpdateGroupKey command error should be {Status.ConstraintError} instead of {e.status}")

        self.step(11)
        gracePeriodInvalidLimit = 86401
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.UpdateGroupKey(
                groupID=groupID1,
                keyID=keyID4,
                gracePeriod=gracePeriodInvalidLimit,
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
