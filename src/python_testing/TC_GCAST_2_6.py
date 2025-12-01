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

from src.python_testing.TC_GCAST_2_1 import is_groupcast_supporting_cluster
from src.python_testing.TC_GCAST_2_3 import membership_entry_matcher

logger = logging.getLogger(__name__)


class TC_GCAST_2_6(MatterBaseTest):
    def desc_TC_GCAST_2_6(self):
        return "[TC-GCAST-2.6] ConfigureAuxiliaryACL (Listener feature) with DUT as Server - PROVISIONAL"

    def steps_TC_G_2_6(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeyID on the DUT."),
            TestStep("1c", "TH subscribes to Membership attribute with min interval 0s and max interval 30s."),
            TestStep("1d", "Join group G1 generating a new KeyID K1 with Key InputKey1: TH sends command JoinGroup (GroupID=G1, Endpoints=[EP1], KeyID=K1, Key=InputKey1)."),
            TestStep(2, "Enable Auxiliary ACL on group G1: TH sends command ConfigureAuxiliaryACL (GroupID=G1, Enable=true)."),
            TestStep(3, "TH awaits subscription report of new Membership within max interval."),
            TestStep(4, "Disable Auxiliary ACL on group G1: TH sends command ConfigureAuxiliaryACL (GroupID=G1, Enable=false)."),
            TestStep(5, "TH awaits subscription report of new Membership within max interval."),
            TestStep(6, "Attempt to enable Auxiliary ACL on a unknown GroupId: TH sends command ConfigureAuxiliaryACL (GroupID=G_UNKNOWN, Enable=true)."),
            TestStep(7, "If GCAST.S.F01(SD) feature is supported on the cluster, join group G2 as Sender: TH sends command JoinGroup (GroupID=G2, Endpoints=[],KeyId=K1) to join group as sender only."),
            TestStep(8, "If GCAST.S.F01(SD) feature is supported on the cluster, attempt to enable Auxiliary ACL on group G2: TH sends command ConfigureAuxiliaryACL (GroupID=G2, Enable=true) on Sender-only membership"),
        ]

    def pics_TC_GCAST_2_6(self) -> list[str]:
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

    async def valid_endpoints_list(self) -> list:
        """
        Get the JoinGroup cmd endpoints list when the listener feature is enabled. Represents the 1 first valid non-root and
        non-aggregator endpoint.
        """
        endpoints_list = []
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
    async def test_TC_GCAST_2_6(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 0
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership

        self.step("1a")
        ln_enabled, sd_enabled = await self.get_feature_map()
        if not ln_enabled:
            logger.info("Listener feature is not enabled, skip remaining steps.")
            self.mark_all_remaining_steps_skipped("1b")
        listener_endpoints_list = await self.valid_endpoints_list()

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
            endpoints=listener_endpoints_list,
            keyID=keyID1,
            key=inputKey1)
        )

        self.step(2)
        await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(
            groupID=groupID1,
            ConfigureAuxiliaryACL=True)
        )

        self.step(3)
        membership_matcher = membership_entry_matcher(groupID1, has_auxiliary_acl="true")
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(4)
        await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(
            groupID=groupID1,
            ConfigureAuxiliaryACL=False)
        )

        self.step(5)
        sub.reset()
        membership_matcher = membership_entry_matcher(groupID1, has_auxiliary_acl="false")
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step(6)
        try:
            groupIDUnknown = 2
            await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(
                groupID=groupIDUnknown,
                ConfigureAuxiliaryACL=True)
            )
            asserts.fail("ConfigureAuxiliaryACL command should have failed with an unknown GroupID, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Send ConfigureAuxiliaryACL command error should be {Status.NotFound} instead of {e.status}")

        if not sd_enabled:
            self.mark_all_remaining_steps_skipped("7")

        self.step(7)
        groupID2 = 2
        endpoints = []
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=endpoints,
            keyID=keyID1)
        )

        self.step(8)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.ConfigureAuxiliaryACL(
                groupID=groupID2,
                ConfigureAuxiliaryACL=True)
            )
            asserts.fail("ConfigureAuxiliaryACL command should have failed with on Sender only, but it succeeded.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Failure,
                                 f"Send ConfigureAuxiliaryACL command error should be {Status.Failure} instead of {e.status}")


if __name__ == "__main__":
    default_matter_test_main()
