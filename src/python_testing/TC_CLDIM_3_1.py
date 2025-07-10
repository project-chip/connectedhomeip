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


def current_speed_matcher(speed: Globals.Enums.ThreeLevelAutoEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureDimension.Attributes.CurrentState:
            return False
        if report.value.speed == speed:
            return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"CurrentState.Speed is {speed}", matcher=predicate)


def current_position_and_speed_matcher(position: int, speed: Globals.Enums.ThreeLevelAutoEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureDimension.Attributes.CurrentState:
            return False
        if (report.value.position == position) and (report.value.speed == speed):
            return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"CurrentState.Position is {position} and CurrentState.Speed is {speed}", matcher=predicate)


class TC_CLDIM_3_1(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_3_1(self) -> str:
        return "[TC-CLDIM-3.1] SetTarget Command Positioning Functionality with DUT as Server"

    def steps_TC_CLDIM_3_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If Positioning feature is not supported, skip remaining steps"),
            TestStep("2c", "Read LimitRange attribute"),
            TestStep("2d", "Establish wilcard subscription to all attributes"),
            TestStep("2e", "Read CurrentState attribute"),
            TestStep("2f", "If Latching feature is not supported or state is unlatched, skip steps 2g to 2l"),
            TestStep("2g", "Read LatchControlModes attribute"),
            TestStep("2h", "If LatchControlModes is manual unlatching, skip step 2i"),
            TestStep("2i", "Send SetTarget command with Latch=False"),
            TestStep("2j", "If LatchControlModes is remote unlatching, skip step 2k"),
            TestStep("2k", "Manually unlatch the device"),
            TestStep("2l", "Wait for CurrentState.Latched to be False"),
            TestStep("3a", "If Position = MaxPosition, skip steps 3b to 3d"),
            TestStep("3b", "Set Position to MaxPosition"),
            TestStep("3c", "Verify TargetState attribute is updated"),
            TestStep("3d", "Wait for CurrentState.Position to be updated to MaxPosition"),
            TestStep("4a", "If Speed feature is not supported or if current speed = Medium, skip steps 4b to 4d"),
            TestStep("4b", "Set Speed to Medium"),
            TestStep("4c", "Verify TargetState attribute is updated"),
            TestStep("4d", "Wait for CurrentState.Speed to be updated to Medium"),
            TestStep("5a", "Set Position to min_position"),
            TestStep("5b", "Verify TargetState attribute is updated"),
            TestStep("5c", "Wait for CurrentState.Position to be updated to MinPosition"),
            TestStep("6a", "If Speed feature is not supported, skip step 6b to 6d"),
            TestStep("6b", "Set Position to MaxPosition and Speed to High"),
            TestStep("6c", "Verify TargetState attribute is updated"),
            TestStep("6d", "Wait for CurrentState.Position to be updated to MaxPosition and CurrentState.Speed to High"),
        ]
        return steps

    def pics_TC_CLDIM_3_1(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_3_1(self):
        endpoint = self.get_endpoint(default=1)
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureDimension.Attributes

        # Default values
        min_position = 0
        max_position = 10000

        # STEP 2a: Read feature map and determine supported features
        self.step("2a")
        feature_map = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_positioning_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
        is_latching_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kMotionLatching
        is_limitation_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kLimitation
        is_speed_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kSpeed

        # STEP 2b: If Positioning Feature is not supported, skip remaining steps
        self.step("2b")
        if not is_positioning_supported:
            logging.info("Positioning Feature is not supported. Skipping remaining steps.")
            self.skip_all_remaining_steps("2c")
            return

        # STEP 2c: Read LimitRange attribute if supported
        self.step("2c")
        if is_limitation_supported:
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            min_position = limit_range.min
            max_position = limit_range.max

        # STEP 2d: Establish wildcard subscription to all attributes"
        self.step("2d")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureDimension)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        # STEP 2e: Read CurrentState attribute
        self.step("2e")
        initial_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

        # STEP 2f: If Latching feature is not supported or state is unlatched, skip steps 2g to 2l
        self.step("2f")
        if (not is_latching_supported) or (not initial_state.latch):
            logging.info("Latching feature is not supported or state is unlatched. Skipping steps 2g to 2l.")
            self.mark_step_range_skipped("2g", "2l")
        else:
            # STEP 2g: Read LatchControlModes attribute
            self.step("2g")
            latch_control_modes = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)

            # STEP 2h: If LatchControlModes is manual unlatching, skip step 2i
            self.step("2h")
            sub_handler.reset()
            if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                logging.info("LatchControlModes is manual unlatching. Skipping step 2i.")
                self.skip_step("2i")
            else:
                # STEP 2i: Send SetTarget command with Latch=False
                self.step("2i")
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=False),
                        endpoint=endpoint
                    )
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 2j: If LatchControlModes is remote unlatching, skip step 2k
            self.step("2j")
            if latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                logging.info("LatchControlModes is remote unlatching. Skipping step 2k.")
                self.skip_step("2k")
            else:
                # STEP 2k: Manually unlatch the device
                self.step("2k")
                self.wait_for_user_input(prompt_msg="Manual unlatch the device, and press Enter when ready.")

            # STEP 2l: Wait for CurrentState.Latched to be False
            self.step("2l")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)

        # STEP 3a: If Position = MaxPosition, skip steps 3b to 3d
        self.step("3a")
        if initial_state.position == max_position:
            logging.info("Initial Position is already at MaxPosition. Skipping steps 3b to 3d.")
            self.mark_step_range_skipped("3b", "3d")
        else:
            # STEP 3b: Set Position to MaxPosition
            self.step("3b")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=max_position),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 3c: Verify TargetState attribute is updated
            self.step("3c")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.position, max_position, "TargetState Position does not match MaxPosition")

            # STEP 3d: Wait for CurrentState.Position to be updated to MaxPosition
            self.step("3d")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_position_matcher(max_position)], timeout_sec=timeout)

        # STEP 4a: If Speed feature is not supported, skip step 4b to 4d
        self.step("4a")
        if (not is_speed_supported) or (initial_state.speed == Globals.Enums.ThreeLevelAutoEnum.kMedium):
            logging.info("Speed feature is not supported. Skipping steps 4b to 4d.")
            self.mark_step_range_skipped("4b", "4d")
        else:
            # STEP 4b: Set Speed to Medium
            self.step("4b")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(
                        speed=Globals.Enums.ThreeLevelAutoEnum.kMedium),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 4c: Verify TargetState attribute is updated
            self.step("4c")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.speed, Globals.Enums.ThreeLevelAutoEnum.kMedium,
                                 "TargetState Speed does not match Medium")

            # STEP 4d: Wait for CurrentState.Speed to be updated to Medium
            self.step("4d")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_speed_matcher(Globals.Enums.ThreeLevelAutoEnum.kMedium)], timeout_sec=timeout)

        # STEP 5a: Set Position to min_position
        self.step("5a")
        sub_handler.reset()
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=min_position),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 5b: Verify TargetState attribute is updated
        self.step("5b")
        target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
        asserts.assert_equal(target_state.position, min_position, "TargetState Position does not match MinPosition")

        # STEP 5c: Wait for CurrentState.Position to be updated to MinPosition
        self.step("5c")
        sub_handler.await_all_expected_report_matches(
            expected_matchers=[current_position_matcher(min_position)], timeout_sec=timeout)

        # STEP 6a: If Speed feature is not supported, skip step 6b to 6d
        self.step("6a")
        if not is_speed_supported:
            logging.info("Speed feature is not supported. Skipping steps 6b to 6d.")
            self.mark_step_range_skipped("6b", "6d")
        else:
            # STEP 6b: Set Position to MaxPosition and Speed to High
            self.step("6b")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(
                        position=max_position, speed=Globals.Enums.ThreeLevelAutoEnum.kHigh),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 6c: Verify TargetState attribute is updated
            self.step("6c")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.position, max_position, "TargetState Position does not match MaxPosition")
            asserts.assert_equal(target_state.speed, Globals.Enums.ThreeLevelAutoEnum.kHigh,
                                 "TargetState Speed does not match High")

            # STEP 6d: Wait for CurrentState.Position to be updated to MaxPosition and CurrentState.Speed to High
            self.step("6d")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_position_and_speed_matcher(max_position, Globals.Enums.ThreeLevelAutoEnum.kHigh)], timeout_sec=timeout)


if __name__ == "__main__":
    default_matter_test_main()
