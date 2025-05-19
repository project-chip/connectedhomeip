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
#      app: ${ALL_CLUSTERS_APP}
#      app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#      script-args: >
#        --storage-path admin_storage.json
#        --commissioning-method on-network
#        --discriminator 1234
#        --passcode 20202021
#        --int-arg PIXIT.CLDIM.LatchingDuration:1
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


class TC_CLDIM_3_2(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_3_2(self) -> str:
        return "[TC-CLDIM-3.2] SetTarget Command Latching Functionality with DUT as Server"

    def steps_TC_CLDIM_3_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If MotionLatching feature is not supported, skip remaining steps"),
            TestStep("2c", "Read AttributeList attribute"),
            TestStep("2d", "Read LimitRange attribute"),
            TestStep("2e", "Read LatchControlModes attribute"),
            TestStep("3a", "If manual latching is required, skip steps 3b and 3c"),
            TestStep("3b", "Send SetTarget command with Latch=True"),
            TestStep("3c", "Manually latch the device"),
            TestStep("4a", "If manual latching is not required, skip steps 4b to 4d"),
            TestStep("4b", "Send SetTarget command with Latch=True"),
            TestStep("4c", "Verify Target attribute is updated"),
            TestStep("4d", "Wait for PIXIT.CLDIM.LatchingDuration seconds"),
            TestStep(5, "Verify CurrentState attribute is updated"),
            TestStep("6a", "Send Step command while device is latched"),
            TestStep("6b", "Send SetTarget command while device is latched"),
            TestStep("7a", "If manual latching is required, unlatch device manually"),
            TestStep("7b", "If manual latching is required, skip steps 7c to 7e"),
            TestStep("7c", "Send SetTarget command with Latch=False"),
            TestStep("7d", "Verify Target attribute is updated"),
            TestStep("7e", "Wait for PIXIT.CLDIM.LatchingDuration seconds"),
            TestStep("7f", "Verify CurrentState attribute is updated"),
        ]
        return steps

    def pics_TC_CLDIM_3_2(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_3_2(self):
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
            min_position = limit_range.min
            max_position = limit_range.max

        # STEP 2e: Read LatchControlModes attribute
        self.step("2e")
        latch_control_modes = 0b0  # Default value as a bitmap
        if attributes.LatchControlModes.attribute_id in attribute_list:
            latch_control_modes = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)

        # STEP 3a: If manual latching is required, skip steps 3b and 3c
        self.step("3a")
        if latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlMode.kRemoteLatching:
            logging.info("Manual latching is required. Skipping steps 3b and 3c.")
            self.skip_step("3b")
            self.skip_step("3c")
        else:
            # STEP 3b: Send SetTarget command with Latch=True
            self.step("3b")
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=True),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Unexpected status returned")

            # STEP 3c: Manually latch the device
            self.step("3c")
            test_step = "Manual latch the device"
            self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when ready.")

        # STEP 4a: If manual latching is not required, skip steps 4b to 4d
        self.step("4a")
        if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlMode.kRemoteLatching:
            self.skip("4b")
            self.skip("4c")
            self.skip("4d")
        else:
            # STEP 4b: Send SetTarget command with Latch=True
            self.step("4b")
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=True),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 4c: Verify Target attribute is updated
            self.step("4c")
            if attributes.Target.attribute_id in attribute_list:
                target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)

                asserts.assert_equal(target.latch, True, "Target Latch is not True")
            else:
                logging.info("Target attribute is not supported. Skipping step 4c.")
                self.mark_current_step_skipped()

            # STEP 4d: Wait for PIXIT.CLDIM.LatchingDuration seconds
            self.step("4d")
            time.sleep(latching_duration)

        # STEP 5: Verify CurrentState attribute is updated
        self.step(5)
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            if is_positioning_supported:
                asserts.assert_greater_equal(current_state.position, min_position,
                                             "CurrentState Position is outside expected range")
                asserts.assert_less_equal(current_state.position, max_position, "CurrentState Position is outside expected range")
            asserts.assert_equal(current_state.latch, True, "CurrentState Latch is not True")

            if is_speed_supported:
                asserts.assert_greater_equal(current_state.speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.speed, 3, "CurrentState Speed is outside allowed range")
        else:
            logging.info("CurrentState attribute is not supported. Skipping step 5.")
            self.mark_current_step_skipped()

        # STEP 6a: Send Step command while device is latched
        self.step("6a")
        if is_positioning_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                        direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease, numberOfSteps=1),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Unexpected status returned")

        # STEP 6b: Send SetTarget command while device is latched
        self.step("6b")
        if is_positioning_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=max_position),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Unexpected status returned")

        # STEP 7a: If manual latching is required, unlatch device manually
        self.step("7a")
        if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlMode.kRemoteLatching:
            test_step = "Manual unlatch the device"
            self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when ready.")
        else:
            logging.info("Manual latching is not required. Skipping step.")
            self.mark_current_step_skipped()

        # STEP 7b: If manual latching is not required, skip steps 7c to 7e
        self.step("7b")
        if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlMode.kRemoteLatching:
            logging.info("Manual latching is not required. Skipping steps 7c to 7e.")
            self.skip_step("7c")
            self.skip_step("7d")
            self.skip_step("7e")
            return
        else:
            # STEP 7c: Send SetTarget command with Latch=False
            self.step("7c")
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=False),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected status returned")

            # STEP 7d: Verify Target attribute is updated
            self.step("7d")
            if attributes.Target.attribute_id in attribute_list:
                target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)

                asserts.assert_equal(target.latch, False, "Target Latch is not False")
            else:
                logging.info("Target attribute is not supported. Skipping step 7d.")
                self.mark_current_step_skipped()

            # STEP 7e: Wait for PIXIT.CLDIM.LatchingDuration seconds
            self.step("7e")
            time.sleep(latching_duration)

        # STEP 7f: Verify CurrentState attribute is updated
        self.step("7f")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            if is_positioning_supported:
                asserts.assert_greater_equal(current_state.position, min_position,
                                             "CurrentState Position is outside expected range")
                asserts.assert_less_equal(current_state.position, max_position, "CurrentState Position is outside expected range")
            asserts.assert_equal(current_state.latch, False, "CurrentState Latch is not False")

            if is_speed_supported:
                asserts.assert_greater_equal(current_state.speed, 0, "CurrentState Speed is outside allowed range")
                asserts.assert_less_equal(current_state.speed, 3, "CurrentState Speed is outside allowed range")
        else:
            logging.info("CurrentState attribute is not supported. Skipping step 6d.")
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
