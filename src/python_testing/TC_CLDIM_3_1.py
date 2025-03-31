#
#    Copyright (c) 2022 Project CHIP Authors
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
#    run1:
#      app: ${TYPE_OF_APP}
#      app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#      script-args: >
#        --storage-path admin_storage.json
#        --commissioning-method on-network
#        --discriminator 1234
#        --passcode 20202021
#        --trace-to json:${TRACE_TEST_JSON}.json
#        --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#      factory-reset: true
#      quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import time

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CLDIM_3_1(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_3_1(self) -> str:
        return "[TC-CLDIM-3.1] {C_SET_TARGET} Command Positioning Functionality with DUT as Server"

    def steps_TC_CLDIM_3_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If Positioning feature is not supported, skip remaining steps"),
            TestStep("2c", "Read AttributeList attribute"),
            TestStep("2d", "Read LimitRange attribute"),
            TestStep("3a", "Set Position to MaxPosition"),
            TestStep("3b", "Verify Target attribute is updated"),
            TestStep("3c", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("3d", "Verify CurrentState attribute is updated"),
            TestStep("4a", "If Speed feature is not supported, skip step 4b to 4e"),
            TestStep("4b", "Set Speed to Medium"),
            TestStep("4c", "Verify Target attribute is updated"),
            TestStep("4d", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("4e", "Verify CurrentState attribute is updated"),
            TestStep("5a", "Set Position to min_position"),
            TestStep("5b", "Verify Target attribute is updated"),
            TestStep("5c", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("5d", "Verify CurrentState attribute is updated"),
            TestStep("6a", "If Speed feature is not supported, skip step 6b to 6e"),
            TestStep("6b", "Set Position to MaxPosition and Speed to High"),
            TestStep("6c", "Verify Target attribute is updated"),
            TestStep("6d", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("6e", "Verify CurrentState attribute is updated"),
        ]
        return steps

    def pics_TC_CLDIM_3_1(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_3_1(self):
        asserts.assert_true('PIXIT.CLDIM.FullMotionDuration' in self.matter_test_config.global_test_params,
                            "PIXIT.CLDIM.FullMotionDuration must be included on the command line in "
                            "the --int-arg flag as PIXIT.CLDIM.FullMotionDuration:<duration>")

        full_motion_duration = self.matter_test_config.global_test_params['PIXIT.CLDIM.FullMotionDuration']

        if(full_motion_duration <= 0):
            asserts.fail("PIXIT.CLDIM.FullMotionDuration must be greater than 0")
        
        endpoint = self.get_endpoint(default=1)

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureDimension.Attributes

        # Default values
        min_position = 0
        max_position = 10000  

        # STEP 2a: Read feature map and determine supported features
        self.step("2a")
        feature_map = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        
        is_positioning_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
        is_latching_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kMotionLatching
        is_speed_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kSpeed

        # STEP 2b: If Positioning Feature is not supported, skip remaining steps
        self.step("2b")
        if not is_positioning_supported:
            logging.info("Positioning Feature is not supported. Skipping remaining steps.")
            self.skip_all_remaining_steps("2c")
            return

        # STEP 2c: Read AttributeList attribute
        self.step("2c")
        attribute_list = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        # STEP 2d: Read LimitRange attribute if supported
        self.step("2d")
        if attributes.LimitRange.attribute_id in attribute_list:
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            min_position = limit_range.Min
            max_position = limit_range.Max

        # STEP 3a: Set Position to MaxPosition
        self.step("3a")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=max_position),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 3b: Verify Target attribute is updated
        self.step("3b")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)

            asserts.assert_equal(target.Position, max_position, "Target Position does not match MaxPosition")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")
            
            if is_speed_supported:
                asserts.assert_greater_equal(target.Speed, 0, "Target Speed is outside allowed range")
                asserts.assert_less_equal(target.Speed, 3, "Target Speed is outside allowed range")
        else:
            logging.info("Target attribute is not supported. Skipping step 3b.")

        # STEP 3c: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("3c")
        time.sleep(full_motion_duration)

        # STEP 3d: Verify CurrentState attribute is updated
        self.step("3d")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            asserts.assert_equal(current_state.Position, max_position, "CurrentState Position does not match MaxPosition")

            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")
            
            if is_speed_supported:
                asserts.assert_greater_equal(current_state.Speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.Speed, 3, "CurrentState Speed is outside allowed range")
        else:
            logging.info("CurrentState attribute is not supported. Skipping step 3d.")

        # STEP 4a: Set Speed to Medium if supported
        self.step("4a")
        if not is_speed_supported:
            logging.info("Speed feature is not supported. Skipping steps 4b to 4e.")
            self.skip_step("4b")
            self.skip_step("4c")
            self.skip_step("4d")
            self.skip_step("4e")

        # STEP 4b: Set Speed to Medium
        self.step("4b")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=max_position, Speed=Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kMedium),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 4c: Verify Target attribute is updated
        self.step("4c")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)

            asserts.assert_equal(target.Position, max_position, "Target Position does not match MaxPosition")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")
                
            asserts.assert_equal(target.Speed, Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kMedium, "Target Speed does not match Medium")
        else:
            logging.info("Target attribute is not supported. Skipping step 4c.")

        # STEP 4d: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("4d")
        time.sleep(full_motion_duration)

        # STEP 4e: Verify CurrentState attribute is updated
        self.step("4e")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            asserts.assert_equal(current_state.Position, max_position, "CurrentState Position does not match MaxPosition")
            
            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")
            
            asserts.assert_equal(current_state.Speed, Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kMedium, "CurrentState Speed does not match Medium")
        else:
            logging.info("CurrentState attribute is not supported. Skipping step 4e.")

        # STEP 5a: Set Position to min_position
        self.step("5a")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=min_position),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 5b: Verify Target attribute is updated
        self.step("5b")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)

            asserts.assert_equal(target.Position, min_position, "Target Position does not match min_position")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")

            if is_speed_supported:
                asserts.assert_equal(target.Speed, Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kMedium, "Target Speed does not match Medium")
        else:
            logging.info("Target attribute is not supported. Skipping step 5b.")

        # STEP 5c: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("5c")
        time.sleep(full_motion_duration)

        # STEP 5d: Verify CurrentState attribute is updated
        self.step("5d")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            asserts.assert_equal(current_state.Position, min_position, "CurrentState Position does not match min_position")

            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")
            
            if is_speed_supported:
                asserts.assert_equal(current_state.Speed, Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kMedium, "CurrentState Speed does not match Medium")
        else:
            logging.info("CurrentState attribute is not supported. Skipping step 5d.")

        # STEP 6a: Set Position to MaxPosition and Speed to High if supported
        self.step("6a")
        if not is_speed_supported:
            logging.info("Speed feature is not supported. Skipping steps 6b to 6e.")
            self.skip_step("6b")
            self.skip_step("6c")
            self.skip_step("6d")
            self.skip_step("6e")

        # STEP 6b: Set Position to MaxPosition and Speed to High
        self.step("6b")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=max_position, Speed=Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kHigh),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 6c: Verify Target attribute is updated
        self.step("6c")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
            
            asserts.assert_equal(target.Position, max_position, "Target Position does not match MaxPosition")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")
            
            asserts.assert_equal(target.Speed, Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kHigh, "Target Speed does not match High")
        else:
            logging.info("Target attribute is not supported. Skipping step 6c.")

        # STEP 6d: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("6d")
        time.sleep(full_motion_duration)

        # STEP 6e: Verify CurrentState attribute is updated
        self.step("6e")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            asserts.assert_equal(current_state.Position, max_position, "CurrentState Position does not match MaxPosition")
            
            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")

            asserts.assert_equal(current_state.Speed, Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kHigh, "CurrentState Speed does not match High")
        else:
            logging.info("CurrentState attribute is not supported. Skipping step 6e.")


if __name__ == "__main__":
    default_matter_test_main()
