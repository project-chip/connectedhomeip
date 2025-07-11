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


class TC_CLDIM_4_2(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_4_2(self) -> str:
        return "[TC-CLDIM-4.2] Step Command Input Sanity Check with DUT as Server"

    def steps_TC_CLDIM_4_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "If Positioning feature is not supported, skip remaining steps"),
            TestStep("2c", "Establish wilcard subscription to all attributes"),
            TestStep("2d", "Read CurrentState attribute"),
            TestStep("2e", "If Latching feature not supported or state is unlatched, skip steps 2f to 2k"),
            TestStep("2f", "Read LatchControlModes attribute"),
            TestStep("2g", "If LatchControlModes is manual unlatching, skip step 2h"),
            TestStep("2h", "Send SetTarget command with Latch=False"),
            TestStep("2i", "If LatchControlModes is remote unlatching, skip step 2j"),
            TestStep("2j", "Manually unlatch the device"),
            TestStep("2k", "Wait for CurrentState.Latched to be False"),
            TestStep(3, "Send Step command with invalid Direction"),
            TestStep(4, "Send Step command with invalid Speed"),
            TestStep(5, "Send Step command with NumberOfSteps = 0"),
        ]
        return steps

    def pics_TC_CLDIM_4_2(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_4_2(self):
        endpoint = self.get_endpoint(default=1)
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout
        attributes = Clusters.ClosureDimension.Attributes

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)

        # STEP 2a: Read FeatureMap attribute
        self.step("2a")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.ClosureDimension,
            attribute=attributes.FeatureMap
        )

        is_positioning_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
        is_latching_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kMotionLatching
        is_speed_supported: bool = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kSpeed

        # STEP 2b: If Positioning feature is not supported, skip remaining steps
        self.step("2b")
        if not is_positioning_supported:
            logging.info("Positioning feature is not supported. Skipping remaining steps.")
            self.skip_all_remaining_steps("2c")
            return

        # STEP 2c: Establish wildcard subscription to all attributes"
        self.step("2c")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureDimension)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        # STEP 2d: Read CurrentState attribute
        self.step("2d")
        initial_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

        # STEP 2e: If Latching feature is not supported or state is unlatched, skip steps 2f to 2k
        self.step("2e")
        if (not is_latching_supported) or (not initial_state.latch):
            logging.info("Latching feature is not supported or state is unlatched. Skipping steps 2f to 2k.")
            self.mark_step_range_skipped("2f", "2k")
        else:
            # STEP 2f: Read LatchControlModes attribute
            self.step("2f")
            latch_control_modes = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)

            # STEP 2g: If LatchControlModes is manual unlatching, skip step 2h
            self.step("2g")
            sub_handler.reset()
            if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                logging.info("LatchControlModes is manual unlatching. Skipping step 2h.")
                self.skip_step("2h")
            else:
                # STEP 2h: Send SetTarget command with Latch=False
                self.step("2h")
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=False),
                        endpoint=endpoint, timedRequestTimeoutMs=1000
                    )
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 2i: If LatchControlModes is remote unlatching, skip step 2j
            self.step("2i")
            if latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                logging.info("LatchControlModes is remote unlatching. Skipping step 2j.")
                self.skip_step("2j")
            else:
                # STEP 2j: Manually unlatch the device
                self.step("2j")
                self.wait_for_user_input(prompt_msg="Manual unlatch the device, and press Enter when ready.")

            # STEP 2k: Wait for CurrentState.Latched to be False
            self.step("2k")
            sub_handler.await_all_expected_report_matches(
                expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)

        # STEP 3: Send Step command with invalid Direction
        self.step(3)
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kUnknownEnumValue,
                    numberOfSteps=1
                ),
                endpoint=endpoint, timedRequestTimeoutMs=1000
            )

            asserts.fail("Expected ConstraintError for invalid Direction")

        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned for invalid Direction")

        # STEP 4: Send Step command with invalid Speed
        self.step(4)
        if is_speed_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                        direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease,
                        numberOfSteps=1,
                        speed=Globals.Enums.ThreeLevelAutoEnum.kUnknownEnumValue,
                    ),
                    endpoint=endpoint, timedRequestTimeoutMs=1000
                )

                asserts.fail("Expected ConstraintError for invalid Speed")

            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned for invalid Speed")

        # STEP 5: Send Step command with NumberOfSteps = 0
        self.step(5)
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.Step(
                    direction=Clusters.ClosureDimension.Enums.StepDirectionEnum.kIncrease,
                    numberOfSteps=0
                ),
                endpoint=endpoint, timedRequestTimeoutMs=1000
            )

            asserts.fail("Expected ConstraintError for NumberOfSteps = 0")

        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned for NumberOfSteps = 0")


if __name__ == "__main__":
    default_matter_test_main()
