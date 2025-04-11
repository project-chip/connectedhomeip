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

import logging
import time

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CLDIM_4_1(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_4_1(self) -> str:
        return "[TC-CLDIM-4.1] {C_STEP} Command Primary Functionality with DUT as Server"

    def steps_TC_CLDIM_4_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If Positioning feature is not supported, skip remaining steps"),
            TestStep("2c", "Read AttributeList attribute"),
            TestStep("2d", "Read StepValue attribute"),
            TestStep("2e", "Read LimitRange attribute"),
            TestStep("3a", "Send Step command to increase position to MaxPosition"),
            TestStep("3b", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("3c", "Verify CurrentState attribute is at MaxPosition"),
            TestStep("4a", "Send Step command to decrease position by 2 steps"),
            TestStep("4b", "Verify Target attribute is updated"),
            TestStep("4c", "Wait for PIXIT.CLDIM.StepMotionDuration * 2 seconds"),
            TestStep("4d", "Verify CurrentState attribute is updated"),
            TestStep("4e", "Send Step command to increase position by 2 steps"),
            TestStep("4f", "Verify Target attribute is updated"),
            TestStep("4g", "Wait for PIXIT.CLDIM.StepMotionDuration * 2 seconds"),
            TestStep("4h", "Verify CurrentState attribute is updated"),
            TestStep("5a", "If Speed Feature is not supported, skip step 5b to 5e"),
            TestStep("5b", "Send Step command to decrease position by 1 step with Speed=High"),
            TestStep("5c", "Verify Target attribute is updated"),
            TestStep("5d", "Wait for PIXIT.CLDIM.StepMotionDuration seconds"),
            TestStep("5e", "Verify CurrentState attribute is updated"),
            TestStep("6a", "If Speed Feature is not supported, skip step 6b to 6e"),
            TestStep("6b", "Send Step command to increase position by 1 step with Speed=Auto"),
            TestStep("6c", "Verify Target attribute is updated"),
            TestStep("6d", "Wait for PIXIT.CLDIM.StepMotionDuration seconds"),
            TestStep("6e", "Verify CurrentState attribute is updated"),
            TestStep("7a", "Send Step command to decrease position by 1 step"),
            TestStep("7b", "Send Step command to decrease position by 1 step"),
            TestStep("7c", "Send Step command to decrease position by 1 step"),
            TestStep("7d", "Verify Target attribute is updated"),
            TestStep("7e", "Wait for PIXIT.CLDIM.StepMotionDuration * 3 seconds"),
            TestStep("7f", "Verify CurrentState attribute is updated"),
            TestStep("8a", "Send Step command to decrease position by 65535"),
            TestStep("8b", "Verify Target attribute is at MinPosition"),
            TestStep("8c", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("8d", "Verify CurrentState attribute is at MinPosition"),
            TestStep("8e", "Send Step command to increase position by 65535"),
            TestStep("8f", "Verify Target attribute is at MaxPosition"),
            TestStep("8g", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("8h", "Verify CurrentState attribute is at MaxPosition"),
        ]
        return steps

    def pics_TC_CLDIM_4_1(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_4_1(self):
        asserts.assert_true('PIXIT.CLDIM.FullMotionDuration' in self.matter_test_config.global_test_params,
                            "PIXIT.CLDIM.FullMotionDuration must be included on the command line in "
                            "the --int-arg flag as PIXIT.CLDIM.FullMotionDuration:<duration>")
        asserts.assert_true('PIXIT.CLDIM.StepMotionDuration' in self.matter_test_config.global_test_params,
                            "PIXIT.CLDIM.StepMotionDuration must be included on the command line in "
                            "the --int-arg flag as PIXIT.CLDIM.StepMotionDuration:<duration>")

        full_motion_duration = self.matter_test_config.global_test_params['PIXIT.CLDIM.FullMotionDuration']
        step_motion_duration = self.matter_test_config.global_test_params['PIXIT.CLDIM.StepMotionDuration']

        if full_motion_duration <= 0:
            asserts.fail("PIXIT.CLDIM.LatchingDuration must be greater than 0")

        if step_motion_duration <= 0:
            asserts.fail("PIXIT.CLDIM.LatchingDuration must be greater than 0")

        endpoint = self.get_endpoint(default=1)

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureDimension.Attributes

        # Default values
        min_position = 0
        max_position = 10000

        # STEP 2a: Read FeatureMap attribute
        self.step("2a")
        feature_map = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_positioning_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
        is_latching_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kMotionLatching
        is_speed_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kSpeed

        # STEP 2b: If Positioning feature is not supported, skip remaining steps
        self.step("2b")
        if not is_positioning_supported:
            logging.info("Positioning feature is not supported. Skipping remaining steps.")
            self.skip_all_remaining_steps("2c")
            return

        # STEP 2c: Read AttributeList attribute
        self.step("2c")
        attribute_list = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        # STEP 2d: Read StepValue attribute
        self.step("2d")
        step_value = 1  # Default step value
        if attributes.StepValue.attribute_id in attribute_list:
            step_value = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.StepValue)

        # STEP 2e: Read LimitRange attribute
        self.step("2e")
        if attributes.LimitRange.attribute_id in attribute_list:
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            min_position = limit_range.Min
            max_position = limit_range.Max

        # STEP 3a: Send Step command to increase position to MaxPosition
        self.step("3a")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease, NumberOfSteps=65535),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 3b: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("3b")
        time.sleep(full_motion_duration)

        # STEP 3c: Verify CurrentState attribute is at MaxPosition
        self.step("3c")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            asserts.assert_equal(current_state.Position, max_position, "CurrentState Position is not at MaxPosition")

            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(current_state.Speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.Speed, 3, "CurrentState Speed is outside allowed range")

        # STEP 4a: Send Step command to decrease position by 2 steps
        self.step("4a")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease, NumberOfSteps=2),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 4b: Verify Target attribute is updated
        self.step("4b")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
            expected_position = max(max_position - 2 * step_value, min_position)
            asserts.assert_equal(target.Position, expected_position, "Target Position is not updated correctly")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(target.Speed, 0, "Target Speed is outside allowed range")
                asserts.assert_less_equal(target.Speed, 3, "Target Speed is outside allowed range")

        # STEP 4c: Wait for PIXIT.CLDIM.StepMotionDuration * 2 seconds
        self.step("4c")
        time.sleep(step_motion_duration * 2)

        # STEP 4d: Verify CurrentState attribute is updated
        self.step("4d")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
            expected_position = max(max_position - 2 * step_value, min_position)
            asserts.assert_equal(current_state.Position, expected_position, "CurrentState Position is not updated correctly")

            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(current_state.Speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.Speed, 3, "CurrentState Speed is outside allowed range")
        
        # STEP 4e: Send Step command to increase position by 2 steps
        self.step("4e")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease, NumberOfSteps=2),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 4f: Verify Target attribute is updated
        self.step("4f")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
            asserts.assert_equal(target.Position, max_position, "Target Position is not updated correctly")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(target.Speed, 0, "Target Speed is outside allowed range")
                asserts.assert_less_equal(target.Speed, 3, "Target Speed is outside allowed range")

        # STEP 4g: Wait for PIXIT.CLDIM.StepMotionDuration * 2 seconds
        self.step("4g")
        time.sleep(step_motion_duration * 2)

        # STEP 4h: Verify CurrentState attribute is updated
        self.step("4h")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
            asserts.assert_equal(current_state.Position, max_position, "CurrentState Position is not updated correctly")

            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(current_state.Speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.Speed, 3, "CurrentState Speed is outside allowed range")

        # STEP 5a: If Speed Feature is not supported, skip step 5b to 5e
        self.step("5a")
        if not is_speed_supported:
            logging.info("Speed feature is not supported. Skipping steps 5b to 5e.")
            self.skip_step("5b")
            self.skip_step("5c")
            self.skip_step("5d")
            self.skip_step("5e")

        # STEP 5b: Send Step command to decrease position by 1 step with Speed=High
        self.step("5b")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                    NumberOfSteps=1,
                    Speed=Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kHigh
                ),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 5c: Verify Target attribute is updated
        self.step("5c")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
            expected_position = max_position - step_value
            asserts.assert_equal(target.Position, expected_position, "Target Position is not updated correctly")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")

            asserts.assert_equal(target.Speed, 3, "Target Speed is not High")

        # STEP 5d: Wait for PIXIT.CLDIM.StepMotionDuration seconds
        self.step("5d")
        time.sleep(step_motion_duration)

        # STEP 5e: Verify CurrentState attribute is updated
        self.step("5e")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
            expected_position = max_position - step_value
            asserts.assert_equal(current_state.Position, expected_position, "CurrentState Position is not updated correctly")

            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")

            asserts.assert_equal(current_state.Speed, 3, "CurrentState Speed is not High")

        # STEP 6a: If Speed Feature is not supported, skip step 6b to 6e
        self.step("6a")
        if not is_speed_supported:
            logging.info("Speed feature is not supported. Skipping steps 6b to 6e.")
            self.skip_step("6b")
            self.skip_step("6c")
            self.skip_step("6d")
            self.skip_step("6e")

        # STEP 6b: Send Step command to increase position by 1 step with Speed=Auto
        self.step("6b")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease,
                    NumberOfSteps=1,
                    Speed=Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kAuto
                ),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 6c: Verify Target attribute is updated
        self.step("6c")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
            expected_position = max_position
            asserts.assert_equal(target.Position, expected_position, "Target Position is not updated correctly")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")

            asserts.assert_equal(target.Speed, 0, "Target Speed is not Auto")

        # STEP 6d: Wait for PIXIT.CLDIM.StepMotionDuration seconds
        self.step("6d")
        time.sleep(step_motion_duration)

        # STEP 6e: Verify CurrentState attribute is updated
        self.step("6e")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
            expected_position = max_position
            asserts.assert_equal(current_state.Position, expected_position, "CurrentState Position is not updated correctly")

            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")

            asserts.assert_equal(current_state.Speed, 0, "CurrentState Speed is not Auto")

        # STEP 7a: Send Step command to decrease position by 1 step
        self.step("7a")

        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                    NumberOfSteps=1
                ),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 7b: Send Step command to decrease position by 1 step
        self.step("7b")

        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                    NumberOfSteps=1
                ),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 7c: Send Step command to decrease position by 1 step
        self.step("7c")

        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                    NumberOfSteps=1
                ),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 7d: Verify Target attribute is updated after multiple steps
        self.step("7d")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
            expected_position = max(max_position - 3 * step_value, min_position)
            asserts.assert_equal(target.Position, expected_position, "Target Position is not updated correctly")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(target.Speed, 0, "Target Speed is outside allowed range")
                asserts.assert_less_equal(target.Speed, 3, "Target Speed is outside allowed range")

        # STEP 7e: Wait for PIXIT.CLDIM.StepMotionDuration * 3 seconds
        self.step("7e")
        time.sleep(step_motion_duration * 3)

        # STEP 7f: Verify CurrentState attribute is updated after multiple steps
        self.step("7f")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
            expected_position = max(max_position - 3 * step_value, min_position)
            asserts.assert_equal(current_state.Position, expected_position, "CurrentState Position is not updated correctly")

            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(current_state.Speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.Speed, 3, "CurrentState Speed is outside allowed range")

        # STEP 8a: Send Step command to decrease position beyond MinPosition
        self.step("8a")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                    NumberOfSteps=65535
                ),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 8b: Verify Target attribute is at MinPosition
        self.step("8b")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
            asserts.assert_equal(target.Position, min_position, "Target Position is not at MinPosition")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(target.Speed, 0, "Target Speed is outside allowed range")
                asserts.assert_less_equal(target.Speed, 3, "Target Speed is outside allowed range")

        # STEP 8c: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("8c")
        time.sleep(full_motion_duration)

        # STEP 8d: Verify CurrentState attribute is at MinPosition
        self.step("8d")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
            asserts.assert_equal(current_state.Position, min_position, "CurrentState Position is not at MinPosition")

            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(current_state.Speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.Speed, 3, "CurrentState Speed is outside allowed range")

        # STEP 8e: Send Step command to increase position beyond MaxPosition
        self.step("8e")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease,
                    NumberOfSteps=65535
                ),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 8f: Verify Target attribute is at MaxPosition
        self.step("8f")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
            asserts.assert_equal(target.Position, max_position, "Target Position is not at MaxPosition")

            if is_latching_supported:
                asserts.assert_equal(target.Latch, False, "Target Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(target.Speed, 0, "Target Speed is outside allowed range")
                asserts.assert_less_equal(target.Speed, 3, "Target Speed is outside allowed range")

        # STEP 8g: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("8g")
        time.sleep(full_motion_duration)

        # STEP 8h: Verify CurrentState attribute is at MaxPosition
        self.step("8h")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
            asserts.assert_equal(current_state.Position, max_position, "CurrentState Position is not at MaxPosition")

            if is_latching_supported:
                asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(current_state.Speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.Speed, 3, "CurrentState Speed is outside allowed range")


if __name__ == "__main__":
    default_matter_test_main()
