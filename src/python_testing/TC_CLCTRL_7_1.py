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
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
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
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

class TC_CLCTRL_7_1(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_7_1(self) -> str:
        return "[TC-CLCTRL-7.1] MoveTo Position while Latched with DUT as Server"

    def steps_TC_CLCTRL_7_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If LT or PS feature not supported, skip test case"),
            TestStep("2c", "Read AttributeList attribute"),
            TestStep("3a", "Send MoveTo command with Position = CloseInFull"),
            TestStep("3b", "Wait for full motion duration"),
            TestStep("3c", "Read OverallState attribute"),
            TestStep("4a", "If manual latching not required, skip steps 4b and 4c"),
            TestStep("4b", "Send MoveTo command with Latch = True (expect InvalidAction)"),
            TestStep("4c", "Manually latch the device"),
            TestStep("5a", "If manual latching required, skip steps 5b to 5d"),
            TestStep("5b", "Send MoveTo command with Latch = True"),
            TestStep("5c", "Read OverallTarget attribute"),
            TestStep("5d", "Wait for latching duration"),
            TestStep(6, "Read OverallState attribute to verify latch status"),
            TestStep(7, "Send MoveTo command with Position = OpenInFull (expect InvalidAction)"),
            TestStep("8a", "Send MoveTo command with Latch = False"),
            TestStep("8b", "Wait for latching duration"),
            TestStep("8c", "Read OverallState attribute to verify unlatch"),
        ]
        return steps

    def pics_TC_CLCTRL_7_1(self) -> list[str]:
        pics = [
            "CLCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_7_1(self):
        
        asserts.assert_true('PIXIT.CLCTRL.FullMotionDuration' in self.matter_test_config.global_test_params,
                    "PIXIT.CLCTRL.FullMotionDuration must be included on the command line in "
                    "the --int-arg flag as PIXIT.CLCTRL.FullMotionDuration:<duration>")
        asserts.assert_true('PIXIT.CLCTRL.LatchingDuration' in self.matter_test_config.global_test_params,
                    "PIXIT.CLCTRL.LatchingDuration must be included on the command line in "
                    "the --int-arg flag as PIXIT.CLCTRL.LatchingDuration:<duration>")
        
        full_motion_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.FullMotionDuration']
        latching_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.LatchingDuration']
        
        is_manual_latching = True if self.check_pics('CLCTRL.S.M.ManualLatching') is not None else False
        
        endpoint = self.get_endpoint(default=1)
        
        # STEP 1: Commissioning
        self.step(1)
        
        # STEP 2
        self.step("2a")
        attributes = Clusters.ClosureControl.Attributes
        feature_map = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        self.step("2b")
        is_latching_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching
        is_positioning_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPositioning
        is_speed_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kSpeed
        
        if not is_latching_feature_supported or not is_positioning_supported:
            logging.info("LT (MotionLatching) or PS (Positioning) feature not supported, skipping test case")
            self.skip_all_remaining_steps("2c")
            return

        self.step("2c")
        attribute_list = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        # STEP 3
        self.step("3a")
        cmd = Clusters.Objects.ClosureControl.Commands.MoveTo(position=Clusters.ClosureControl.Enums.TargetPositionEnum.kCloseInFull)
        await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
        
        self.step("3b")
        logging.info(f"Waiting for {full_motion_duration} seconds to complete movement")
        time.sleep(full_motion_duration)
        
        self.step("3c")
        overall_state = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallState)
        
        # Verify device is fully closed
        asserts.assert_true(hasattr(overall_state, "positioning"), "OverallState.Positioning field missing")
        asserts.assert_equal(overall_state.positioning, Clusters.ClosureControl.Enums.PositioningEnum.kFullyClosed, "OverallState.Positioning is not FullyClosed(0)")
        
        asserts.assert_true(hasattr(overall_state, "latch"), "OverallState.Latch field missing")
        asserts.assert_false(overall_state.latch, "OverallState.Latch is not False")
        
        if is_speed_supported:
            asserts.assert_greater_equal(overall_state.speed, Clusters.Globals.ThreeLevelAutoEnum.kAuto, "OverallState.Speed is not greater than 0")
            asserts.assert_less_equal(overall_state.speed, Clusters.Globals.ThreeLevelAutoEnum.kHigh, "OverallState.Speed is not in range [0, 3]")
        
        # Check SecureState if it exists
        if hasattr(overall_state, "secureState"):
            asserts.assert_false(overall_state.secureState, "OverallState.SecureState is not False")

        # STEP 4
        self.step("4a")
        if not is_manual_latching:
            logging.info("Manual latching not required, skipping steps 4b and 4c")
            self.step("4b")
            logging.info("Test step skipped")
            self.step("4c")
            logging.info("Test step skipped")
        else:
            self.step("4b")
            try:
                cmd = Clusters.Objects.ClosureControl.Commands.MoveTo(latch=True)
                await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
                asserts.fail("Expected InvalidAction error but received Success")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidAction, f"Unexpected error returned: {e.status}")
            
            self.step("4c")
            test_step = "Manual latch the device"
            self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when ready.")

        # STEP 5
        self.step("5a")
        if is_manual_latching:
            logging.info("Manual latching required, skipping steps 5b to 5d")
            self.step("5b")
            logging.info("Test step skipped")
            self.step("5c")
            logging.info("Test step skipped")
            self.step("5d")
            logging.info("Test step skipped")
        else:
            self.step("5b")
            cmd = Clusters.Objects.ClosureControl.Commands.MoveTo(latch=True)
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            
            self.step("5c")
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallTarget)
            
            # Verify OverallTarget fields
            asserts.assert_true(hasattr(overall_target, "latch"), "OverallTarget.Latch field missing")
            asserts.assert_true(overall_target.latch, "OverallTarget.Latch is not True")
            
            asserts.assert_equal(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kCloseInFull, "OverallTarget.Position is not CloseInFull(0)")
            
            if is_speed_supported:
                asserts.assert_true(Clusters.Globals.ThreeLevelAutoEnum.kAuto <= overall_target.speed <= Clusters.Globals.ThreeLevelAutoEnum.kHigh, f"OverallTarget.Speed out of range: {overall_target.speed}")
            
            self.step("5d")
            logging.info(f"Waiting for {latching_duration} seconds to complete latching")
            time.sleep(latching_duration)

        # STEP 6
        self.step(6)
        overall_state = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallState)
        
        # Verify OverallState fields
        asserts.assert_equal(overall_state.positioning, Clusters.ClosureControl.Enums.PositioningEnum.kFullyClosed, "OverallState.Positioning is not FullyClosed(0)")
        asserts.assert_true(overall_state.latch, "OverallState.Latch is not True")
        
        if is_speed_supported:
            asserts.assert_greater_equal(overall_state.speed, Clusters.Globals.ThreeLevelAutoEnum.kAuto, "OverallState.Speed is not greater than 0")
            asserts.assert_less_equal(overall_state.speed, Clusters.Globals.ThreeLevelAutoEnum.kHigh, "OverallState.Speed is not in range [0, 3]")    
    
        if hasattr(overall_state, "secureState"):
            asserts.assert_true(overall_state.secureState, "OverallState.SecureState is not True")

        # STEP 7
        self.step(7)
        try:
            cmd = Clusters.Objects.ClosureControl.Commands.MoveTo(position=Clusters.ClosureControl.Enums.TargetPositionEnum.kOpenInFull)
            await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            asserts.fail("Expected InvalidAction error but received Success")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidAction, f"Unexpected error returned: {e.status}")

        # STEP 8
        self.step("8a")
        cmd = Clusters.Objects.ClosureControl.Commands.MoveTo(latch=False)
        await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
        
        self.step("8b")
        logging.info(f"Waiting for {latching_duration} seconds to complete unlatching")
        time.sleep(latching_duration)
        
        self.step("8c")
        overall_state = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallState)
        
        # Verify OverallState fields after unlatching
        asserts.assert_equal(overall_state.positioning, Clusters.ClosureControl.Enums.PositioningEnum.kFullyClosed, "OverallState.Positioning is not FullyClosed(0)")
        asserts.assert_false(overall_state.latch, "OverallState.Latch is not False")
        
        if is_speed_supported:
            asserts.assert_greater_equal(overall_state.speed, Clusters.Globals.ThreeLevelAutoEnum.kAuto, "OverallState.Speed is not greater than 0")
            asserts.assert_less_equal(overall_state.speed, Clusters.Globals.ThreeLevelAutoEnum.kHigh, "OverallState.Speed is not in range [0, 3]") 
        
        if hasattr(overall_state, "secureState"):
            asserts.assert_false(overall_state.secureState, "OverallState.SecureState is not False")

if __name__ == "__main__":
    default_matter_test_main()