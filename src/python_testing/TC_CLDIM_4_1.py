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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CLOSURE_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 2
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.interaction_model import InteractionModelError, Status
from chip.testing.event_attribute_reporting import ClusterAttributeChangeAccumulator
from chip.testing.matter_testing import (AttributeMatcher, AttributeValue, MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main)
from mobly import asserts


def current_latch_matcher(latch: bool) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureDimension.Attributes.CurrentState:
            return False
        if report.value.latch == latch:
            return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"CurrentState.Latch is {latch}", matcher=predicate)


def current_position_matcher(position: int) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureDimension.Attributes.CurrentState:
            return False
        if report.value.position == position:
            return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"CurrentState.Position is {position}", matcher=predicate)


def current_position_and_speed_matcher(position: int, speed: Globals.Enums.ThreeLevelAutoEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureDimension.Attributes.CurrentState:
            return False
        if (report.value.position == position) and (report.value.speed == speed):
            return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"CurrentState.Position is {position} and CurrentState.Speed is {speed}", matcher=predicate)


class TC_CLDIM_4_1(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_4_1(self) -> str:
        return "[TC-CLDIM-4.1] Step Command Primary Functionality with DUT as Server"

    def steps_TC_CLDIM_4_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If Positioning feature is not supported, skip remaining steps"),
            TestStep("2c", "Read StepValue attribute"),
            TestStep("2d", "If Limitation feature is supported, read LimitRange attribute"),
            TestStep("2e", "Establish wilcard subscription to all attributes"),
            TestStep("2f", "Read CurrentState attribute"),
            TestStep("2g", "If Latching feature not supported or state is unlatched, skip steps 2h to 2m"),
            TestStep("2h", "Read LatchControlModes attribute"),
            TestStep("2i", "If LatchControlModes is manual unlatching, skip step 2j"),
            TestStep("2j", "Send SetTarget command with Latch=False"),
            TestStep("2k", "If LatchControlModes is remote unlatching, skip step 2l"),
            TestStep("2l", "Manually unlatch the device"),
            TestStep("2m", "Wait for CurrentState.Latched to be False"),
            TestStep("3a", "Send Step command to increase position to MaxPosition"),
            TestStep("3b", "Wait for CurrentState.Position to be updated to MaxPosition"),
            TestStep("4a", "Send Step command to decrease position by 2 steps"),
            TestStep("4b", "Verify TargetState attribute is updated"),
            TestStep("4c", "Wait for CurrentState.Position to be updated"),
            TestStep("4d", "Send Step command to increase position by 2 steps"),
            TestStep("4e", "Verify TargetState attribute is updated"),
            TestStep("4f", "Wait for CurrentState.Position to be updated"),
            TestStep("5a", "If Speed Feature is not supported, skip step 5b to 5d"),
            TestStep("5b", "Send Step command to decrease position by 1 step with Speed=High"),
            TestStep("5c", "Verify TargetState attribute is updated"),
            TestStep("5d", "Wait for CurrentState to be updated"),
            TestStep("6a", "If Speed Feature is not supported, skip step 6b to 6d"),
            TestStep("6b", "Send Step command to increase position by 1 step with Speed=Auto"),
            TestStep("6c", "Verify TargetState attribute is updated"),
            TestStep("6d", "Wait for CurrentState to be updated"),
            TestStep("7a", "Read CurrentState attribute"),
            TestStep("7b", "Send Step command to decrease position by 65535"),
            TestStep("7c", "Verify TargetState attribute is at MinPosition"),
            TestStep("7d", "Wait for CurrentState to be updated"),
            TestStep("7e", "Send Step command to increase position by 65535"),
            TestStep("7f", "Verify TargetState attribute is at MaxPosition"),
            TestStep("7g", "Wait for CurrentState to be updated"),
        ]
        return steps

    def pics_TC_CLDIM_4_1(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_4_1(self):
        endpoint = self.get_endpoint(default=1)
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureDimension.Attributes

        # Default values
        min_position = 0
        max_position = 10000

        # STEP 2a: Read FeatureMap attribute
        self.step("2a")
        feature_map = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_positioning_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
        is_latching_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kMotionLatching
        is_limitation_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kLimitation
        is_speed_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kSpeed

        # STEP 2b: If Positioning feature is not supported, skip remaining steps
        self.step("2b")
        if not is_positioning_supported:
            logging.info("Positioning feature is not supported. Skipping remaining steps.")
            self.skip_all_remaining_steps("2c")
            return

        # STEP 2c: Read StepValue attribute
        self.step("2c")
        step_value = 1  # Default step value
        step_value = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.StepValue)

        # STEP 2d: Read LimitRange attribute
        self.step("2d")
        if is_limitation_supported:
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            min_position = limit_range.min
            max_position = limit_range.max

        # STEP 2e: Establish wildcard subscription to all attributes"
        self.step("2e")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureDimension)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        # STEP 2f: Read CurrentState attribute
        self.step("2f")
        initial_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

        # STEP 2g: If Latching feature is not supported or state is unlatched, skip steps 2h to 2m
        self.step("2g")
        if (not is_latching_supported) or (not initial_state.latch):
            logging.info("Latching feature is not supported or state is unlatched. Skipping steps 2h to 2m.")
            self.mark_step_range_skipped("2h", "2m")
        else:
            # STEP 2h: Read LatchControlModes attribute
            self.step("2h")
            latch_control_modes = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)

            # STEP 2i: If LatchControlModes is manual unlatching, skip step 2j
            self.step("2i")
            sub_handler.reset()
            if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                logging.info("LatchControlModes is manual unlatching. Skipping step 2j.")
                self.skip_step("2j")
            else:
                # STEP 2j: Send SetTarget command with Latch=False
                self.step("2j")
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=False),
                        endpoint=endpoint
                    )
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 2k: If LatchControlModes is remote unlatching, skip step 2l
            self.step("2k")
            if latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                logging.info("LatchControlModes is remote unlatching. Skipping step 2l.")
                self.skip_step("2l")
            else:
                # STEP 2l: Manually unlatch the device
                self.step("2l")
                self.wait_for_user_input(prompt_msg="Manual unlatch the device, and press Enter when ready.")

            # STEP 2m: Wait for CurrentState.Latched to be False
            self.step("2m")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)

        # STEP 3a: Send Step command to increase position to MaxPosition
        self.step("3a")
        sub_handler.reset()
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease, numberOfSteps=65535),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 3b: Wait for CurrentState.Position to be updated to MaxPosition
        self.step("3b")
        if initial_state.position == max_position:
            logging.info("MaxPosition == 0. Skipping step 3b.")
            self.mark_current_step_skipped()
        else:
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_position_matcher(max_position)], timeout_sec=timeout)

        # STEP 4a: Send Step command to decrease position by 2 steps
        self.step("4a")
        sub_handler.reset()
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease, numberOfSteps=2),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 4b: Verify TargetState attribute is updated
        self.step("4b")
        target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
        expected_position = max(max_position - 2 * step_value, min_position)
        asserts.assert_equal(target_state.position, expected_position, "TargetState Position is not updated correctly")

        # STEP 4c: Wait for CurrentState.Position to be updated
        self.step("4c")
        expected_position = max(max_position - 2 * step_value, min_position)
        sub_handler.await_all_expected_report_matches(
            expected_matchers=[current_position_matcher(expected_position)], timeout_sec=timeout)

        # STEP 4d: Send Step command to increase position by 2 steps
        self.step("4d")
        sub_handler.reset()
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease, numberOfSteps=2),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 4e: Verify TargetState attribute is updated
        self.step("4e")
        target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
        asserts.assert_equal(target_state.position, max_position, "TargetState Position is not updated correctly")

        # STEP 4f: Wait for CurrentState.Position to be updated
        self.step("4f")
        sub_handler.await_all_expected_report_matches(
            expected_matchers=[current_position_matcher(max_position)], timeout_sec=timeout)

        # STEP 5a: If Speed Feature is not supported, skip step 5b to 5d
        self.step("5a")
        if not is_speed_supported:
            logging.info("Speed feature is not supported. Skipping steps 5b to 5d.")
            self.mark_step_range_skipped("5b", "5d")
        else:
            # STEP 5b: Send Step command to decrease position by 1 step with Speed=High
            self.step("5b")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                        direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                        numberOfSteps=1,
                        speed=Globals.Enums.ThreeLevelAutoEnum.kHigh
                    ),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 5c: Verify TargetState attribute is updated
            self.step("5c")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            expected_position = max_position - step_value
            asserts.assert_equal(target_state.position, expected_position, "TargetState Position is not updated correctly")

            asserts.assert_equal(target_state.speed, Globals.Enums.ThreeLevelAutoEnum.kHigh,
                                 "TargetState Speed is not High")

            # STEP 5d: Wait for CurrentState to be updated
            self.step("5d")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_position_and_speed_matcher(max_position - step_value, Globals.Enums.ThreeLevelAutoEnum.kHigh)], timeout_sec=timeout)

        # STEP 6a: If Speed Feature is not supported, skip step 6b to 6d
        self.step("6a")
        if not is_speed_supported:
            logging.info("Speed feature is not supported. Skipping steps 6b to 6d.")
            self.mark_step_range_skipped("6b", "6d")
        else:
            # STEP 6b: Send Step command to increase position by 1 step with Speed=Auto
            self.step("6b")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                        direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease,
                        numberOfSteps=1,
                        speed=Globals.Enums.ThreeLevelAutoEnum.kAuto
                    ),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 6c: Verify TargetState attribute is updated
            self.step("6c")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            expected_position = max_position
            asserts.assert_equal(target_state.position, expected_position, "TargetState Position is not updated correctly")

            asserts.assert_equal(target_state.speed, Globals.Enums.ThreeLevelAutoEnum.kAuto,
                                 "TargetState Speed is not Auto")

            # STEP 6d: Wait for CurrentState to be updated
            self.step("6d")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_position_and_speed_matcher(max_position, Globals.Enums.ThreeLevelAutoEnum.kAuto)], timeout_sec=timeout)

        # STEP 7a: Read CurrentState attribute
        self.step("7a")
        initial_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

        # STEP 7b: Send Step command to decrease position beyond MinPosition
        self.step("7b")
        sub_handler.reset()
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                    numberOfSteps=65535
                ),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 7c: Verify TargetState attribute is at MinPosition
        self.step("7c")
        target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
        asserts.assert_equal(target_state.position, min_position, "TargetState Position is not at MinPosition")

        # STEP 7d: Wait for CurrentState to be updated
        self.step("7d")
        if initial_state.position == min_position:
            logging.info("Initial position == MinPosition. Skipping step 7d.")
            self.mark_current_step_skipped()
        else:
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_position_matcher(min_position)], timeout_sec=timeout)

        # STEP 7e: Send Step command to increase position beyond MaxPosition
        self.step("7e")
        sub_handler.reset()
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease,
                    numberOfSteps=65535
                ),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 7f: Verify TargetState attribute is at MaxPosition
        self.step("7f")
        target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
        asserts.assert_equal(target_state.position, max_position, "TargetState Position is not at MaxPosition")

        # STEP 7g: Wait for CurrentState to be updated
        self.step("7g")
        sub_handler.await_all_expected_report_matches(
            expected_matchers=[current_position_matcher(max_position)], timeout_sec=timeout)


if __name__ == "__main__":
    default_matter_test_main()
