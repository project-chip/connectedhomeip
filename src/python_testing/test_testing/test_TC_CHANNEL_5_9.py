#
#    Copyright (c) 2024 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at:
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${TV_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
import logging
from chip.interaction_model import Status
from chip.clusters import Objects as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, pics
from mobly import asserts


class TC_CHANNEL_5_9(MatterBaseTest):
    def desc_TC_CHANNEL_5_9(self) -> str:
        return "[TC-CHANNEL-5.9] Change Channel Verification (DUT as Server)"

    def steps_TC_CHANNEL_5_9(self) -> list[TestStep]:
        return [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("1", "Read the ChannelList attribute"),
            TestStep("2", "Read the CurrentChannel attribute"),
            TestStep("3", "Send ChangeChannel command with a different valid Match string"),
            TestStep("4", "Verify CurrentChannel updated to the selected channel")
        ]

    def pics_TC_CHANNEL_5_9(self) -> list[str]:
        return ["CHANNEL.S"]

    async def read_channel_list(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Channel,
            attribute=Clusters.Channel.Attributes.ChannelList
        )

    async def read_current_channel(self, endpoint):
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Channel,
            attribute=Clusters.Channel.Attributes.CurrentChannel
        )

    async def send_change_channel_command(self, endpoint, match_str):
        return await self.default_controller.SendCommand(
            nodeid=self.dut_node_id,
            endpoint=endpoint,
            payload=Clusters.Channel.Commands.ChangeChannel(match=match_str)
        )

    def extract_channel_identifier(self, channel):
        """Returns a prioritized match string from the channel struct as per spec."""
        # Priority:NAME > Identifier > AffiliateCallSign > CallSign > Major.Minor
        if getattr(channel, "name", None):
            return channel.name
        if getattr(channel, "identifier", None):
            return channel.identifier
        if getattr(channel, "affiliateCallSign", None):
            return channel.affiliateCallSign
        if getattr(channel, "callSign", None):
            return channel.callSign
        if hasattr(channel, "majorNumber") and hasattr(channel, "minorNumber"):
            return f"{channel.majorNumber}.{channel.minorNumber}"
        return None

    @async_test_body
    async def test_TC_CHANNEL_5_9(self):
        endpoint = self.user_params.get("endpoint", 1)

        # Step 0: Commissioning already done by test-runner setup
        self.step("0")

        # Step 1: Read the ChannelList attribute and validate it's a list
        if pics("CHANNEL.S.A0000"):
            self.step("1")
            channel_list = await self.read_channel_list(endpoint)
            asserts.assert_is_instance(channel_list, list, "ChannelList must be a list.")
            logging.info(f"Available channels: {channel_list}")
         # Filter out channels that don't have any matchable identifiers
            valid_channels = [ch for ch in channel_list if self.extract_channel_identifier(ch)]
            asserts.assert_true(len(valid_channels) >= 2, "Need at least 2 valid channels for test.")
        else:
            self.skip_step("1")

         # Step 2: Read the CurrentChannel attribute to store current state before switching
        if pics("CHANNEL.S.A0002"):
            self.step("2")
            current_channel = await self.read_current_channel(endpoint)
            logging.info(f"CurrentChannel: {current_channel}")
            current_identifier = self.extract_channel_identifier(current_channel) if current_channel else None
        else:
            self.skip_step("2")

         # Step 3: Send ChangeChannel command with a different valid Match string than current
        if pics("CHANNEL.S.C00.Rsp"):
            self.step("3")
            selected_channel = next(
                (ch for ch in valid_channels if self.extract_channel_identifier(ch) != current_identifier), None)
            asserts.assert_is_not_none(selected_channel, "No different channel found for switching.")

            match_str = self.extract_channel_identifier(selected_channel)
            logging.info(f"Selected match string for ChangeChannel: {match_str}")

            response = await self.send_change_channel_command(endpoint, match_str)
            asserts.assert_equal(response.status, Status.Success, f"Expected Success status, got {response.status}")
        else:
            self.skip_step("3")

         # Step 4: Read CurrentChannel again to verify it has updated to the newly selected channel
        if pics("CHANNEL.S.A0002"):
            self.step("4")
            updated_channel = await self.read_current_channel(endpoint)
            updated_identifier = self.extract_channel_identifier(updated_channel)
            logging.info(f"Updated CurrentChannel: {updated_channel}")
            asserts.assert_equal(updated_identifier.lower(), match_str.lower(), "Channel did not update to expected value")
        else:
            self.skip_step("4")
            
if __name__ == "__main__":
    default_matter_test_main()
