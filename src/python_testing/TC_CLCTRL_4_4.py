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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import time

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

class TC_CLCTRL_4_4(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_4_4(self) -> str:
        return "[TC-CLCTRL-4.4] MoveTo Command CountdownTime with Server as DUT"

    def steps_TC_CLCTRL_4_4(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read the AttributeList"),
            TestStep("2b", "Check if CountdownTime attribute is supported"),
            TestStep(3, "Verify the CountdownTime when no operation is in progress"),
            TestStep("4a", "Prepare the DUT by moving to open position"),
            TestStep("4b", "Wait until previous movement is complete"),
            TestStep("4c", "Initiate a movement to closed position"),
            TestStep("4d", "Check CountdownTime immediately after initiating movement"),
            TestStep("4e", "Check MainState to ensure movement is occurring"),
            TestStep("5a", "Skip if no countdown time reported"),
            TestStep("5b", "Wait until the operation should be complete"),
            TestStep("5c", "Verify that the MainState is Stopped(0)"),
            TestStep("6a", "Skip if countdown time reported in Step 4c is NULL"),
            TestStep("6b", "Start a movement to open position"),
            TestStep("6c", "Check CountdownTime during movement"),
            TestStep("6d", "Stop the movement"),
            TestStep("6e", "Check CountdownTime after interrupting the movement"),
        ]
        return steps

    def pics_TC_CLCTRL_4_4(self) -> list[str]:
        pics = [
            "CLCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_4_4(self):
        asserts.assert_true('PIXIT.CLCTRL.FullMotionDuration' in self.matter_test_config.global_test_params,
                            "PIXIT.CLCTRL.FullMotionDuration must be included on the command line in "
                            "the --int-arg flag as PIXIT.CLCTRL.FullMotionDuration:<duration>")
        
        full_motion_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.FullMotionDuration']

        endpoint = self.get_endpoint(default=1)

        self.step(1)
        
        # STEP 2: Check if CountdownTime attribute is supported
        self.step("2a")
        attribute_list = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.ClosureControl.Attributes.AttributeList)
        
        self.step("2b")
        countdown_time_supported = Clusters.ClosureControl.Attributes.CountdownTime.attribute_id in attribute_list
        
        if not countdown_time_supported:
            logging.info("CountdownTime attribute not supported, skipping test")
            return
        
        # STEP 3: Verify the CountdownTime when no operation is in progress
        self.step("3a")
        countdown_time = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.ClosureControl.Attributes.CountdownTime)
        asserts.assert_equal(countdown_time, 0, "CountdownTime should be 0 when no operation is in progress")
        
        # STEP 4: Verify the CountdownTime when an operation is initiated
        self.step("4a")
        
        # Read feature map to determine feature support
        feature_map = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.ClosureControl.Attributes.FeatureMap)
        is_ps_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPosition
        
        if not is_ps_feature_supported:
            logging.info("Positioning feature not supported, skipping remaining test steps")
            return
        
        cluster = Clusters.Objects.ClosureControl
        
        # First ensure we're at open position
        await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({"position": 1}))  # OpenInFull
        
        self.step("4b")
        # Wait to ensure any previous movement is complete
        time.sleep(full_motion_duration)
        
        self.step("4c")
        # Now initiate a movement to closed position
        await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({"position": 0}))  # CloseInFull
        
        self.step("4d")
        # Check CountdownTime immediately after initiating movement
        countdown_time = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.ClosureControl.Attributes.CountdownTime)
        
        # CountdownTime should be either a positive value or NULL
        if countdown_time is not None:
            asserts.assert_true(countdown_time > 0, "CountdownTime should be positive during movement")
            stored_countdown_time = countdown_time
        else:
            logging.info("CountdownTime returned NULL, device does not provide time estimates")
            stored_countdown_time = None
        
        self.step("4e")
        # Check MainState to ensure movement is occurring
        main_state = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.ClosureControl.Attributes.MainState)
        asserts.assert_equal(main_state, 1, "MainState should be Moving(1) during motion")
        
        # STEP 5: Verify the CountdownTime becomes 0 when an operation completes
        self.step("5a")
        if stored_countdown_time is None:
            logging.info("CountdownTime returned NULL, skipping remaining steps.")
            return
        
        self.step("5b")
        # Wait until the operation should be complete (with some margin)
        time.sleep(stored_countdown_time + 2)
        
        # Check CountdownTime after operation should be complete
        countdown_time = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.ClosureControl.Attributes.CountdownTime)
        asserts.assert_equal(countdown_time, 0, "CountdownTime should be 0 when operation completes")
        
        self.step("5c")
        # Check MainState to ensure movement has stopped
        main_state = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.ClosureControl.Attributes.MainState)
        asserts.assert_equal(main_state, 0, "MainState should be Stopped(0) when operation completes")
        
        # STEP 6: Verify the CountdownTime behavior when an operation is interrupted
        self.step("6a")
        if stored_countdown_time is None:
            logging.info("CountdownTime returned NULL, skipping remaining steps.")
            return
        
        self.step("6b")
        # Start a movement to open position
        await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.MoveTo({"position": 1}))  # OpenInFull
            
        self.step("6c")
        # Check CountdownTime during movement
        countdown_time = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.ClosureControl.Attributes.CountdownTime)
        asserts.assert_true(countdown_time > 0, "CountdownTime should be positive during movement")
            
        self.step("6d")
        # Stop the movement
        stop_cmd_supported = False
        try:
            await self.send_single_cmd(endpoint=endpoint, cluster=cluster, command=cluster.Commands.Stop({}))
            stop_cmd_supported = True
        except Exception as e:
            logging.info(f"Stop command not supported: {e}")
        
        self.step("6e")
        # If Stop command is supported, check that CountdownTime is reset
        if stop_cmd_supported:
            countdown_time = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=Clusters.ClosureControl.Attributes.CountdownTime)
            asserts.assert_equal(countdown_time, 0, "CountdownTime should be 0 after movement is stopped")

if __name__ == "__main__":
    default_matter_test_main()