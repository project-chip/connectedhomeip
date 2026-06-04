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

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, AttributeValue, MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


def current_latch_matcher(latch: bool) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureDimension.Attributes.CurrentState:
            return False
        return report.value.latch == latch
    return AttributeMatcher.from_callable(description=f"CurrentState.Latch is {latch}", matcher=predicate)


def current_position_matcher(position: int) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureDimension.Attributes.CurrentState:
            return False
        return report.value.position == position
    return AttributeMatcher.from_callable(description=f"CurrentState.Position is {position}", matcher=predicate)


class TC_CLDIM_5_3(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_5_3(self) -> str:
        return "[TC-CLDIM-5.3] GroupedSetTarget Command Field Sanity Check with DUT as Server"

    def steps_TC_CLDIM_5_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If Access feature is supported, skip remaining steps and end test case"),
            TestStep("2c", "If Limitation feature is supported, read LimitRange attribute"),
            TestStep("2d", "If Positioning feature is supported, read Resolution attribute"),
            TestStep("2e", "Establish wilcard subscription to all attributes"),
            TestStep("2f", "Read CurrentState attribute"),
            TestStep("2g", "If Latching feature is not supported or state is unlatched, skip steps 2h to 2m"),
            TestStep("2h", "Read LatchControlModes attribute"),
            TestStep("2i", "If LatchControlModes is manual unlatching, skip step 2j"),
            TestStep("2j", "Send GroupedSetTarget command with Latch=False"),
            TestStep("2k", "If LatchControlModes is remote unlatching, skip step 2l"),
            TestStep("2l", "Manually unlatch the device"),
            TestStep("2m", "Wait for CurrentState.Latched to be False"),
            TestStep(3, "Send GroupedSetTarget command with no fields"),
            TestStep("4a", "If Positioning feature is supported, skip step 4b to 4e"),
            TestStep("4b", "Send GroupedSetTarget command with Position MaxPosition"),
            TestStep("4c", "Send GroupedSetTarget command with Position below MinPosition"),
            TestStep("4d", "Send GroupedSetTarget command with Position above MaxPosition"),
            TestStep("4e", "Send GroupedSetTarget command with Position exceeding 100%"),
            TestStep("5a", "If LimitRange is unsupported, skip step 5b to 5g"),
            TestStep("5b", "Send GroupedSetTarget command with Position 0%"),
            TestStep("5c", "Verify TargetState attribute is updated"),
            TestStep("5d", "Wait for CurrentState.Position to be updated to MinPosition"),
            TestStep("5e", "Send GroupedSetTarget command with Position 100%"),
            TestStep("5f", "Verify TargetState attribute is updated"),
            TestStep("5g", "Wait for CurrentState.Position to be updated to 100%"),
            TestStep(6, "Send GroupedSetTarget command with invalid Position"),
            TestStep("7a", "If Positioning feature is unsupported, skip step 7b to 7j"),
            TestStep("7b", "Read CurrentState attribute"),
            TestStep("7c", "Send GroupedSetTarget command with Position not a multiple of Resolution"),
            TestStep("7d", "Verify TargetState attribute is updated"),
            TestStep("7e", "If not Resolution != 1: Wait for CurrentState.Position to be updated"),
            TestStep("7f", "If not Resolution == 1: Wait for CurrentState.Position to be updated"),
            TestStep("7g", "Send GroupedSetTarget command with Position not a multiple of Resolution"),
            TestStep("7h", "Verify TargetState attribute is updated"),
            TestStep("7i", "If Resolution <= 2 and position change expected: Wait for CurrentState.Position to be updated"),
            TestStep("7j", "If Resolution > 2 and position change expected: Wait for CurrentState.Position to be updated"),
            TestStep(8, "Send GroupedSetTarget command with Latch field when MotionLatching is unsupported"),
            TestStep(9, "Send GroupedSetTarget command with Speed field when Speed is unsupported"),
            TestStep(10, "Send GroupedSetTarget command with invalid Speed when Speed is unsupported"),
            TestStep(11, "Send GroupedSetTarget command with invalid Speed"),
        ]

    def pics_TC_CLDIM_5_3(self) -> list[str]:
        return [
            "CLDIM.S", "CLDIM.S.C02.Rsp"
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_CLDIM_5_3(self):
        endpoint = self.get_endpoint()
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
        is_access_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kAccess

        # STEP 2b: If Access feature is supported, skip remaining steps
        self.step("2b")
        if is_access_supported:
            log.info("Acess feature is supported. Skipping remaining steps.")
            self.mark_all_remaining_steps_skipped("2c")
            return

        # STEP 2c: Read LimitRange attribute if supported
        self.step("2c")
        if is_limitation_supported:
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            min_position = limit_range.min
            max_position = limit_range.max

        # STEP 2d: Read Resolution attribute if supported
        self.step("2d")
        resolution = 1  # Default resolution
        if is_positioning_supported:
            resolution = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Resolution)

        # STEP 2e: Establish wildcard subscription to all attributes"
        self.step("2e")
        sub_handler = AttributeSubscriptionHandler(expected_cluster=Clusters.ClosureDimension)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        # STEP 2f: Read CurrentState attribute
        self.step("2f")
        current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

        # STEP 2g: If Latching feature is not supported or state is unlatched, skip steps 2h to 2m
        self.step("2g")
        if (not is_latching_supported) or (not current_state.latch):
            log.info("Latching feature is not supported or state is unlatched. Skipping steps 2h to 2m.")
            self.mark_step_range_skipped("2h", "2m")
        else:
            # STEP 2h: Read LatchControlModes attribute
            self.step("2h")
            latch_control_modes = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)

            # STEP 2i: If LatchControlModes is manual unlatching, skip step 2j
            self.step("2i")
            sub_handler.reset()
            if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                log.info("LatchControlModes is manual unlatching. Skipping step 2j.")
                self.skip_step("2j")
            else:
                # STEP 2j: Send GroupedSetTarget command with Latch=False
                self.step("2j")
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(latch=False),
                    endpoint=endpoint
                )

            # STEP 2k: If LatchControlModes is remote unlatching, skip step 2l
            self.step("2k")
            if latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                log.info("LatchControlModes is remote unlatching. Skipping step 2l.")
                self.skip_step("2l")
            else:
                # STEP 2l: Manually unlatch the device
                self.step("2l")
                self.wait_for_user_input(prompt_msg="Manual unlatch the device, and press Enter when ready.")

            # STEP 2m: Wait for CurrentState.Latched to be False
            self.step("2m")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)

        # STEP 3: Send GroupedSetTarget command with no fields
        self.step(3)
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(),
                endpoint=endpoint
            )

            asserts.fail("Expected InvalidCommand for empty GroupedSetTarget command")

        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Unexpected status returned")

        # STEP 4a: If Positioning feature is supported, skip step 4b to 4e
        self.step("4a")
        if is_positioning_supported:
            log.info("Positioning feature is supported. Skipping steps 4b to 4e.")
            self.mark_step_range_skipped("4b", "4e")
        else:
            # STEP 4b: Send GroupedSetTarget command with Position MaxPosition
            self.step("4b")

            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(position=max_position),
                endpoint=endpoint
            )

            # STEP 4c: Send GroupedSetTarget command with Position below MinPosition
            self.step("4c")
            if min_position > 0:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(position=min_position - 1),
                    endpoint=endpoint
                )
            else:
                log.info("MinPosition not > 0. Skipping step 4c.")
                self.mark_current_step_skipped()

            # STEP 4d: Send GroupedSetTarget command with Position above MaxPosition
            self.step("4d")
            if max_position < 10000:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(position=max_position + 1),
                    endpoint=endpoint
                )
            else:
                log.info("MaxPosition not < 10000. Skipping step 4d.")
                self.mark_current_step_skipped()

            # STEP 4e: Send GroupedSetTarget command with Position exceeding 100%
            self.step("4e")
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(position=10001),
                endpoint=endpoint
            )

        # STEP 5a: If LimitRange is unsupported, skip step 5b to 5g
        self.step("5a")
        if (not is_positioning_supported) or (not is_limitation_supported):
            log.info("Positioning feature or Limitation feature is not supported. Skipping steps 5b to 5g.")
            self.mark_step_range_skipped("5b", "5g")
        else:
            # STEP 5b: Send GroupedSetTarget command with Position 0%
            self.step("5b")
            sub_handler.reset()
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(position=0),
                endpoint=endpoint
            )

            # STEP 5c: Verify TargetState attribute is updated
            self.step("5c")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
            asserts.assert_equal(target_state.position, min_position, "TargetState Position does not match MinPosition")

            # STEP 5d: Wait for CurrentState.Position to be updated to MinPosition
            self.step("5d")
            if current_state.position > 0:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(min_position)], timeout_sec=timeout)
            else:
                log.info("Initial Position not > 0. Skipping step 5d.")
                self.mark_current_step_skipped()

            # STEP 5e: Send GroupedSetTarget command with Position 100%
            self.step("5e")
            sub_handler.reset()

            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(position=10000),
                endpoint=endpoint
            )

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
                log.info("MaxPosition not < 10000. Skipping step 5g.")
                self.mark_current_step_skipped()

        # STEP 6: Send GroupedSetTarget command with invalid Position
        self.step(6)
        if is_positioning_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(position=10001),
                    endpoint=endpoint
                )
                asserts.fail("Expected ConstraintError for invalid Position")

            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned")
        else:
            log.info("Positioning feature is not supported. Skipping step 6.")
            self.mark_current_step_skipped()

        # STEP 7a: If Resolution is unsupported, skip step 7b to 7j
        self.step("7a")
        if (not is_positioning_supported):
            log.info("Positioning feature is not supported. Skipping steps 7b to 7j.")
            self.mark_step_range_skipped("7b", "7j")
        else:
            # STEP 7b: Read CurrentState attribute
            self.step("7b")
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            # STEP 7c: Send GroupedSetTarget command with Position not a multiple of Resolution
            self.step("7c")
            sub_handler.reset()
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(position=min_position + resolution - 1),
                endpoint=endpoint
            )

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
            if (resolution != 1) or (current_state.position == min_position):
                self.mark_current_step_skipped()
            else:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(min_position)], timeout_sec=timeout)
                current_state.position = min_position

            # STEP 7f: If not Resolution == 1: Wait for CurrentState.Position to be updated
            self.step("7f")
            if (resolution == 1) or (current_state.position == min_position + resolution):
                self.mark_current_step_skipped()
            else:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(min_position + resolution)], timeout_sec=timeout)
                current_state.position = min_position + resolution

            # STEP 7g: Send GroupedSetTarget command with Position not a multiple of Resolution
            self.step("7g")
            sub_handler.reset()
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(position=(max_position - resolution) + 1),
                endpoint=endpoint
            )

            # STEP 7h: Verify TargetState attribute is updated
            self.step("7h")
            target_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)

            if resolution <= 2:
                asserts.assert_equal(target_state.position, max_position, "TargetState Position does not match expected value")
            else:
                asserts.assert_equal(target_state.position, max_position - resolution,
                                     "TargetState Position does not match expected value")

            # STEP 7i: If Resolution <= 2 and position change expected: Wait for CurrentState.Position to be updated
            self.step("7i")
            if (resolution > 2 or current_state.position == max_position):
                self.mark_current_step_skipped()
            else:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(max_position)], timeout_sec=timeout)

            # STEP 7j: If Resolution > 2 and position change expected: Wait for CurrentState.Position to be updated
            self.step("7j")
            if (resolution <= 2 or current_state.position == max_position - resolution):
                self.mark_current_step_skipped()
            else:
                sub_handler.await_all_expected_report_matches(
                    expected_matchers=[current_position_matcher(max_position - resolution)], timeout_sec=timeout)

        # STEP 8: Send GroupedSetTarget command with Latch field when MotionLatching is unsupported
        self.step(8)
        if not is_latching_supported:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(latch=True),
                endpoint=endpoint
            )
        else:
            log.info("Latching feature is supported. Skipping step 8.")
            self.mark_current_step_skipped()

        # STEP 9: Send GroupedSetTarget command with Speed field when Speed is unsupported
        self.step(9)
        if not is_speed_supported:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(
                    speed=Globals.Enums.ThreeLevelAutoEnum.kHigh),
                endpoint=endpoint
            )
        else:
            log.info("Speed feature is supported. Skipping step 9.")
            self.mark_current_step_skipped()

        # STEP 10: Send GroupedSetTarget command with invalid Speed when Speed is unsupported
        self.step(10)
        if not is_speed_supported:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(speed=4),  # Invalid speed
                endpoint=endpoint
            )
        else:
            log.info("Speed feature is supported. Skipping step 10.")
            self.mark_current_step_skipped()

        # STEP 11: Send GroupedSetTarget command with invalid Speed
        self.step(11)
        if is_speed_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.GroupedSetTarget(speed=4),  # Invalid speed
                    endpoint=endpoint
                )

                asserts.fail("Expected ConstraintError for invalid Speed")

            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned")
        else:
            log.info("Speed feature is not supported. Skipping step 11.")
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
