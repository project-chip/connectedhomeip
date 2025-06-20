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
#        --trace-to json:${TRACE_TEST_JSON}.json
#        --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#      factory-reset: true
#      quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import chip.clusters as Clusters
from chip.clusters import Globals
from chip.interaction_model import (InteractionModelError, Status)
from chip.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main, AttributeMatcher, AttributeValue, ClusterAttributeChangeAccumulator)
from mobly import asserts


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
            TestStep("2d", "Read LimitRange attribute"),
            TestStep("2e", "Establish wilcard subscription to all attributes"),
            TestStep("2f", "Read CurrentState attribute"),
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
            TestStep("7a", "Send Step command to decrease position by 1 step"),
            TestStep("7b", "Send Step command to decrease position by 1 step"),
            TestStep("7c", "Send Step command to decrease position by 1 step"),
            TestStep("7d", "Verify TargetState attribute is updated"),
            TestStep("7e", "Wait for CurrentState to be updated"),
            TestStep("8a", "Read CurrentState attribute"),
            TestStep("8b", "Send Step command to decrease position by 65535"),
            TestStep("8c", "Verify TargetState attribute is at MinPosition"),
            TestStep("8d", "Wait for CurrentState to be updated"),
            TestStep("8e", "Send Step command to increase position by 65535"),
            TestStep("8f", "Verify TargetState attribute is at MaxPosition"),
            TestStep("8g", "Wait for CurrentState to be updated"),
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

        is_positioning_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
        is_speed_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kSpeed

        # STEP 2b: If Positioning feature is not supported, skip remaining steps
        self.step("2b")
        if not is_positioning_supported:
            logging.info("Positioning feature is not supported. Skipping remaining steps.")
            self.skip_all_remaining_steps("2c")
            return

        # STEP 2c: Read StepValue attribute
        self.step("2c")
        step_value = 1  # Default step value
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.StepValue):
            step_value = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.StepValue)

        # STEP 2d: Read LimitRange attribute
        self.step("2d")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.LimitRange):
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            min_position = limit_range.min
            max_position = limit_range.max

        # STEP 2e: Establish wildcard subscription to all attributes"
        self.step("2e")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureDimension)
        await sub_handler.start(self.default_controller, self.dut.node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30)

        # STEP 2f: Read CurrentState attribute
        self.step("2f")
        initial_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

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
        if initial_state.Position == max_position:
            logging.info("MaxPosition == 0. Skipping step 3b.")
            self.mark_current_step_skipped()
        else:
            sub_handler.await_all_expected_report_matches(
                expected_matches=[current_position_matcher(max_position)], timeout_sec=timeout)

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
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.TargetState):
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            expected_position = max(max_position - 2 * step_value, min_position)
            asserts.assert_equal(target_state.position, expected_position, "TargetState Position is not updated correctly")

        # STEP 4c: Wait for CurrentState.Position to be updated
        self.step("4c")
        expected_position = max(max_position - 2 * step_value, min_position)
        sub_handler.await_all_expected_report_matches(
            expected_matches=[current_position_matcher(expected_position)], timeout_sec=timeout)

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
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.TargetState):
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.position, max_position, "TargetState Position is not updated correctly")

        # STEP 4f: Wait for CurrentState.Position to be updated
        self.step("4f")
        sub_handler.await_all_expected_report_matches(
            expected_matches=[current_position_matcher(max_position)], timeout_sec=timeout)

        # STEP 5a: If Speed Feature is not supported, skip step 5b to 5d
        self.step("5a")
        if not is_speed_supported:
            logging.info("Speed feature is not supported. Skipping steps 5b to 5d.")
            self.skip_step("5b")
            self.skip_step("5c")
            self.skip_step("5d")
        else:
            # STEP 5b: Send Step command to decrease position by 1 step with Speed=High
            self.step("5b")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                        direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                        numberOfSteps=1,
                        speed=Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kHigh
                    ),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 5c: Verify TargetState attribute is updated
            self.step("5c")
            if await self.attribute_guard(endpoint=endpoint, attribute=attributes.TargetState):
                target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
                expected_position = max_position - step_value
                asserts.assert_equal(target_state.position, expected_position, "TargetState Position is not updated correctly")

                asserts.assert_equal(target_state.speed, Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kHigh,
                                     "TargetState Speed is not High")

            # STEP 5d: Wait for CurrentState to be updated
            self.step("5d")
            sub_handler.await_all_expected_report_matches(
                expected_matches=[current_position_and_speed_matcher(max_position - step_value, Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kHigh)], timeout_sec=timeout)

        # STEP 6a: If Speed Feature is not supported, skip step 6b to 6d
        self.step("6a")
        if not is_speed_supported:
            logging.info("Speed feature is not supported. Skipping steps 6b to 6d.")
            self.skip_step("6b")
            self.skip_step("6c")
            self.skip_step("6d")
        else:
            # STEP 6b: Send Step command to increase position by 1 step with Speed=Auto
            self.step("6b")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                        direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease,
                        numberOfSteps=1,
                        speed=Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kAuto
                    ),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 6c: Verify TargetState attribute is updated
            self.step("6c")
            if await self.attribute_guard(endpoint=endpoint, attribute=attributes.TargetState):
                target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
                expected_position = max_position
                asserts.assert_equal(target_state.position, expected_position, "TargetState Position is not updated correctly")

                asserts.assert_equal(target_state.speed, Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kAuto,
                                     "TargetState Speed is not Auto")

            # STEP 6d: Wait for CurrentState to be updated
            self.step("6d")
            sub_handler.await_all_expected_report_matches(
                expected_matches=[current_position_and_speed_matcher(max_position, Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kAuto)], timeout_sec=timeout)

        # STEP 7a: Send Step command to decrease position by 1 step
        self.step("7a")

        sub_handler.reset()
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                    numberOfSteps=1
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
                    direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                    numberOfSteps=1
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
                    direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease,
                    numberOfSteps=1
                ),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 7d: Verify TargetState attribute is updated after multiple steps
        self.step("7d")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.TargetState):
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            expected_position = max(max_position - 3 * step_value, min_position)
            asserts.assert_equal(target_state.position, expected_position, "TargetState Position is not updated correctly")

        # STEP 7e: Wait for CurrentState to be updated
        self.step("7e")
        expected_position = max(max_position - 3 * step_value, min_position)
        sub_handler.await_all_expected_report_matches(
            expected_matches=[current_position_matcher(expected_position)], timeout_sec=timeout)

        # STEP 8a: Read CurrentState attribute
        self.step("8a")
        initial_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

        # STEP 8b: Send Step command to decrease position beyond MinPosition
        self.step("8b")
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

        # STEP 8c: Verify TargetState attribute is at MinPosition
        self.step("8c")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.TargetState):
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.position, min_position, "TargetState Position is not at MinPosition")

        # STEP 8d: Wait for CurrentState to be updated
        self.step("8d")
        if initial_state.Position == min_position:
            logging.info("Initial position == MinPosition. Skipping step 8d.")
            self.mark_current_step_skipped()
        else:
            sub_handler.await_all_expected_report_matches(
                expected_matches=[current_position_matcher(min_position)], timeout_sec=timeout)

        # STEP 8e: Send Step command to increase position beyond MaxPosition
        self.step("8e")
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

        # STEP 8f: Verify TargetState attribute is at MaxPosition
        self.step("8f")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.TargetState):
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.position, max_position, "TargetState Position is not at MaxPosition")

        # STEP 8g: Wait for CurrentState to be updated
        self.step("8g")
        sub_handler.await_all_expected_report_matches(
            expected_matches=[current_position_matcher(max_position)], timeout_sec=timeout)


if __name__ == "__main__":
    default_matter_test_main()
