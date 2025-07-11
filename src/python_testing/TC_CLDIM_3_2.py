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
            TestStep("2c", "Read LimitRange attribute"),
            TestStep("2d", "Read LatchControlModes attribute"),
            TestStep("2e", "Establish wilcard subscription to all attributes"),
            TestStep("2f", "Read CurrentState attribute"),
            TestStep("2g", "If state is unlatched, skip steps 2h to 2l"),
            TestStep("2h", "If LatchControlModes is manual unlatching, skip step 2i"),
            TestStep("2i", "Send SetTarget command with Latch=False"),
            TestStep("2j", "If LatchControlModes is remote unlatching, skip step 2k"),
            TestStep("2k", "Manually unlatch the device"),
            TestStep("2l", "Wait for CurrentState.Latched to be False"),
            TestStep("3a", "If manual latching is required, skip steps 3b and 3c"),
            TestStep("3b", "Send SetTarget command with Latch=True"),
            TestStep("3c", "Manually latch the device"),
            TestStep("3d", "If manual latching is not required, skip steps 3e to 3f"),
            TestStep("3e", "Send SetTarget command with Latch=True"),
            TestStep("3f", "Verify TargetState attribute is updated"),
            TestStep("3g", "Wait for CurrentState.Latch to be updated to True"),
            TestStep("4a", "Send Step command while device is latched"),
            TestStep("4b", "Send SetTarget command while device is latched"),
            TestStep("5a", "If manual latching is required, unlatch device manually"),
            TestStep("5b", "If manual latching is required, skip steps 5c to 5d"),
            TestStep("5c", "Send SetTarget command with Latch=False"),
            TestStep("5d", "Verify TargetState attribute is updated"),
            TestStep("5e", "Wait for CurrentState.Latch to be updated to False"),
        ]
        return steps

    def pics_TC_CLDIM_3_2(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_3_2(self):
        endpoint = self.get_endpoint(default=1)
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureDimension.Attributes

        # Default values
        max_position = 10000

        # STEP 2a: Read feature map and determine supported features
        self.step("2a")
        feature_map = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_positioning_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
        is_latching_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kMotionLatching
        is_limitation_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kLimitation

        # STEP 2b: If MotionLatching Feature is not supported, skip remaining steps
        self.step("2b")
        if not is_latching_supported:
            logging.info("MotionLatching Feature is not supported. Skipping remaining steps.")
            self.skip_all_remaining_steps("2c")
            return

        # STEP 2c: Read LimitRange attribute if supported
        self.step("2c")
        if is_limitation_supported:
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            max_position = limit_range.max

        # STEP 2d: Read LatchControlModes attribute
        self.step("2d")
        latch_control_modes = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)

        # STEP 2e: Establish wildcard subscription to all attributes"
        self.step("2e")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureDimension)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        # STEP 2f: Read CurrentState attribute
        self.step("2f")
        initial_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

        # STEP 2g: If state is unlatched, skip steps 2h to 2l
        self.step("2g")
        if not initial_state.latch:
            logging.info("Latching feature is not supported or state is unlatched. Skipping steps 2h to 2l.")
            self.mark_step_range_skipped("2h", "2l")
        else:
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
                        endpoint=endpoint, timedRequestTimeoutMs=1000
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

        # STEP 3a: If manual latching is required, skip steps 3b and 3c
        self.step("3a")
        sub_handler.reset()
        if latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteLatching:
            logging.info("Manual latching is required. Skipping steps 3b and 3c.")
            self.mark_step_range_skipped("3b", "3c")
        else:
            # STEP 3b: Send SetTarget command with Latch=True
            self.step("3b")
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=True),
                    endpoint=endpoint, timedRequestTimeoutMs=1000
                )
                asserts.fail("Expected InvalidInState error, but no exception occurred.")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Unexpected status returned")

            # STEP 3c: Manually latch the device
            self.step("3c")
            self.wait_for_user_input(prompt_msg="Manual latch the device, and press Enter when ready.")

        # STEP 3d: If manual latching is not required, skip steps 3e to 3f
        self.step("3d")
        if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteLatching:
            self.mark_step_range_skipped("3e", "3f")
        else:
            # STEP 3e: Send SetTarget command with Latch=True
            self.step("3e")
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=True),
                    endpoint=endpoint, timedRequestTimeoutMs=1000
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 3f: Verify TargetState attribute is updated
            self.step("3f")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.latch, True, "TargetState Latch is not True")

        # STEP 3g: Wait for CurrentState.Latch to be updated to True
        self.step("3g")
        sub_handler.await_all_expected_report_matches(
            expected_matchers=[current_latch_matcher(True)], timeout_sec=timeout)

        # STEP 4a: Send Step command while device is latched
        self.step("4a")
        if is_positioning_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                        direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kDecrease, numberOfSteps=1),
                    endpoint=endpoint, timedRequestTimeoutMs=1000
                )
                asserts.fail("Expected InvalidInState error, but no exception occurred.")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Unexpected status returned")

        # STEP 4b: Send SetTarget command while device is latched
        self.step("4b")
        if is_positioning_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=max_position),
                    endpoint=endpoint, timedRequestTimeoutMs=1000
                )
                asserts.fail("Expected InvalidInState error, but no exception occurred.")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Unexpected status returned")

        # STEP 5a: If manual unlatching is required, unlatch device manually
        self.step("5a")
        if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
            self.wait_for_user_input(prompt_msg="Manual unlatch the device, and press Enter when ready.")
        else:
            logging.info("Manual latching is not required. Skipping step.")
            self.mark_current_step_skipped()

        # STEP 5b: If manual unlatching is not required, skip steps 5c to 5d
        self.step("5b")
        if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
            logging.info("Manual latching is not required. Skipping steps 5c to 5d.")
            self.mark_step_range_skipped("5c", "5d")
            return
        else:
            # STEP 5c: Send SetTarget command with Latch=False
            self.step("5c")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=False),
                    endpoint=endpoint, timedRequestTimeoutMs=1000
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected status returned")

            # STEP 5d: Verify TargetState attribute is updated
            self.step("5d")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.latch, False, "TargetState Latch is not False")

        # STEP 5e: Wait for CurrentState.Latch to be updated to False
        self.step("5e")
        sub_handler.await_all_expected_report_matches(
            expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)


if __name__ == "__main__":
    default_matter_test_main()
