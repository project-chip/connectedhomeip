#    Copyright (c) 2025 Project CHIP Authors
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
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CHIP_TV_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_command, run_if_endpoint_matches

logger = logging.getLogger(__name__)


class TC_CHANNEL_5_4(MatterBaseTest):
    def desc_TC_CHANNEL_5_4(self) -> str:
        return "[TC-CHANNEL-5.4] Change Channel Verification"

    def steps_TC_CHANNEL_5_4(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commission TH to DUT", is_commissioning=True),
            TestStep(1, "TH sends ChangeChannel command to DUT"),
        ]
        return steps

    def pics_TC_CHANNEL_5_4(self) -> list[str]:
        return ["CHANNEL.C"]

    @run_if_endpoint_matches(has_command(Clusters.Channel.Commands.ChangeChannel))
    async def test_TC_CHANNEL_5_4(self):
        # Step 0: Commission TH to DUT
        self.step(0)

        # Step 1: TH sends ChangeChannel command to DUT
        self.step(1)
        endpoint = self.get_endpoint()

        logger.info(f"Using endpoint: {endpoint}")

        # 1. Read ChannelList attribute
        channel_list = await self.read_single_attribute_check_success(
            cluster=Clusters.Channel,
            attribute=Clusters.Channel.Attributes.ChannelList,
            endpoint=endpoint
        )
        logger.info(f"Read ChannelList: {channel_list}")

        # 2. Handle empty list
        if not channel_list:
            asserts.fail("ChannelList is empty.")
        else:
            # 3. Pick from list (using the first available channel)
            target_match = channel_list[0].name
            logger.info(f"Selected channel from list: {target_match}")

        # 4. Send ChangeChannel command
        logger.info(f"Sending ChangeChannel command with match='{target_match}' to endpoint {endpoint}")
        cmd = Clusters.Channel.Commands.ChangeChannel(match=target_match)
        await self.send_single_cmd(cmd, endpoint=endpoint)
        logger.info("ChangeChannel command sent successfully.")

        # 5. Verify CurrentChannel matches
        current_channel = await self.read_single_attribute_check_success(
            cluster=Clusters.Channel,
            attribute=Clusters.Channel.Attributes.CurrentChannel,
            endpoint=endpoint
        )
        logger.info(f"Read CurrentChannel: {current_channel}")

        # 6. Verify that the current channel name matches the requested target
        asserts.assert_equal(current_channel.name, target_match,
                             f"CurrentChannel name ({current_channel.name}) does not match requested target ({target_match})")
        logger.info("Verification successful: CurrentChannel matches target.")


if __name__ == "__main__":
    default_matter_test_main()
