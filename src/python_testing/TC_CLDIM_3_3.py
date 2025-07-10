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


class TC_CLDIM_3_3(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_3_3(self) -> str:
        return "[TC-CLDIM-3.3] SetTarget Command Field Sanity Check with DUT as Server"

    def steps_TC_CLDIM_3_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If Limitation feature is supportes, read LimitRange attribute"),
            TestStep("2c", "If Positioning feature is supportes, read Resolution attribute"),
            TestStep("2d", "Establish wilcard subscription to all attributes"),
            TestStep("2e", "Read CurrentState attribute"),
            TestStep("2f", "If Latching feature is not supported or state is unlatched, skip steps 2g to 2l"),
            TestStep("2g", "Read LatchControlModes attribute"),
            TestStep("2h", "If LatchControlModes is manual unlatching, skip step 2i"),
            TestStep("2i", "Send SetTarget command with Latch=False"),
            TestStep("2j", "If LatchControlModes is remote unlatching, skip step 2k"),
            TestStep("2k", "Manually unlatch the device"),
            TestStep("2l", "Wait for CurrentState.Latched to be False"),
            TestStep(3, "Send SetTarget command with no fields"),
            TestStep("4a", "If Positioning feature is supported, skip step 4b to 4e"),
            TestStep("4b", "Send SetTarget command with Position MaxPosition"),
            TestStep("4c", "Send SetTarget command with Position below MinPosition"),
            TestStep("4d", "Send SetTarget command with Position above MaxPosition"),
            TestStep("4e", "Send SetTarget command with Position exceeding 100%"),
            TestStep("5a", "If LimitRange is unsupported, skip step 5b to 5g"),
            TestStep("5b", "Send SetTarget command with Position 0%"),
            TestStep("5c", "Verify TargetState attribute is updated"),
            TestStep("5d", "Wait for CurrentState.Position to be updated to 0%"),
            TestStep("5e", "Send SetTarget command with Position 100%"),
            TestStep("5f", "Verify TargetState attribute is updated"),
            TestStep("5g", "Wait for CurrentState.Position to be updated to 100%"),
            TestStep(6, "Send SetTarget command with invalid Position"),
            TestStep("7a", "If Positioning feature is unsupported, skip step 7b to 7j"),
            TestStep("7b", "Read CurrentState attribute"),
            TestStep("7c", "Send SetTarget command with Position not a multiple of Resolution"),
            TestStep("7d", "Verify TargetState attribute is updated"),
            TestStep("7e", "If not Resolution != 1: Wait for CurrentState.Position to be updated"),
            TestStep("7f", "If not Resolution == 1: Wait for CurrentState.Position to be updated"),
            TestStep("7g", "Send SetTarget command with Position not a multiple of Resolution"),
            TestStep("7h", "Verify TargetState attribute is updated"),
            TestStep("7i", "If not Resolution > 2: Wait for CurrentState.Position to be updated"),
            TestStep("7j", "If not Resolution <= 2: Wait for CurrentState.Position to be updated"),
            TestStep(8, "Send SetTarget command with Latch field when MotionLatching is unsupported"),
            TestStep(9, "Send SetTarget command with Speed field when Speed is unsupported"),
            TestStep(10, "Send SetTarget command with invalid Speed when Speed is unsupported"),
            TestStep(11, "Send SetTarget command with invalid Speed"),
        ]
        return steps

    def pics_TC_CLDIM_3_3(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_3_3(self):
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

        # STEP 2b: Read LimitRange attribute if supported
        self.step("2b")
        if is_limitation_supported:
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            min_position = limit_range.min
            max_position = limit_range.max

        # STEP 2c: Read Resolution attribute if supported
        self.step("2c")
        resolution = 1  # Default resolution
        if is_positioning_supported:
            resolution = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Resolution)

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

        # STEP 3: Send SetTarget command with no fields
        self.step(3)
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(),
                endpoint=endpoint
            )

            asserts.fail("Expected InvalidCommand for empty SetTarget command")

        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Unexpected status returned")

        # STEP 4a: If Positioning feature is supported, skip step 4b to 4e
        self.step("4a")
        if is_positioning_supported:
            logging.info("Positioning feature is supported. Skipping steps 4b to 4e.")
            self.mark_step_range_skipped("4b", "4e")
        else:
            # STEP 4b: Send SetTarget command with Position MaxPosition
            self.step("4b")

            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=max_position),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 4c: Send SetTarget command with Position below MinPosition
            self.step("4c")
            if min_position > 0:
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=min_position - 1),
                        endpoint=endpoint
                    )
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            else:
                logging.info("MinPosition not > 0. Skipping step 4c.")
                self.mark_current_step_skipped()

            # STEP 4d: Send SetTarget command with Position above MaxPosition
            self.step("4d")
            if max_position < 10000:
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=max_position + 1),
                        endpoint=endpoint
                    )
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            else:
                logging.info("MaxPosition not < 10000. Skipping step 4d.")
                self.mark_current_step_skipped()

            # STEP 4e: Send SetTarget command with Position exceeding 100%
            self.step("4e")
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=10001),
                    endpoint=endpoint
                )

                asserts.fail("Expected ConstraintError for Position exceeding 100%")

            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned")

        # STEP 5a: If LimitRange is unsupported, skip step 5b to 5g
        self.step("5a")
        if (not is_positioning_supported) or (not is_limitation_supported):
            logging.info("Positioning feature or Limitation feature is not supported. Skipping steps 5b to 5g.")
            self.mark_step_range_skipped("5b", "5g")
        else:
            # STEP 5b: Send SetTarget command with Position 0%
            self.step("5b")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=0),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 5c: Verify TargetState attribute is updated
            self.step("5c")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.position, min_position, "TargetState Position does not match MinPosition")

            # STEP 5d: Wait for CurrentState.Position to be updated to 0%
            self.step("5d")
            if initial_state.position > 0:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(0)], timeout_sec=timeout)
            else:
                logging.info("Initial Position not > 0. Skipping step 5d.")
                self.mark_current_step_skipped()

            # STEP 5e: Send SetTarget command with Position 100%
            self.step("5e")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=10000),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 5f: Verify TargetState attribute is updated
            self.step("5f")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.position, max_position, "TargetState Position does not match MaxPosition")

            # STEP 5g: Wait for CurrentState.Position to be updated to 100%
            self.step("5g")
            if max_position < 10000:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(max_position)], timeout_sec=timeout)
            else:
                logging.info("MaxPosition not < 10000. Skipping step 5g.")
                self.mark_current_step_skipped()

        # STEP 6: Send SetTarget command with invalid Position
        self.step(6)
        if is_positioning_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=10001),
                    endpoint=endpoint
                )
                asserts.fail("Expected ConstraintError for invalid Position")

            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned")
        else:
            logging.info("Positioning feature is not supported. Skipping step 6.")
            self.mark_current_step_skipped()

        # STEP 7a: If Resolution is unsupported, skip step 7b to 7j
        self.step("7a")
        if (not is_positioning_supported):
            logging.info("Positioning feature is not supported. Skipping steps 7b to 7j.")
            self.mark_step_range_skipped("7b", "7j")
        else:
            # STEP 7b: Read CurrentState attribute
            self.step("7b")
            initial_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            # STEP 7c: Send SetTarget command with Position not a multiple of Resolution
            self.step("7c")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=min_position + resolution - 1),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 7d: Verify TargetState attribute is updated
            self.step("7d")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)

            if resolution == 1:
                asserts.assert_equal(target_state.position, min_position, "TargetState Position does not match expected value")
            else:
                asserts.assert_equal(target_state.position, min_position + resolution,
                                     "TargetState Position does not match expected value")

            # STEP 7e: If not Resolution != 1: Wait for CurrentState.Position to be updated
            self.step("7e")
            if (resolution != 1) or (initial_state.position == min_position):
                self.mark_current_step_skipped()
            else:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(min_position)], timeout_sec=timeout)

            # STEP 7f: If not Resolution == 1: Wait for CurrentState.Position to be updated
            self.step("7f")
            if (resolution == 1) or (initial_state.position == min_position + resolution):
                self.mark_current_step_skipped()
            else:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(min_position + resolution)], timeout_sec=timeout)

            # STEP 7g: Send SetTarget command with Position not a multiple of Resolution
            self.step("7g")
            sub_handler.reset()
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=(max_position - resolution) + 1),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 7h: Verify TargetState attribute is updated
            self.step("7h")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)

            if resolution <= 2:
                asserts.assert_equal(target_state.position, max_position, "TargetState Position does not match expected value")
            else:
                asserts.assert_equal(target_state.position, max_position - resolution,
                                     "TargetState Position does not match expected value")

            # STEP 7i: If not Resolution > 2: Wait for CurrentState.Position to be updated
            self.step("7i")
            if (resolution > 2):
                self.mark_current_step_skipped()
            else:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(max_position)], timeout_sec=timeout)

            # STEP 7j: If not Resolution <= 2: Wait for CurrentState.Position to be updated
            self.step("7j")
            if (resolution <= 2):
                self.mark_current_step_skipped()
            else:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(max_position - resolution)], timeout_sec=timeout)

        # STEP 8: Send SetTarget command with Latch field when MotionLatching is unsupported
        self.step(8)
        if not is_latching_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=True),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
        else:
            logging.info("Latching feature is supported. Skipping step 8.")
            self.mark_current_step_skipped()

        # STEP 9: Send SetTarget command with Speed field when Speed is unsupported
        self.step(9)
        if not is_speed_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(
                        speed=Globals.Enums.ThreeLevelAutoEnum.kHigh),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
        else:
            logging.info("Speed feature is supported. Skipping step 9.")
            self.mark_current_step_skipped()

        # STEP 10: Send SetTarget command with invalid Speed when Speed is unsupported
        self.step(10)
        if not is_speed_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(speed=4),  # Invalid speed
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
        else:
            logging.info("Speed feature is supported. Skipping step 10.")
            self.mark_current_step_skipped()

        # STEP 11: Send SetTarget command with invalid Speed
        self.step(11)
        if is_speed_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(speed=4),  # Invalid speed
                    endpoint=endpoint
                )

                asserts.fail("Expected ConstraintError for invalid Speed")

            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned")
        else:
            logging.info("Speed feature is not supported. Skipping step 11.")
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
