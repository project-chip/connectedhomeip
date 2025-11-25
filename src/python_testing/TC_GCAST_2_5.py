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
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.interaction_model import InteractionModelError, Status

logger = logging.getLogger(__name__)


class TC_GCAST_2_5(MatterBaseTest):
    def desc_TC_GCAST_2_5(self):
        return "[TC-GCAST-2.5] LeaveGroup partial & full removal with DUT as Server - PROVISIONAL"

    def steps_TC_G_2_5(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeyID on the DUT"),
            TestStep("1c", "TH subscribes to Membership attribute with min interval 0s and max interval 30s"),
            TestStep("1d", "Join Group G1 generating a new Key with KeyID K1 using JoinGroup"),
            TestStep("1e", "Join Group G2 with existing KeyId K1: TH sends command JoinGroup"),
            TestStep("2a", "Completely Leave Group G2 by omitting the endpoint list parameters: TH sends command LeaveGroup."),
            TestStep("2b", "TH awaits subscription report of new Membership within max interval."),
            TestStep(3, "If GCAST.S.F00(LN) feature is not supported on the cluster skip to step 5"),
            TestStep("4a", "Join Group G3 with existing KeyId K1: TH sends command JoinGroup"),
            TestStep("4b", "If DUT only support one non-root and non-aggregator endpoint, skip to step 4e."),
            TestStep("4c", "Remove EP2 from Group G3: TH sends command LeaveGroup"),
            TestStep("4d", "TH awaits subscription report of new Membership within max interval."),
            TestStep("4e", "Remove EP1 from Group G3: TH sends command LeaveGroup"),
            TestStep("4f", "TH awaits subscription report of new Membership within max interval."),
            TestStep(5, "Attempt to Leave a non-existing group: TH sends command LeaveGroup"),
            TestStep(6, "Leave all groups: TH sends command LeaveGroup with GroupID=0"),
            TestStep(7, "TH awaits subscription report of new Membership within max interval."),
        ]

    def pics_TC_GCAST_2_5(self) -> list[str]:
        pics = ["GCAST.S", "GCAST.S.C02.Rsp", "GCAST.S.C05.Tx"]
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

    async def valid_endpoints_list(self, ln_enabled, sd_enabled):
        """
        Get the JoinGroup cmd endpoints list based on enabled features such as Listener/Sender.
        If only Sender is enabled, endpoints list is empty. If listener is enabled and only 1 endpoint is valid (excluding
        root and aggregator), endpoints list is [EP1]. If more than 1 endpoint is supported, endpoints list is [EP1, EP2].
        """
        endpoints_list = []
        if sd_enabled and not ln_enabled:
            endpoints_list = []
        elif ln_enabled:
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
                        if Clusters.OnOff.id in server_list:
                            endpoints_list.append(endpoint)
            asserts.assert_true(len(endpoints_list),
                                "Listener feature is enabled. Endpoint list should not be empty. There should be a valid endpoint for the GroupCast JoinGroup Command.")
            if len(endpoints_list) == 1:
                endpoints_list = [endpoints_list[0]]
            else:
                endpoints_list = endpoints_list[:2]
        return endpoints_list


    @async_test_body
    async def test_TC_GCAST_2_5(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 1
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        self.step("1a")
        ln_enabled, sd_enabled = await self.get_feature_map()
        if not ln_enabled and not sd_enabled:
            asserts.fail("At least one of the following features must be enabled: Listener or Sender.")
        endpoints_list = await self.valid_endpoints_list(ln_enabled, sd_enabled)

        self.step("1b")
        groupID0 = 0
        await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=groupID0))

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
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=endpoints_list,
            keyID=keyID1)
        )

        self.step("2a")
        resp: Clusters.Groupcast.Commands.LeaveGroupResponse = await self.send_single_cmd(
            Clusters.Groupcast.Commands.LeaveGroup(groupID=groupID2)
        )
        asserts.assert_is_not_none(resp.endpoints, "LeaveGroupResponse endpoints should not be None")
        asserts.assert_equal(resp.endpoints, endpoints_list,
                             f"LeaveGroupResponse cmd endpoints list {resp.endpoints} is not equal to the endpoints list provided in step 1e {endpoints_list}")

        self.step("2b")
        sub.wait_for_attribute_report()
        membership_reports = sub.attribute_reports.get(membership_attribute, [])
        asserts.assert_greater(len(membership_reports), 0, "No membership reports received")
        latest_membership = membership_reports[-1].value
        for entry in latest_membership:
            if entry.groupID == groupID2:
                asserts.fail(f"Group with GroupID {groupID2} should not be present in membership report")

        self.step("3")
        if not ln_enabled:
            self.mark_step_range_skipped("4a", "4f")

        self.step("4a")
        groupID3 = 3
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID3,
            endpoints=endpoints_list,
            keyID=keyID1)
        )

        self.step("4b")
        endpoint_1 = [endpoints_list[0]]
        if len(endpoints_list) == 1:
            self.mark_step_range_skipped("4c", "4d")
        else:
            endpoint_2 = [endpoints_list[1]]

        self.step("4c")
        resp: Clusters.Groupcast.Commands.LeaveGroupResponse = await self.send_single_cmd(
            Clusters.Groupcast.Commands.LeaveGroup(
                groupID=groupID3,
                endpoints=endpoint_2)
        )
        asserts.assert_is_not_none(resp.endpoints, "LeaveGroupResponse endpoints should not be None")
        asserts.assert_equal(resp.endpoints, endpoint_2,
                             f"LeaveGroupResponse cmd endpoints list {resp.endpoints} is not equal to the removed endpoints {endpoint_2}")

        self.step("4d")
        sub.wait_for_attribute_report()
        membership_reports = sub.attribute_reports.get(membership_attribute, [])
        asserts.assert_greater(len(membership_reports), 0, "No membership reports received")
        latest_membership = membership_reports[-1].value
        group3_entry = None
        for entry in latest_membership:
            if entry.groupID == groupID3:
                group3_entry = entry
                break
        asserts.assert_is_not_none(group3_entry, f"Group {groupID3} not found in membership report")
        asserts.assert_equal(endpoint_1, group3_entry.endpoints,
                             f"Group {groupID3} endpoints {group3_entry.endpoints} should equal remaining endpoints {endpoint_1} after removing {endpoint_2}")


        self.step("4e")
        resp: Clusters.Groupcast.Commands.LeaveGroupResponse = await self.send_single_cmd(
            Clusters.Groupcast.Commands.LeaveGroup(
                groupID=groupID3,
                endpoints=endpoint_1)
        )
        asserts.assert_is_not_none(resp.endpoints, "LeaveGroupResponse endpoints should not be None")
        asserts.assert_equal(resp.endpoints, endpoint_1,
                             f"LeaveGroupResponse cmd endpoints list {resp.endpoints} is not equal to the removed endpoints {endpoint_1}")

        self.step("4f")
        sub.wait_for_attribute_report()
        membership_reports = sub.attribute_reports.get(membership_attribute, [])
        asserts.assert_greater(len(membership_reports), 0, "No membership reports received")
        latest_membership = membership_reports[-1].value
        group3_entry = None
        for entry in latest_membership:
            if entry.groupID == groupID3:
                group3_entry = entry
                break
        if sd_enabled:
            asserts.assert_is_not_none(group3_entry, f"Group {groupID3} should exist when Sender feature is enabled")
            asserts.assert_equal(group3_entry.endpoints, [], 
                                f"Group {groupID3} should have empty endpoints list when Sender feature is enabled, got {group3_entry.endpoints}")
        else:
            asserts.assert_is_none(group3_entry, "Sender feature is not enabled, G3 entry should not exist")

        self.step("5")
        groupIDNonExisting = 4
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(
                groupID=groupIDNonExisting)
            )
            asserts.fail("LeaveGroup command should have failed with a non-existing group, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Send LeaveGroup command error should be {Status.NotFound} instead of {e.status}")

        self.step("6")
        resp: Clusters.Groupcast.Commands.LeaveGroupResponse = await self.send_single_cmd(
            Clusters.Groupcast.Commands.LeaveGroup(
                groupID=groupID0)
        )
        if resp.listTooLarge is not True:
            asserts.assert_equal(resp.endpoints, endpoints_list,
                                 f"LeaveGroupResponse endpoints list {resp.endpoints} must be equal to the endpoint list provided for G1 at step 1a {endpoints_list}.")

        self.step("7")
        sub.wait_for_attribute_report()
        membership_reports = sub.attribute_reports.get(membership_attribute, [])
        asserts.assert_equal(len(membership_reports), 0,
                             f"Expected no membership reports, but received {len(membership_reports)} reports.")

        self.step("8")
        resp: Clusters.Groupcast.Commands.LeaveGroupResponse = await self.send_single_cmd(
            Clusters.Groupcast.Commands.LeaveGroup(
                groupID=groupID0)
        )
        asserts.assert_equal(resp.endpoints, [],
                             f"LeaveGroupResponse endpoints list {resp.endpoints} must be empty.")

if __name__ == "__main__":
    default_matter_test_main()
