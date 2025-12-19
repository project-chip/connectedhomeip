#
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md"#defining-the-ci-test-arguments
# for details about the block below.
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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


import logging

from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
import matter.clusters as Clusters

logger = logging.getLogger(__name__)

class TC_CHANNEL_5_5(MatterBaseTest):
    def desc_TC_CHANNEL_5_5(self) -> str:
        return "[TC-CHANNEL-5.5] Change Channel By Number Verification (DUT as Client)"

    def steps_TC_CHANNEL_5_5(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commission TH to DUT", is_commissioning=True),
            TestStep(1, "DUT sends ChangeChannelByNumber command to TH"),
        ]
        return steps

    def pics_TC_CHANNEL_5_5(self) -> list[str]:
        return ["CHANNEL.C"]
    
    @async_test_body
    async def test_TC_CHANNEL_5_5(self):
        # Step 0: Commission TH to DUT
        self.step(0)
        # Step 1: DUT sends ChangeChannelByNumber command to TH
        self.step(1)
        if self.pics_guard(self.check_pics("CHANNEL.C.C02.Tx")):
            # Get endpoint from command line argument
            endpoint = self.get_endpoint()
            logging.info(f"Using endpoint: {endpoint}")
            # 1. Read ChannelList attribute
            channel_list = await self.read_single_attribute_check_success(
                cluster=Clusters.Channel,
                attribute=Clusters.Channel.Attributes.ChannelList,
                endpoint=endpoint
            )
            logging.info(f"Read ChannelList: {channel_list}")

            # 2. Read CurrentChannel to know which channel to avoid
            current_channel = await self.read_single_attribute_check_success(
                cluster=Clusters.Channel,
                attribute=Clusters.Channel.Attributes.CurrentChannel,
                endpoint=endpoint
            )
            logging.info(f"Read CurrentChannel before change: {current_channel}")
            # 3. Find a channel different from CurrentChannel
            # ChangeChannelByNumber returns FAILURE if target equals current channel
            target_channel = None
            for channel in channel_list:
                if (channel.majorNumber != current_channel.majorNumber or
                    channel.minorNumber != current_channel.minorNumber):
                    target_channel = channel
                    break
            if target_channel is None:
                asserts.fail(
                    "Cannot find a channel different from CurrentChannel in ChannelList. "
                    f"CurrentChannel: majorNumber={current_channel.majorNumber}, minorNumber={current_channel.minorNumber}. "
                    "ChangeChannelByNumber requires a different target channel."
                )
            target_major_number = target_channel.majorNumber
            target_minor_number = target_channel.minorNumber
            logging.info(f"Selected different channel from list: MajorNumber={target_major_number}, MinorNumber={target_minor_number}")
            # 4. Send ChangeChannelByNumber command
            logging.info(f"Sending ChangeChannelByNumber command with majorNumber={target_major_number}, minorNumber={target_minor_number} to endpoint {endpoint}")
            cmd = Clusters.Channel.Commands.ChangeChannelByNumber(
                majorNumber=target_major_number,
                minorNumber=target_minor_number
            )
            await self.send_single_cmd(cmd, endpoint=endpoint)
            logging.info("ChangeChannelByNumber command sent successfully.")
            # 5. Verify CurrentChannel matches
            current_channel = await self.read_single_attribute_check_success(
                cluster=Clusters.Channel,
                attribute=Clusters.Channel.Attributes.CurrentChannel,
                endpoint=endpoint
            )
            logging.info(f"Read CurrentChannel: {current_channel}")
            # Verify that the current channel matches the requested target
            asserts.assert_equal(current_channel.majorNumber, target_major_number,
                                 f"CurrentChannel majorNumber ({current_channel.majorNumber}) does not match requested target ({target_major_number})")
            asserts.assert_equal(current_channel.minorNumber, target_minor_number,
                                 f"CurrentChannel minorNumber ({current_channel.minorNumber}) does not match requested target ({target_minor_number})")
            logging.info("Verification successful: CurrentChannel matches target.")

if __name__ == "__main__":
    default_matter_test_main()