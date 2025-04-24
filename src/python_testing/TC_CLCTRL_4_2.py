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

class TC_CLCTRL_4_2(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_4_2(self) -> str:
        return "[TC-CLCTRL-4.2] MoveTo Command Latching Functionality with DUT as Server"

    def steps_TC_CLCTRL_4_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If the LT feature is not supported, skip test case"),
            TestStep("2c", "Read AttributeList attribute"),
            TestStep("3a", "If ManualLatching is not supported, skip steps 3b and 3c"),
            TestStep("3b", "Send MoveTo command with Latch = True"),
            TestStep("3c", "Move the device manually to set OverallState.Latch to True"),
            TestStep("4a", "If ManualLatching is supported, skip steps 4b to 4d"),
            TestStep("4b", "Send MoveTo command with Latch = True"),
            TestStep("4c", "Read OverallTarget attribute"),
            TestStep("4d", "Wait for latching duration"),
            TestStep(5, "Read OverallState attribute"),
            TestStep("6a", "Send MoveTo command with Latch = False"),
            TestStep("6b", "Read OverallTarget attribute"),
            TestStep("6c", "Wait for latching duration"),
            TestStep("6d", "Read OverallState attribute"),
        ]
        return steps

    def pics_TC_CLCTRL_4_2(self) -> list[str]:
        pics = [
            "CLCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_4_2(self):
        asserts.assert_true('PIXIT.CLCTRL.FullMotionDuration' in self.matter_test_config.global_test_params,
                            "PIXIT.CLCTRL.FullMotionDuration must be included on the command line in "
                            "the --int-arg flag as PIXIT.CLCTRL.FullMotionDuration:<duration>")
        
        latching_duration = self.matter_test_config.global_test_params['PIXIT.CLCTRL.FullMotionDuration']
        is_manual_latching = True if self.check_pics('CLCTRL.S.M.ManualLatching') is not None else False

        endpoint = self.get_endpoint(default=1)

        self.step(1)
        attributes = Clusters.ClosureControl.Attributes

        self.step("2a")
        feature_map = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        
        self.step("2b")
        is_latching_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kLatch
        is_positioning_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPosition
        is_speed_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kSpeed
        
        if not is_latching_feature_supported:
            logging.info("LT (MotionLatching) feature not supported, skipping test case")
            self.skip_all_remaining_steps("2c")
            return

        self.step("2c")
        attribute_list = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        # STEP 3: Manual Latching.
        self.step("3a")
        # Check if device requires manual latching
        if not is_manual_latching:
            logging.info("Device does not require manual latching, skipping steps 3b and 3c")
            self.step("3b")
            logging.info("Test step skipped")
            self.step("3c")
            logging.info("Test step skipped")
        else:
            self.step("3b")
            try:
                cmd = Clusters.Objects.ClosureControl.Commands.MoveTo(latch=True)
                await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
                asserts.fail("Expected InvalidAction error but received Success")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidAction, f"Unexpected error returned: {e.status}")
            
            self.step("3c")
            test_step = "Manual latch the device"
            self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when ready.")

        # STEP 4: Send MoveTo command with Latch = True
        self.step("4a")
        if is_manual_latching:
            logging.info("Manual latching required, skipping steps 4b to 4d")
            self.step("4b")
            logging.info("Test step skipped")
            self.step("4c")
            logging.info("Test step skipped")
            self.step("4d")
            logging.info("Test step skipped")
        else:
            self.step("4b")
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureControl.Commands.MoveTo(Latch=True),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            
            self.step("4c")
            if attributes.OverallTarget.attribute_id in attribute_list:
                overall_target = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallTarget)
            else:
                asserts.assert_true(False, "OverallTarget attribute not supported")
            
            # Verify OverallTarget fields
            if is_positioning_supported:
                asserts.assert_true(hasattr(overall_target, "position"), "OverallTarget.Position field missing")
                asserts.assert_is_not_none(overall_target.position, "OverallTarget.Position is None")
                asserts.assert_greater_equal(overall_target.position, 0, "OverallTarget.Position is outside allowed range")
                asserts.assert_less_equal(overall_target.position, 4, "OverallTarget.Position is outside allowed range")
                
            asserts.assert_true(hasattr(overall_target, "latch"), "OverallTarget.Latch field missing")
            asserts.assert_true(overall_target.latch, "OverallTarget.Latch is not True")
            
            if is_speed_supported:
                asserts.assert_true(hasattr(overall_target, "speed"), "OverallTarget.Speed field missing")
                asserts.assert_is_not_none(overall_target.speed, "OverallTarget.Speed is None")
                asserts.assert_greater_equal(overall_target.speed, 0, "OverallTarget.Speed is outside allowed range")
                asserts.assert_less_equal(overall_target.speed, 3, "OverallTarget.Speed is outside allowed range")
            
            self.step("4d")
            logging.info(f"Waiting for {latching_duration} seconds to complete latching")
            time.sleep(latching_duration)

        # STEP 5: Verify that OverallState.Latch is True
        self.step(5)
        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallState)
        else:
            asserts.assert_true(False, "OverallState attribute not supported")

        if is_positioning_supported:
            asserts.assert_true(hasattr(overall_state, "positioning"), "OverallState.Positioning field missing")
            asserts.assert_is_not_none(overall_state.positioning, "OverallState.Positioning is None")
            asserts.assert_greater_equal(overall_state.positioning, 0, "OverallState.Positioning is outside allowed range")
            asserts.assert_less_equal(overall_state.positioning, 5, "OverallState.Positioning is outside allowed range")

        # Verify OverallState fields
        asserts.assert_true(hasattr(overall_state, "latch"), "OverallState.Latch field missing")
        asserts.assert_true(overall_state.latch, "OverallState.Latch is not True")
        
        if is_speed_supported:
            asserts.assert_true(hasattr(overall_state, "speed"), "OverallState.Speed field missing")
            asserts.assert_is_not_none(overall_state.speed, "OverallState.Speed is None")
            asserts.assert_greater_equal(overall_state.speed, 0, "OverallState.Speed is outside allowed range")
            asserts.assert_less_equal(overall_state.speed, 3, "OverallState.Speed is outside allowed range")
        
        # Check SecureState if it exists
        if hasattr(overall_state, "secureState"):
            asserts.assert_is_not_none(overall_state.secureState, "OverallState.SecureState is not available")

        # STEP 6: Unlatch the device
        self.step("6a")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureControl.Commands.MoveTo(Latch=False),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step("6b")
        if attributes.OverallTarget.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallTarget)
        else:
            asserts.assert_true(False, "OverallTarget attribute not supported")
        
        # Verify OverallTarget fields after unlatching        
        if is_positioning_supported:
            asserts.assert_true(hasattr(overall_target, "position"), "OverallTarget.Position field missing")
            asserts.assert_is_not_none(overall_target.position, "OverallTarget.Position is None")
            asserts.assert_greater_equal(overall_target.position, 0, "OverallTarget.Position is outside allowed range")
            asserts.assert_less_equal(overall_target.position, 5, "OverallTarget.Position is outside allowed range")
        
        asserts.assert_true(hasattr(overall_target, "latch"), "OverallTarget.Latch field missing")
        asserts.assert_false(overall_target.latch, "OverallTarget.Latch is not False")
        
        is_speed_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kSP
        if is_speed_supported:
            asserts.assert_true(hasattr(overall_target, "speed"), "OverallTarget.Speed field missing")
            asserts.assert_is_not_none(overall_target.speed, "OverallTarget.Speed is None")
            asserts.assert_greater_equal(overall_target.speed, 0, "OverallTarget.Speed is outside allowed range")
            asserts.assert_less_equal(overall_target.speed, 3, "OverallTarget.Speed is outside allowed range")
        
        self.step("6c")
        logging.info(f"Waiting for {latching_duration} seconds to complete unlatching")
        time.sleep(latching_duration)
        
        self.step("6d")
        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallState)
        else:
            asserts.assert_true(False, "OverallState attribute not supported")
        
        # Verify OverallState fields after unlatching        
        if is_positioning_supported:
            asserts.assert_true(hasattr(overall_state, "positioning"), "OverallState.Positioning field missing")
            asserts.assert_is_not_none(overall_state.positioning, "OverallState.Positioning is None")
            asserts.assert_greater_equal(overall_state.positioning, 0, "OverallState.Positioning is outside allowed range")
            asserts.assert_less_equal(overall_state.positioning, 5, "OverallState.Positioning is outside allowed range")
        
        asserts.assert_true(hasattr(overall_state, "latch"), "OverallState.Latch field missing")
        asserts.assert_false(overall_state.latch, "OverallState.Latch is not False")
        
        if is_speed_supported:
            asserts.assert_true(hasattr(overall_state, "speed"), "OverallState.Speed field missing")
            asserts.assert_is_not_none(overall_state.speed, "OverallState.Speed is None")
            asserts.assert_greater_equal(overall_state.speed, 0, "OverallState.Speed is outside allowed range")
            asserts.assert_less_equal(overall_state.speed, 3, "OverallState.Speed is outside allowed range")
        
        # Check SecureState if it exists
        if hasattr(overall_state, "secureState"):
            asserts.assert_is_not_none(overall_state.secureState, "OverallState.SecureState is not available")

if __name__ == "__main__":
    default_matter_test_main()