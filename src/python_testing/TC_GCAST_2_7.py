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


class TC_GCAST_2_7(MatterBaseTest):
    def desc_TC_GCAST_2_7(self):
        return "[TC-GCAST-2.5] LeaveGroup partial & full removal with DUT as Server - PROVISIONAL"

    def steps_TC_G_2_7(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeyID on the DUT"),
            TestStep(2, "Iteratively Join group until the Group count = M_max using a new GroupId and KeyId every time: "
                        "TH sends command JoinGroup (GroupID=Gn, Endpoints='see notes', KeyID=Kn, Key=InputKeyn) until the Group count = M_max"),
            TestStep(3, "Attempt to join 1 additional group: TH sends command JoinGroup (GroupID=Gn+1, Endpoints='see notes', KeyID=Kn+1, Key=InputKeyn+1)"),
            TestStep(4, "Leave one group: TH sends command LeaveGroup (GroupID=Gn)"),
            TestStep(5, "Repeat Step 2: TH sends command JoinGroup (GroupID=Gn+1, Endpoints='see notes', KeyID=Kn+1, Key=InputKeyn+1)"),
        ]

    def pics_TC_GCAST_2_7(self) -> list[str]:
        pics = ["GCAST.S", "GCAST.S.A0001"]
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
    async def test_TC_GCAST_2_7(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 1
        groupcast_cluster = Clusters.Objects.Groupcast
        max_membership_count_attribute = Clusters.Groupcast.Attributes.MaxMembershipCount

        self.step("1a")
        ln_enabled, sd_enabled = await self.get_feature_map()
        if not ln_enabled and not sd_enabled:
            asserts.fail("At least one of the following features must be enabled: Listener or Sender.")
        endpoints_list = await self.valid_endpoints_list(ln_enabled, sd_enabled)

        # Get M_Max Value
        M_max = await self.read_single_attribute_check_success(groupcast_cluster, max_membership_count_attribute)
        asserts.assert_true(M_max >= 10, "MaxMembershipCount attribute should be >= 10")

        self.step("1b")
        groupID0 = 0
        await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=groupID0))

        self.step(2)
        for membership in range(M_max):
            groupID = membership + 1
            keyID = membership + 1
            inputKey = secrets.token_bytes(16)

            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID,
                endpoints=endpoints_list,
                keyID=keyID,
                key=inputKey)
            )

        self.step(3)
        groupIDExhausted = M_max + 1
        keyIDExhausted = M_max + 1
        inputKeyExhausted = secrets.token_bytes(16)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupIDExhausted,
                endpoints=endpoints_list,
                keyID=keyIDExhausted,
                key=inputKeyExhausted)
            )
            asserts.fail("JoinGroup command should have failed with ResourceExhausted, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ResourceExhausted,
                                 f"Send JoinGroup command error should be {Status.ResourceExhausted} instead of {e.status}")

        self.step(4)
        await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=M_max))

        self.step(5)
        groupIDLimit = M_max
        keyIDLimit = M_max
        inputKeyLimit = secrets.token_bytes(16)
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupIDLimit,
            endpoints=endpoints_list,
            keyID=keyIDLimit,
            key=inputKeyLimit)
        )


if __name__ == "__main__":
    default_matter_test_main()
