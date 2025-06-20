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
            TestStep("2c", "Read AttributeList attribute"),
            TestStep("2d", "Read LimitRange attribute"),
            TestStep("2e", "Establish wilcard subscription to all attributes"),
            TestStep("2f", "Read CurrentState attribute"),
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

        is_positioning_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
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
            min_position = limit_range.min
            max_position = limit_range.max

        # STEP 2e: Establish wildcard subscription to all attributes"
        self.step("2e")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureDimension)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30)

        # STEP 2f: Read CurrentState attribute
        self.step("2f")
        initial_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

        # STEP 3a: If Position = MaxPosition, skip steps 3b to 3d
        self.step("3a")
        if initial_state.position == max_position:
            logging.info("Initial Position is already at MaxPosition. Skipping steps 3b to 3d.")
            self.skip_step("3b")
            self.skip_step("3c")
            self.skip_step("3d")
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
            if attributes.TargetState.attribute_id in attribute_list:
                target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)

                asserts.assert_equal(target_state.position, max_position, "TargetState Position does not match MaxPosition")
            else:
                logging.info("TargetState attribute is not supported. Skipping step 3b.")
                self.mark_current_step_skipped()

            # STEP 3d: Wait for CurrentState.Position to be updated to MaxPosition
            self.step("3d")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_position_matcher(max_position)], timeout_sec=timeout)

        # STEP 4a: If Speed feature is not supported, skip step 4b to 4d
        self.step("4a")
        if (not is_speed_supported) or (initial_state.speed == Globals.Enums.ThreeLevelAutoEnum.kMedium):
            logging.info("Speed feature is not supported. Skipping steps 4b to 4d.")
            self.skip_step("4b")
            self.skip_step("4c")
            self.skip_step("4d")
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
            if attributes.TargetState.attribute_id in attribute_list:
                target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)

                asserts.assert_equal(target_state.speed, Globals.Enums.ThreeLevelAutoEnum.kMedium,
                                     "TargetState Speed does not match Medium")
            else:
                logging.info("TargetState attribute is not supported. Skipping step 4c.")
                self.mark_current_step_skipped()

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
        if attributes.TargetState.attribute_id in attribute_list:
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)

            asserts.assert_equal(target_state.position, min_position, "TargetState Position does not match MinPosition")
        else:
            logging.info("TargetState attribute is not supported. Skipping step 5b.")
            self.mark_current_step_skipped()

        # STEP 5c: Wait for CurrentState.Position to be updated to MinPosition
        self.step("5c")
        sub_handler.await_all_expected_report_matches(
            expected_matchers=[current_position_matcher(min_position)], timeout_sec=timeout)

        # STEP 6a: If Speed feature is not supported, skip step 6b to 6d
        self.step("6a")
        if not is_speed_supported:
            logging.info("Speed feature is not supported. Skipping steps 6b to 6d.")
            self.skip_step("6b")
            self.skip_step("6c")
            self.skip_step("6d")
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
            if attributes.TargetState.attribute_id in attribute_list:
                target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)

                asserts.assert_equal(target_state.position, max_position, "TargetState Position does not match MaxPosition")

                asserts.assert_equal(target_state.speed, Globals.Enums.ThreeLevelAutoEnum.kHigh,
                                     "TargetState Speed does not match High")
            else:
                logging.info("TargetState attribute is not supported. Skipping step 6c.")
                self.mark_current_step_skipped()

            # STEP 6d: Wait for CurrentState.Position to be updated to MaxPosition and CurrentState.Speed to High
            self.step("6d")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_position_and_speed_matcher(max_position, Globals.Enums.ThreeLevelAutoEnum.kHigh)], timeout_sec=timeout)


if __name__ == "__main__":
    default_matter_test_main()
