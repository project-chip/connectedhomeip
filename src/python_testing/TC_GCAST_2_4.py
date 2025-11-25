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
from mobly import asserts
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.interaction_model import InteractionModelError, Status

logger = logging.getLogger(__name__)


class TC_GCAST_2_4(MatterBaseTest):
    def desc_TC_GCAST_2_4(self):
        return "[TC-GCAST-2.4] ExpireGracePeriod command effect with DUT as Server - PROVISIONAL"

    def steps_TC_G_2_4(self):
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
        ]

    def pics_TC_GCAST_2_4(self) -> list[str]:
        pics = ["GCAST.S", "GCAST.S.C02.Rsp", "GCAST.S.C03.Rsp"]
        return pics

    async def get_feature_map(self):
        """Get supported features."""
        feature_map = await self.read_single_attribute_check_success(
            Clusters.Groupcast,
            Clusters.Groupcast.Attributes.FeatureMap)
        ln_enabled = bool(feature_map & Clusters.Groupcast.Bitmaps.Feature.kListener)
        sd_enabled = bool(feature_map & Clusters.Groupcast.Bitmaps.Feature.kSender)
        logger.info(f"FeatureMap: {feature_map} : LN supported: {ln_enabled} | SD supported: {sd_enabled}")
        return ln_enabled, sd_enabled

    async def retrieve_join_group_cmd_endpoints(self, ln_enabled, sd_enabled):
        """Get the JoinGroup cmd endpoints list based on enabled features such as Listener/Sender."""
        join_group_cmd_endpoints = None
        if sd_enabled and not ln_enabled:
            join_group_cmd_endpoints = []
        elif ln_enabled:
            device_type_list = await self.read_single_attribute_all_endpoints(
                cluster=Clusters.Descriptor,
                attribute=Clusters.Descriptor.Attributes.DeviceTypeList)
            logging.info(f"Device Type List: {device_type_list}")
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
                        logging.info(f"Server List: {server_list}")
                        if Clusters.OnOff.id in server_list:
                            join_group_cmd_endpoints.append(endpoint)
            asserts.assert_true(len(join_group_cmd_endpoints),
                                "Endpoint list should not be empty. There should be a valid endpoint for the GroupCast JoinGroup Command.")
        else:
            asserts.assert_is_none(join_group_cmd_endpoints,
                                   "Endpoint list is None, meaning Listener and Sender feature are both disabled.")
        return join_group_cmd_endpoints


    @async_test_body
    async def test_TC_GCAST_2_4(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 1
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        self.step("1a")
        ln_enabled, sd_enabled = await self.get_feature_map()
        join_group_cmd_endpoints = await self.retrieve_join_group_cmd_endpoints(ln_enabled, sd_enabled)

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
        keyID2 = 2
        gracePeriod = 5
        inputKey2 = bytes.fromhex("d0d1e2d3d4d5d6d7d8d9dadbdcdddedf")
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=join_group_cmd_endpoints,
            keyID=keyID2,
            gracePeriod=gracePeriod,
            key=inputKey2)
        )

        self.step(2)
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
        asserts.assert_is_not_none(group1_entry.expiringKeyID, "ExpiringKeyID should be present")
        asserts.assert_equal(group1_entry.expiringKeyID, keyID1,
                             f"Expected ExpiringKeyID={keyID1}, got {group1_entry.expiringKeyID}")

        self.step(3)
        await self.send_single_cmd(Clusters.Groupcast.Commands.ExpireGracePeriod(groupID=groupID1))

        self.step(4)
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
        asserts.assert_is_none(group1_entry.expiringKeyID, "ExpiringKeyID should not be present")

        self.step(5)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.ExpireGracePeriod(groupID=groupID1))
            asserts.fail("ExpireGracePeriod command should have failed when there is no ExpiringKey, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure,
                                 f"Send ExpireGracePeriod command error should be {Status.Failure} instead of {e.status}")

if __name__ == "__main__":
    default_matter_test_main()
