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
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import (AttributeValue, ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep,
                                         async_test_body, default_matter_test_main)
from mobly import asserts


class TC_CLDIM_5_1(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_5_1(self) -> str:
        return "[TC-CLDIM-5.1] Subscription Report Verification with DUT as Server"

    def steps_TC_CLDIM_5_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "Read AttributeList attribute"),
            TestStep("2c", "Read LimitRange attribute"),
            TestStep("2d", "Read LacthControlModes attribute"),
            TestStep("2e", "Setup subscription to CurrentState attribute"),
            TestStep("3a", "If Positioning feature is not supported, skip steps 3b to 3f"),
            TestStep("3b", "Read CurrentState attribute and save as ExpectedState"),
            TestStep("3c", "If ExpectedState.Position != MinPosition, send SetTarget command with Position=MinPosition"),
            TestStep("3d", "If ExpectedState.Position != MinPosition, wait for CurrentState.Position to be updated to MinPosition"),
            TestStep("3e", "Send SetTarget command with Position=MaxPosition"),
            TestStep("3f", "Wait for CurrentState.Position to be updated to MaxPosition"),
            TestStep("4a", "If Latching feature is not supported, skip steps 4b to 4g"),
            TestStep("4b", "Read CurrentState attribute and save as ExpectedState"),
            TestStep("4c", "If manual latching is required, manually latch the device"),
            TestStep("4d", "If manual latching is not required, send SetTarget command with Latch=True"),
            TestStep("4e", "Wait for CurrentState.Latch to be updated to True"),
            TestStep("4f", "If manual unlatching is required, manually unlatch the device"),
            TestStep("4g", "If manual unlatching is not required, send SetTarget command with Latch=False"),
            TestStep("4h", "Wait for CurrentState.Latch to be updated to False"),
        ]
        return steps

    def pics_TC_CLDIM_5_1(self) -> list[str]:
        pics = [
            "CLDIM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLDIM_5_1(self):
        endpoint = self.get_endpoint(default=1)

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureDimension.Attributes
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout

        # Default values
        min_position = 0
        max_position = 10000

        # STEP 2a: Read feature map and determine supported features
        self.step("2a")
        feature_map = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_positioning_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kPositioning
        is_latching_supported = feature_map & Clusters.ClosureDimension.Bitmaps.Feature.kMotionLatching

        # STEP 2b: Read AttributeList attribute
        self.step("2b")
        attribute_list = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        # STEP 2c: Read LimitRange attribute if supported
        self.step("2c")
        if attributes.LimitRange.attribute_id in attribute_list:
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            min_position = limit_range.min
            max_position = limit_range.max

        # STEP 2d: Read LatchControlModes attribute
        self.step("2d")
        latch_control_modes = 0b0  # Default value as a bitmap
        if attributes.LatchControlModes.attribute_id in attribute_list:
            latch_control_modes = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)

        # STEP 2d: Setup subscription to CurrentState attribute
        self.step("2e")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureDimension)
        await sub_handler.start(self.default_controller, self.dut.node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30)

        # STEP 3a: If Positioning feature is not supported, skip steps 3b to 3f
        self.step("3a")
        if not is_positioning_supported:
            logging.info("Positioning feature is not supported. Skipping steps 3b to 3f.")
            self.skip_step("3b")
            self.skip_step("3c")
            self.skip_step("3d")
            self.skip_step("3e")
            self.skip_step("3f")
        else:
            # STEP 3b: Read CurrentState attribute and save as ExpectedState
            self.step("3b")
            expected_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            # STEP 3c: If ExpectedState.Position != MinPosition, send SetTarget command with Position=MinPosition
            self.step("3c")
            if expected_state.position != min_position:
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=min_position),
                        endpoint=endpoint
                    )
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            else:
                logging.info("ExpectedState.Position is already at MinPosition. Skipping step.")
                self.mark_current_step_skipped()

            # STEP 3d: If ExpectedState.Position != MinPosition, wait for CurrentState.Position to be updated to MinPosition
            self.step("3d")
            if expected_state.position != min_position:
                expected_state.position = min_position
                expected_final_value = [AttributeValue(
                    endpoint, attribute=Clusters.ClosureDimension.Attributes.CurrentState, value=expected_state)]
                sub_handler.await_all_final_values_reported(expected_final_value, timeout_sec=timeout)
            else:
                logging.info("ExpectedState.Position is already at MinPosition. Skipping step.")
                self.mark_current_step_skipped()

            # STEP 3e: Send SetTarget command with Position=MaxPosition
            self.step("3e")
            sub_handler.reset()

            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(position=max_position),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

            # STEP 3f: Wait for CurrentState.Position to be updated to MaxPosition
            self.step("3f")
            expected_state.position = max_position
            expected_final_value = [AttributeValue(
                endpoint, attribute=Clusters.ClosureDimension.Attributes.CurrentState, value=expected_state)]
            sub_handler.await_all_final_values_reported(expected_final_value, timeout_sec=timeout)

        # STEP 4a: If Latching feature is not supported, skip steps 4b to 4g
        self.step("4a")
        if not is_latching_supported:
            logging.info("Latching feature is not supported. Skipping steps 4b to 4g.")
            self.skip_step("4b")
            self.skip_step("4c")
            self.skip_step("4d")
            self.skip_step("4e")
            self.skip_step("4f")
            self.skip_step("4g")
        else:
            # STEP 4b: Read CurrentState attribute and save as ExpectedState
            self.step("4b")
            expected_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
            sub_handler.reset()

            # STEP 4c: If manual latching is required, manually latch the device
            self.step("4c")
            if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlMode.kRemoteLatching:
                test_step = "Manually latch the device"
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when ready.")
            else:
                logging.info("Manual latching is not required. Skipping step.")
                self.mark_current_step_skipped()

            # STEP 4d: If manual latching is not required, send SetTarget command with Latch=True
            self.step("4d")
            if latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlMode.kRemoteLatching:
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=True),
                        endpoint=endpoint
                    )
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            else:
                logging.info("Manual latching is required. Skipping step.")
                self.mark_current_step_skipped()

            # STEP 4e: Wait for CurrentState.Latch to be updated to True
            self.step("4e")
            expected_state.latch = True
            expected_final_value = [AttributeValue(
                endpoint, attribute=Clusters.ClosureDimension.Attributes.CurrentState, value=expected_state)]
            sub_handler.await_all_final_values_reported(expected_final_value, timeout_sec=timeout)

            # STEP 4f: If manual unlatching is required, manually unlatch the device
            self.step("4f")
            sub_handler.reset()

            if not latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlMode.kRemoteUnlatching:
                test_step = "Manually unlatch the device"
                self.wait_for_user_input(prompt_msg=f"{test_step}, and press Enter when ready.")
            else:
                logging.info("Manual unlatching is not required. Skipping step.")
                self.mark_current_step_skipped()

            # STEP 4g: If manual unlatching is not required, send SetTarget command with Latch=False
            self.step("4g")

            if latch_control_modes & Clusters.ClosureDimension.Bitmaps.LatchControlMode.kRemoteUnlatching:
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(latch=False),
                        endpoint=endpoint
                    )
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            else:
                logging.info("Manual unlatching is required. Skipping step.")
                self.mark_current_step_skipped()

        # STEP 4h: Wait for CurrentState.Latch to be updated to False
        self.step("4h")
        expected_state.latch = False
        expected_final_value = [AttributeValue(
            endpoint, attribute=Clusters.ClosureDimension.Attributes.CurrentState, value=expected_state)]
        sub_handler.await_all_final_values_reported(expected_final_value, timeout_sec=timeout)


if __name__ == "__main__":
    default_matter_test_main()
