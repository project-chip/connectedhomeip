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


class TC_CLDIM_5_1(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_5_1(self) -> str:
        return "[TC-CLDIM-5.1] {C_STEP} and {C_SET_TARGET} Commands while Latched with DUT as Server"

    def steps_TC_CLDIM_5_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If MotionLatching feature is not supported, skip remaining steps"),
            TestStep("2c", "Read AttributeList attribute"),
            TestStep("2d", "Read LimitRange attribute"),
            TestStep("3a", "If manual latching is required, skip steps 3b and 3c"),
            TestStep("3b", "Send SetTarget command with Latch=True"),
            TestStep("3c", "Manually latch the device"),
            TestStep("4a", "If manual latching is not required, skip steps 4b to 4d"),
            TestStep("4b", "Send SetTarget command with Latch=True"),
            TestStep("4c", "Verify Target attribute is updated"),
            TestStep("4d", "Wait for PIXIT.CLDIM.LatchingDuration seconds"),
            TestStep(5, "Verify CurrentState attribute is updated"),
            TestStep("6a", "Send C_STEP command while latched"),
            TestStep("6b", "Send C_SET_TARGET command while latched"),
            TestStep("7a", "Send SetTarget command with Latch=False"),
            TestStep("7b", "Wait for PIXIT.CLDIM.LatchingDuration seconds"),
            TestStep("7c", "Verify CurrentState attribute is updated"),
        ]
        return steps

    def pics_TC_CLDIM_5_1(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_5_1(self):
        asserts.assert_true('PIXIT.CLDIM.LatchingDuration' in self.matter_test_config.global_test_params,
                            "PIXIT.CLDIM.LatchingDuration must be included on the command line in "
                            "the --int-arg flag as PIXIT.CLDIM.LatchingDuration:<duration>")

        latching_duration = self.matter_test_config.global_test_params['PIXIT.CLDIM.LatchingDuration']

        if latching_duration <= 0:
            asserts.fail("PIXIT.CLDIM.LatchingDuration must be greater than 0")

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

        # STEP 2b: If MotionLatching Feature is not supported, skip remaining steps
        self.step("2b")
        if not is_latching_supported:
            logging.info("MotionLatching Feature is not supported. Skipping remaining steps.")
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

        # STEP 3a: If manual latching is required, skip steps 3b and 3c
        self.step("3a")
        if self.check_pics("CLDIM.S.M.ManualLatching"):
            logging.info("Manual latching is required. Skipping steps 3b and 3c.")
            self.skip_step("3b")
            self.skip_step("3c")

        # STEP 3b: Send SetTarget command with Latch=True
        self.step("3b")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Latch=True),
                endpoint=endpoint
            )

            asserts.fail("Expected InvalidAction error, but command succeeded")

        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidAction, "Unexpected status returned")

        # STEP 3c: Manually latch the device
        self.step("3c")
        test_step = "Manually latch the device"
        self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when ready.")

        # STEP 4a: If manual latching is not required, skip steps 4b to 4d
        self.step("4a")
        if not self.check_pics("CLDIM.S.M.ManualLatching"):
            self.skip_step("4b")
            self.skip_step("4c")
            self.skip_step("4d")

        # STEP 4b: Send SetTarget command with Latch=True
        self.step("4b")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Latch=True),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 4c: Verify Target attribute is updated
        self.step("4c")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)

            if is_positioning_supported:
                asserts.assert_greater_equal(target.Position, min_position, "Target Position is outside expected range")
                asserts.assert_less_equal(target.Position, max_position, "Target Position is outside expected range")
            asserts.assert_equal(target.Latch, True, "Target Latch is not True")

            if is_speed_supported:
                asserts.assert_greater_equal(target.Speed, 0, "Target Speed is outside allowed range")
                asserts.assert_less_equal(target.Speed, 3, "Target Speed is outside allowed range")

        # STEP 4d: Wait for PIXIT.CLDIM.LatchingDuration seconds
        self.step("4d")
        time.sleep(latching_duration)

        # STEP 5: Verify CurrentState attribute is updated
        self.step(5)
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            if is_positioning_supported:
                asserts.assert_greater_equal(current_state.Position, min_position, "CurrentState Position is outside expected range")
                asserts.assert_less_equal(current_state.Position, max_position, "CurrentState Position is outside expected range")

            asserts.assert_equal(current_state.Latch, True, "CurrentState Latch is not True")

            if is_speed_supported:
                asserts.assert_greater_equal(current_state.Speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.Speed, 3, "CurrentState Speed is outside allowed range")

        # STEP 6a: Send C_STEP command while latched
        self.step("6a")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(Direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease, NumberOfSteps=1),
                endpoint=endpoint
            )

            asserts.fail("Expected InvalidAction error, but command succeeded")

        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidAction, "Unexpected status returned")

        # STEP 6b: Send C_SET_TARGET command while latched
        self.step("6b")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=max_position),
                endpoint=endpoint
            )

            asserts.fail("Expected InvalidAction error, but command succeeded")

        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidAction, "Unexpected status returned")

        # STEP 7a: Send SetTarget command with Latch=False
        self.step("7a")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Latch=False),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 7b: Wait for PIXIT.CLDIM.LatchingDuration seconds
        self.step("7b")
        time.sleep(latching_duration)

        # STEP 7c: Verify CurrentState attribute is updated
        self.step("7c")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            if is_positioning_supported:
                asserts.assert_greater_equal(current_state.Position, min_position, "CurrentState Position is outside expected range")
                asserts.assert_less_equal(current_state.Position, max_position, "CurrentState Position is outside expected range")

            asserts.assert_equal(current_state.Latch, False, "CurrentState Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(current_state.Speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.Speed, 3, "CurrentState Speed is outside allowed range")

if __name__ == "__main__":
    default_matter_test_main()
