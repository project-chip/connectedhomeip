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
#      app: ${TYPE_OF_APP}
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
import time

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_CLDIM_3_3(MatterBaseTest):
    async def read_cldim_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureDimension
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLDIM_3_3(self) -> str:
        return "[TC-CLDIM-3.3] {C_SET_TARGET} Command Field Sanity Check with DUT as Server"

    def steps_TC_CLDIM_3_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "Read AttributeList attribute"),
            TestStep("2c", "Read LimitRange attribute"),
            TestStep("2d", "Read Resolution attribute"),
            TestStep(3, "Send SetTarget command with no fields"),
            TestStep("4a", "If Positioning feature is supported, skip step 4b to 4e"),
            TestStep("4b", "Send SetTarget command with Position MaxPosition"),
            TestStep("4c", "Send SetTarget command with Position below MinPosition"),
            TestStep("4d", "Send SetTarget command with Position above MaxPosition"),
            TestStep("4e", "Send SetTarget command with Position exceeding 100%"),
            TestStep("5a", "If LimitRange is unsupported, skip step 5b to 5i"),
            TestStep("5b", "Send SetTarget command with Position at MinPosition"),
            TestStep("5c", "Verify Target attribute is updated"),
            TestStep("5d", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("5e", "Verify CurrentState attribute is updated"),
            TestStep("5f", "Send SetTarget command with Position at MaxPosition"),
            TestStep("5g", "Verify Target attribute is updated"),
            TestStep("5h", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("5i", "Verify CurrentState attribute is updated"),
            TestStep(6, "Send SetTarget command with invalid Position"),
            TestStep("7a", "If Resolution is unsupported, skip step 7b to 7i"),
            TestStep("7b", "Send SetTarget command with Position not a multiple of Resolution"),
            TestStep("7c", "Verify Target attribute is updated"),
            TestStep("7d", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("7e", "Verify CurrentState attribute is updated"),
            TestStep("7f", "Send SetTarget command with Position not a multiple of Resolution"),
            TestStep("7g", "Verify Target attribute is updated"),
            TestStep("7h", "Wait for PIXIT.CLDIM.FullMotionDuration seconds"),
            TestStep("7i", "Verify CurrentState attribute is updated"),
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
        asserts.assert_true('PIXIT.CLDIM.FullMotionDuration' in self.matter_test_config.global_test_params,
                            "PIXIT.CLDIM.FullMotionDuration must be included on the command line in "
                            "the --int-arg flag as PIXIT.CLDIM.FullMotionDuration:<duration>")

        full_motion_duration = self.matter_test_config.global_test_params['PIXIT.CLDIM.FullMotionDuration']

        if full_motion_duration <= 0:
            asserts.fail("PIXIT.CLDIM.FullMotionDuration must be greater than 0")

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

        # STEP 2b: Read AttributeList attribute
        self.step("2b")
        attribute_list = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        # STEP 2c: Read LimitRange attribute if supported
        self.step("2c")
        if attributes.LimitRange.attribute_id in attribute_list:
            limit_range = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.LimitRange)
            min_position = limit_range.Min
            max_position = limit_range.Max

        # STEP 2d: Read Resolution attribute if supported
        self.step("2d")
        resolution = 1  # Default resolution
        if attributes.Resolution.attribute_id in attribute_list:
            resolution = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Resolution)

        # STEP 3: Send SetTarget command with no fields
        self.step(3)
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Unexpected status returned")

        # STEP 4a: If Positioning feature is supported, skip step 4b to 4e
        self.step("4a")
        if is_positioning_supported:
            logging.info("Positioning feature is supported. Skipping steps 4b to 4e.")
            self.skip_step("4b")
            self.skip_step("4c")
            self.skip_step("4d")
            self.skip_step("4e")

        # STEP 4b: Send SetTarget command with Position MaxPosition
        self.step("4b")
        
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=max_position),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 4c: Send SetTarget command with Position below MinPosition
        self.step("4c")
        if min_position > 0:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=min_position - 1),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
        else:
            logging.info("MinPosition not > 0. Skipping step 4c.")

        # STEP 4d: Send SetTarget command with Position above MaxPosition
        self.step("4d")
        if max_position < 10000:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=max_position + 1),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
        else:
            logging.info("MaxPosition not < 10000. Skipping step 4d.")

        # STEP 4e: Send SetTarget command with Position exceeding 100%
        self.step("4e")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=10001),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned")

        # STEP 5a: If LimitRange is unsupported, skip step 5b to 5i
        self.step("5a")
        if (not is_positioning_supported) or (attributes.LimitRange.attribute_id not in attribute_list):
            logging.info("Positioning Feature or LimitRange attribute is not supported. Skipping steps 5b to 5i.")
            self.skip_step("5b")
            self.skip_step("5c")
            self.skip_step("5d")
            self.skip_step("5e")
            self.skip_step("5f")
            self.skip_step("5g")
            self.skip_step("5h")
            self.skip_step("5i")

        # STEP 5b: Send SetTarget command with Position at MinPosition
        self.step("5b")
        if min_position > 0:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=0),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
        else:
            logging.info("MinPosition not > 0. Skipping step 5b.")

        # STEP 5c: Verify Target attribute is updated
        self.step("5c")
        if min_position > 0:
            if attributes.Target.attribute_id in attribute_list:
                target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
                asserts.assert_equal(target.Position, min_position, "Target Position does not match MinPosition")
        else:
            logging.info("MinPosition not > 0. Skipping step 5c.")

        # STEP 5d: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("5d")
        if min_position > 0:
            time.sleep(full_motion_duration)
        else:
            logging.info("MinPosition not > 0. Skipping step 5d.")

        # STEP 5e: Verify CurrentState attribute is updated
        self.step("5e")
        if min_position > 0:
            if attributes.CurrentState.attribute_id in attribute_list:
                current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
                asserts.assert_equal(current_state.Position, min_position, "CurrentState Position does not match MinPosition")
        else:
            logging.info("MinPosition not > 0. Skipping step 5e.")

        # STEP 5f: Send SetTarget command with Position at MaxPosition
        self.step("5f")
        if max_position < 10000:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=10000),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
        else:
            logging.info("MaxPosition not < 10000. Skipping step 5f.")

        # STEP 5g: Verify Target attribute is updated
        self.step("5g")
        if max_position < 10000:
            if attributes.Target.attribute_id in attribute_list:
                target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)
                asserts.assert_equal(target.Position, max_position, "Target Position does not match MaxPosition")
        else:
            logging.info("MaxPosition not < 10000. Skipping step 5g.")

        # STEP 5h: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("5h")
        if max_position < 10000:
            time.sleep(full_motion_duration)
        else:
            logging.info("MaxPosition not < 10000. Skipping step 5h.")

        # STEP 5i: Verify CurrentState attribute is updated
        self.step("5i")
        if max_position < 10000:
            if attributes.CurrentState.attribute_id in attribute_list:
                current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
                asserts.assert_equal(current_state.Position, max_position, "CurrentState Position does not match MaxPosition")
        else:
            logging.info("MaxPosition not < 10000. Skipping step 5i.")

        # STEP 6: Send SetTarget command with invalid Position
        self.step(6)
        if is_positioning_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=10001),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned")
        else: 
            logging.info("Positioning feature is not supported. Skipping step 6.")

        # STEP 7a: If Resolution is unsupported, skip step 7b to 7i
        self.step("7a")
        if (not is_positioning_supported) or (attributes.Resolution.attribute_id not in attribute_list):
            logging.info("Resolution attribute is not supported. Skipping steps 7b to 7i.")
            self.skip_step("7b")
            self.skip_step("7c")
            self.skip_step("7d")
            self.skip_step("7e")
            self.skip_step("7f")
            self.skip_step("7g")
            self.skip_step("7h")
            self.skip_step("7i")

        # STEP 7b: Send SetTarget command with Position not a multiple of Resolution
        self.step("7b")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=min_position + resolution - 1),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 7c: Verify Target attribute is updated
        self.step("7c")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)

            if resolution == 1:
                asserts.assert_equal(target.Position, min_position, "Target Position does not match expected value")
            else:
                asserts.assert_equal(target.Position, min_position + resolution, "Target Position does not match expected value")
        else:
            logging.info("Target attribute not supported. Skipping step 7c.")

        # STEP 7d: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("7d")
        time.sleep(full_motion_duration)

        # STEP 7e: Verify CurrentState attribute is updated
        self.step("7e")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            if resolution == 1:
                asserts.assert_equal(current_state.Position, min_position, "CurrentState Position does not match expected value")
            else:
                asserts.assert_equal(current_state.Position, min_position + resolution, "CurrentState Position does not match expected value")
        else:
            logging.info("CurrentState attribute not supported. Skipping step 7e.")

        # STEP 7f: Send SetTarget command with Position not a multiple of Resolution
        self.step("7f")
        try:
            await self.send_single_cmd(
                cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Position=(max_position - resolution) + 1),
                endpoint=endpoint
            )
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 7g: Verify Target attribute is updated
        self.step("7g")
        if attributes.Target.attribute_id in attribute_list:
            target = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.Target)

            if resolution <= 2:
                asserts.assert_equal(target.Position, max_position, "Target Position does not match expected value")
            else:
                asserts.assert_equal(target.Position, max_position - resolution, "Target Position does not match expected value")
        else:
            logging.info("Target attribute not supported. Skipping step 7g.")

        # STEP 7h: Wait for PIXIT.CLDIM.FullMotionDuration seconds
        self.step("7h")
        time.sleep(full_motion_duration)

        # STEP 7i: Verify CurrentState attribute is updated
        self.step("7i")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state = await self.read_cldim_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            if resolution <= 2:
                asserts.assert_equal(current_state.Position, max_position, "CurrentState Position does not match expected value")
            else:
                asserts.assert_equal(current_state.Position, max_position - resolution, "CurrentState Position does not match expected value")
        else:
            logging.info("CurrentState attribute not supported. Skipping step 7i.")

        # STEP 8: Send SetTarget command with Latch field when MotionLatching is unsupported
        self.step(8)
        if not is_latching_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Latch=True),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        # STEP 9: Send SetTarget command with Speed field when Speed is unsupported
        self.step(9)
        if not is_speed_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Speed=Clusters.ClosureDimension.Enums.ThreeLevelAutoEnum.kHigh),
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
        else:
            logging.info("Speed feature is supported. Skipping step 9.")

        # STEP 10: Send SetTarget command with invalid Speed when Speed is unsupported
        self.step(10)
        if not is_speed_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Speed=4),  # Invalid speed
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned")
        else:
            logging.info("Speed feature is supported. Skipping step 10.")

        # STEP 11: Send SetTarget command with invalid Speed
        self.step(11)
        if is_speed_supported:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Objects.ClosureDimension.Commands.SetTarget(Speed=4),  # Invalid speed
                    endpoint=endpoint
                )
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected status returned")
        else:
            logging.info("Speed feature is not supported. Skipping step 11.")

if __name__ == "__main__":
    default_matter_test_main()
