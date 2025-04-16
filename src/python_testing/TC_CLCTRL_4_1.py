#
#    Copyright (c) 2023 Project CHIP Authors
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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from random import choice

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class TC_CLCTRL_4_1(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_4_1(self) -> str:
        return "[TC_CLCTRL_4_1] MoveTo Command Position Functionality with DUT as Server"

    def steps_TC_CLCTRL_4_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", is_commissioning=True),
            TestStep("2a", "TH reads from the DUT the (0xFFFC) FeatureMap attribute."),
            TestStep("2b", "If the PS feature is not supported on the cluster, skip remaining steps and end test case."),
            TestStep("2c", "TH reads from the DUT the (0xFFFB) AttributeList attribute."),
            TestStep("3a", "TH sends command MoveTo with Position = Signature."),
            TestStep("3b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute."),
            TestStep("3c", "TH waits for PIXIT.CLCTRL.FullMotionDuration seconds."),
            TestStep("3d", "If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute."),
            TestStep("3e", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("4a", "If VT feature is not supported on the cluster, skip steps 4b to 4f."),
            TestStep("4b", "TH sends command MoveTo with Position = Ventilation."),
            TestStep("4c", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute."),
            TestStep("4d", "TH waits for PIXIT.CLCTRL.FullMotionDuration seconds."),
            TestStep("4e", "If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute."),
            TestStep("4f", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("5a", "If PD feature is not supported on the cluster, skip steps 5b to 5f."),
            TestStep("5b", "TH sends command MoveTo with Position = Pedestrian."),
            TestStep("5c", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute."),
            TestStep("5d", "TH waits for PIXIT.CLCTRL.FullMotionDuration seconds."),
            TestStep("5e", "If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute."),
            TestStep("5f", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("6a", "TH sends command MoveTo with Position = OpenInFull."),
            TestStep("6b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute."),
            TestStep("6c", "TH waits for PIXIT.CLCTRL.FullMotionDuration seconds."),
            TestStep("6d", "If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute."),
            TestStep("6e", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("7a", "TH sends command MoveTo with Position = CloseInFull."),
            TestStep("7b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute."),
            TestStep("7c", "TH waits for PIXIT.CLCTRL.FullMotionDuration seconds."),
            TestStep("7d", "If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute."),
            TestStep("7e", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("8a", "If SP feature is not supported on the cluster, skip steps 8b to 8g."),
            TestStep("8b", "TH sends command MoveTo with Speed = High."),
            TestStep("8c", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute."),
            TestStep("8d", "If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute."),
            TestStep("8e", "TH sends command MoveTo with Speed = Low."),
            TestStep("8f", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute."),
            TestStep("8g", "If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute."),
            TestStep("9a", "If PT feature is not supported on the cluster, skip steps 9b to 9f."),
            TestStep("9b", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Protected(5) Test Event."),
            TestStep("9c", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("9d", "TH sends command MoveTo with Position = CloseInFull."),
            TestStep("9e", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear."),
            TestStep("9f", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("10a", "If MO feature is not supported on the cluster, skip steps 10b to 10f."),
            TestStep("10b", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged(6) Test Event."),
            TestStep("10c", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("10d", "TH sends command MoveTo with Position = CloseInFull."),
            TestStep("10e", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear."),
            TestStep("10f", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("11a", "If IS feature is not supported on the cluster, skip steps 11b to 11k."),
            TestStep("11b", "TH sends command MoveTo with Position = OpenInFull."),
            TestStep("11c", "If attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute."),
            TestStep("11d", "TH waits for 2 seconds."),
            TestStep("11e", "If attribute is supported on the cluster, TH reads from the DUT the OverallState attribute."),
            TestStep("11f", "If attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("11g", "TH sends command MoveTo with Position = CloseInFull."),
            TestStep("11h", "If attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute."),
            TestStep("11i", "TH waits for 2 seconds."),
            TestStep("11j", "If attribute is supported on the cluster, TH reads from the DUT the OverallState attribute."),
            TestStep("11k", "If attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
        ]
        return steps

    def pics_TC_CLCTRL_4_1(self) -> list[str]:
        pics = [
            "CLCTRL.S"
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_4_1(self):
        endpoint = self.get_endpoint(default=1)

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureControl.Attributes

        # STEP 2a: TH reads from the DUT the (0xFFFC) FeatureMap attribute
        self.step("2a")

        feature_map = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        logging.info(f"FeatureMap: {feature_map}")
        is_ps_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPosition
        is_vt_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kVentilation
        is_pd_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPedestrian
        is_sp_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kSpeed
        is_pt_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kProtected
        is_mo_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotion
        is_is_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kIs
        is_lt_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kLatch

        # STEP 2b: If the PS feature is not supported on the cluster, skip remaining steps and end test case
        self.step("2b")

        if not is_ps_feature_supported:
            logging.info("Position feature not supported, skipping test case")

            # Skipping all remainig steps
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")

            return

        # STEP 2c: TH reads from the DUT the (0xFFFB) AttributeList attribute
        self.step("2c")

        attribute_list = await self.read_clctrl_attribute_expect_success(endpoint, attributes.AttributeList)
        logging.info(f"AttributeList: {attribute_list}")

        # STEP 3a: TH sends command MoveTo with Position = Signature
        self.step("3a")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kSignature,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 3b: If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute
        self.step("3b")

        if attributes.OverallTarget.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTarget)
            logging.info(f"OverallTarget: {overall_target}")
            asserts.assert_equal(overall_target.Position, Clusters.ClosureControl.Bitmaps.OverallTarget.kSignature,
                                 "OverallTarget.Position is not Signature")
            if is_lt_feature_supported:
                asserts.assert_false(overall_target.Latch, "OverallTarget.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_target.Speed, 0, 3, "OverallTarget.Speed is out of range")
        else:
            logging.info("OverallTarget attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 3c: TH waits for PIXIT.CLCTRL.FullMotionDuration seconds
        self.step("3c")

        full_motion_duration = self.matter_test_config.get("PIXIT.CLCTRL.FullMotionDuration", 0)
        if full_motion_duration is None:
            logging.error("PIXIT.CLCTRL.FullMotionDuration is not defined")
            return
        if full_motion_duration < 0:
            logging.error("PIXIT.CLCTRL.FullMotionDuration is negative")
            return
        if full_motion_duration > 0:
            logging.info(f"Waiting for {full_motion_duration} seconds")
            await self.wait(full_motion_duration)
        else:
            logging.info("No wait time specified, continuing without waiting")

        # STEP 3d: If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute
        self.step("3d")

        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallState)
            logging.info(f"OverallState: {overall_state}")
            asserts.assert_equal(overall_state.Position, Clusters.ClosureControl.Bitmaps.OverallState.kSignature,
                                 "OverallState.Position is not Signature")
            if is_lt_feature_supported:
                asserts.assert_false(overall_state.Latch, "OverallState.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_state.Speed, 0, 3, "OverallState.Speed is out of range")
            if overall_state.SecureState != NullValue:
                asserts.assert_false(overall_state.SecureState, "OverallState.SecureState is not False")
            else:
                logging.info("SecureState is NULL.")
        else:
            logging.info("OverallState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 3e: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("3e")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            is_stopped = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kStopped
            asserts.assert_true(is_stopped, "MainState is not in the expected state")
            if is_stopped:
                logging.info("MainState is in the stopped state")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 4a: If VT feature is not supported on the cluster, skip steps 4b to 4f
        self.step("4a")
        if not is_vt_feature_supported:
            logging.info("Ventilation feature not supported, skipping test case")

            # Skipping test steps 4b to 4f
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")
                if step.test_plan_number == "4f":
                    break
            return

        # STEP 4b: TH sends command MoveTo with Position = Ventilation
        self.step("4b")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kVentilation,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 4c: If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute
        self.step("4c")

        if attributes.OverallTarget.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTarget)
            logging.info(f"OverallTarget: {overall_target}")
            asserts.assert_equal(overall_target.Position, Clusters.ClosureControl.Bitmaps.OverallTarget.kVentilation,
                                 "OverallTarget.Position is not Ventilation")
            if is_lt_feature_supported:
                asserts.assert_false(overall_target.Latch, "OverallTarget.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_target.Speed, 0, 3, "OverallTarget.Speed is out of range")
        else:
            logging.info("OverallTarget attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 4d: TH waits for PIXIT.CLCTRL.FullMotionDuration seconds
        self.step("4d")

        if full_motion_duration is None:
            logging.error("PIXIT.CLCTRL.FullMotionDuration is not defined")
            return
        if full_motion_duration < 0:
            logging.error("PIXIT.CLCTRL.FullMotionDuration is negative")
            return
        if full_motion_duration > 0:
            logging.info(f"Waiting for {full_motion_duration} seconds")
            await self.wait(full_motion_duration)
        else:
            logging.info("No wait time specified, continuing without waiting")

        # STEP 4e: If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute
        self.step("4e")

        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallState)
            logging.info(f"OverallState: {overall_state}")
            asserts.assert_equal(overall_state.Position, Clusters.ClosureControl.Bitmaps.OverallState.kVentilation,
                                 "OverallState.Position is not Ventilation")
            if is_lt_feature_supported:
                asserts.assert_false(overall_state.Latch, "OverallState.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_state.Speed, 0, 3, "OverallState.Speed is out of range")
            if overall_state.SecureState != NullValue:
                asserts.assert_false(overall_state.SecureState, "OverallState.SecureState is not False")
            else:
                logging.info("SecureState is NULL.")
        else:
            logging.info("OverallState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 4f: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("4f")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            is_stopped = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kStopped
            asserts.assert_true(is_stopped, "MainState is not in the expected state")
            if is_stopped:
                logging.info("MainState is in the stopped state")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 5a: If PD feature is not supported on the cluster, skip steps 5b to 5f
        self.step("5a")

        if not is_pd_feature_supported:
            logging.info("Pedestrian feature not supported, skipping test case")

            # Skipping test steps 5b to 5f
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")
                if step.test_plan_number == "5f":
                    break
            return

        # STEP 5b: TH sends command MoveTo with Position = Pedestrian
        self.step("5b")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kPedestrian,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 5c: If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute
        self.step("5c")

        if attributes.OverallTarget.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTarget)
            logging.info(f"OverallTarget: {overall_target}")
            asserts.assert_equal(overall_target.Position, Clusters.ClosureControl.Bitmaps.OverallTarget.kPedestrian,
                                 "OverallTarget.Position is not Pedestrian")
            if is_lt_feature_supported:
                asserts.assert_false(overall_target.Latch, "OverallTarget.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_target.Speed, 0, 3, "OverallTarget.Speed is out of range")
        else:
            logging.info("OverallTarget attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 5d: TH waits for PIXIT.CLCTRL.FullMotionDuration seconds
        self.step("5d")

        if full_motion_duration is None:
            logging.error("PIXIT.CLCTRL.FullMotionDuration is not defined")
            return
        if full_motion_duration < 0:
            logging.error("PIXIT.CLCTRL.FullMotionDuration is negative")
            return
        if full_motion_duration > 0:
            logging.info(f"Waiting for {full_motion_duration} seconds")
            await self.wait(full_motion_duration)
        else:
            logging.info("No wait time specified, continuing without waiting")

        # STEP 5e: If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute
        self.step("5e")

        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallState)
            logging.info(f"OverallState: {overall_state}")
            asserts.assert_equal(overall_state.Position, Clusters.ClosureControl.Bitmaps.OverallState.kPedestrian,
                                 "OverallState.Position is not Pedestrian")
            if is_lt_feature_supported:
                asserts.assert_false(overall_state.Latch, "OverallState.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_state.Speed, 0, 3, "OverallState.Speed is out of range")
            if overall_state.SecureState != NullValue:
                asserts.assert_false(overall_state.SecureState, "OverallState.SecureState is not False")
            else:
                logging.info("SecureState is NULL.")
        else:
            logging.info("OverallState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 5f: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("5f")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            is_stopped = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kStopped
            asserts.assert_true(is_stopped, "MainState is not in the expected state")
            if is_stopped:
                logging.info("MainState is in the stopped state")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 6a: TH sends command MoveTo with Position = OpenInFull
        self.step("6a")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kOpenInFull,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 6b: If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute
        self.step("6b")

        if attributes.OverallTarget.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTarget)
            logging.info(f"OverallTarget: {overall_target}")
            asserts.assert_equal(overall_target.Position, Clusters.ClosureControl.Bitmaps.OverallTarget.kOpenInFull,
                                 "OverallTarget.Position is not OpenInFull")
            if is_lt_feature_supported:
                asserts.assert_false(overall_target.Latch, "OverallTarget.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_target.Speed, 0, 3, "OverallTarget.Speed is out of range")
        else:
            logging.info("OverallTarget attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 6c: TH waits for PIXIT.CLCTRL.FullMotionDuration seconds
        self.step("6c")

        if full_motion_duration is None:
            logging.error("PIXIT.CLCTRL.FullMotionDuration is not defined")
            return
        if full_motion_duration < 0:
            logging.error("PIXIT.CLCTRL.FullMotionDuration is negative")
            return
        if full_motion_duration > 0:
            logging.info(f"Waiting for {full_motion_duration} seconds")
            await self.wait(full_motion_duration)
        else:
            logging.info("No wait time specified, continuing without waiting")

        # STEP 6d: If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute
        self.step("6d")

        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallState)
            logging.info(f"OverallState: {overall_state}")
            asserts.assert_equal(overall_state.Position, Clusters.ClosureControl.Bitmaps.OverallState.kOpenInFull,
                                 "OverallState.Position is not OpenInFull")
            if is_lt_feature_supported:
                asserts.assert_false(overall_state.Latch, "OverallState.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_state.Speed, 0, 3, "OverallState.Speed is out of range")
            if overall_state.SecureState != NullValue:
                asserts.assert_false(overall_state.SecureState, "OverallState.SecureState is not False")
            else:
                logging.info("SecureState is NULL.")
        else:
            logging.info("OverallState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 6e: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("6e")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            is_stopped = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kStopped
            asserts.assert_true(is_stopped, "MainState is not in the expected state")
            if is_stopped:
                logging.info("MainState is in the stopped state")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 7a: TH sends command MoveTo with Position = CloseInFull
        self.step("7a")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 7b: If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute
        self.step("7b")

        if attributes.OverallTarget.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTarget)
            logging.info(f"OverallTarget: {overall_target}")
            asserts.assert_equal(overall_target.Position, Clusters.ClosureControl.Bitmaps.OverallTarget.kCloseInFull,
                                 "OverallTarget.Position is not CloseInFull")
            if is_lt_feature_supported:
                asserts.assert_false(overall_target.Latch, "OverallTarget.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_target.Speed, 0, 3, "OverallTarget.Speed is out of range")
        else:
            logging.info("OverallTarget attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 7c: TH waits for PIXIT.CLCTRL.FullMotionDuration seconds
        self.step("7c")

        if full_motion_duration is None:
            logging.error("PIXIT.CLCTRL.FullMotionDuration is not defined")
            return
        if full_motion_duration < 0:
            logging.error("PIXIT.CLCTRL.FullMotionDuration is negative")
            return
        if full_motion_duration > 0:
            logging.info(f"Waiting for {full_motion_duration} seconds")
            await self.wait(full_motion_duration)
        else:
            logging.info("No wait time specified, continuing without waiting")

        # STEP 7d: If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute
        self.step("7d")

        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallState)
            logging.info(f"OverallState: {overall_state}")
            asserts.assert_equal(overall_state.Position, Clusters.ClosureControl.Bitmaps.OverallState.kCloseInFull,
                                 "OverallState.Position is not CloseInFull")
            if is_lt_feature_supported:
                asserts.assert_false(overall_state.Latch, "OverallState.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_state.Speed, 0, 3, "OverallState.Speed is out of range")
            if overall_state.SecureState != NullValue:
                asserts.assert_true(overall_state.SecureState, "OverallState.SecureState is not True")
            else:
                logging.info("SecureState is NULL.")
        else:
            logging.info("OverallState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 7e: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("7e")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            is_stopped = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kStopped
            asserts.assert_true(is_stopped, "MainState is not in the expected state")
            if is_stopped:
                logging.info("MainState is in the stopped state")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 8a: If SP feature is not supported on the cluster, skip steps 8b to 8g
        self.step("8a")

        if not is_sp_feature_supported:
            logging.info("Speed feature not supported, skipping test case")

            # Skipping test steps 8b to 8g
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")
                if step.test_plan_number == "8g":
                    break
            return

        # STEP 8b: TH sends command MoveTo with Speed = High
        self.step("8b")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                speed=Clusters.ClosureControl.Bitmaps.Speed.kHigh,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 8c: If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute
        self.step("8c")

        if attributes.OverallTarget.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTarget)
            logging.info(f"OverallTarget: {overall_target}")
            asserts.assert_equal(overall_target.Position, Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
                                 "OverallTarget.Position is not CloseInFull")
            asserts.assert_equal(overall_target.Speed, Clusters.ClosureControl.Bitmaps.Speed.kHigh,
                                 "OverallTarget.Speed is not High")
            if is_lt_feature_supported:
                asserts.assert_false(overall_target.Latch, "OverallTarget.Latch is not False")
        else:
            logging.info("OverallTarget attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 8d: If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute
        self.step("8d")

        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallState)
            logging.info(f"OverallState: {overall_state}")
            asserts.assert_equal(overall_state.Position, Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
                                 "OverallState.Position is not CloseInFull")
            asserts.assert_equal(overall_state.Speed, Clusters.ClosureControl.Bitmaps.Speed.kHigh, "OverallState.Speed is not High")
            if is_lt_feature_supported:
                asserts.assert_false(overall_state.Latch, "OverallState.Latch is not False")
            if overall_state.SecureState != NullValue:
                asserts.assert_true(overall_state.SecureState, "OverallState.SecureState is not True")
            else:
                logging.info("SecureState is NULL.")
        else:
            logging.info("OverallState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 8e: TH sends command MoveTo with Speed = Low
        self.step("8e")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                speed=Clusters.ClosureControl.Bitmaps.Speed.kLow,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 8f: If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute
        self.step("8f")

        if attributes.OverallTarget.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTarget)
            logging.info(f"OverallTarget: {overall_target}")
            asserts.assert_equal(overall_target.Position, Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
                                 "OverallTarget.Position is not CloseInFull")
            asserts.assert_equal(overall_target.Speed, Clusters.ClosureControl.Bitmaps.Speed.kLow, "OverallTarget.Speed is not Low")
            if is_lt_feature_supported:
                asserts.assert_false(overall_target.Latch, "OverallTarget.Latch is not False")
        else:
            logging.info("OverallTarget attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 8g: If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute
        self.step("8g")

        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallState)
            logging.info(f"OverallState: {overall_state}")
            asserts.assert_equal(overall_state.Position, Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
                                 "OverallState.Position is not CloseInFull")
            asserts.assert_equal(overall_state.Speed, Clusters.ClosureControl.Bitmaps.Speed.kLow, "OverallState.Speed is not Low")
            if is_lt_feature_supported:
                asserts.assert_false(overall_state.Latch, "OverallState.Latch is not False")
            if overall_state.SecureState != NullValue:
                asserts.assert_true(overall_state.SecureState, "OverallState.SecureState is not True")
            else:
                logging.info("SecureState is NULL.")
        else:
            logging.info("OverallState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 9a: If PT feature is not supported on the cluster, skip steps 9b to 9f
        self.step("9a")

        if not is_pt_feature_supported:
            logging.info("Position feature not supported, skipping test case")

            # Skipping test steps 9b to 9f
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")
                if step.test_plan_number == "9f":
                    break
            return

        # STEP 9b: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Protected(5) Test Event
        self.step("9b")

        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostics, command=Clusters.GeneralDiagnostics.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostics.TestEventTriggerRequest(
                enabled_key=self.matter_test_config.get("PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY"),
                event_trigger=self.matter_test_config.get("PIXIT.CLCTRL.TEST_EVENT_TRIGGER"),
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command TestEventTrigger: {e.status}")
            return
        else:
            logging.info("Command TestEventTrigger sent successfully")

        # STEP 9c: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("9c")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            asserts.assert_equal(main_state, Clusters.ClosureControl.Bitmaps.MainState.kProtected, "MainState is not Protected")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 9d: TH sends command MoveTo with Position = CloseInFull
        self.step("9d")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent invalid state
        if e.status == Status.INVALID_STATE:
            logging.info("Command MoveTo is invalid while MainState is Protected")
        else:
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return

        # STEP 9e: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear
        self.step("9e")

        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostics, command=Clusters.GeneralDiagnostics.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostics.TestEventTriggerRequest(
                enabled_key=self.matter_test_config.get("PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY"),
                event_trigger=self.matter_test_config.get("PIXIT.CLCTRL.TEST_EVENT_TRIGGER"),
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command TestEventTrigger: {e.status}")
            return
        else:
            logging.info("Command TestEventTrigger sent successfully")

        # STEP 9f: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("9f")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            is_stopped = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kStopped
            asserts.assert_true(is_stopped, "MainState is not in the expected state")
            if is_stopped:
                logging.info("MainState is in the stopped state")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 10a: If MO feature is not supported on the cluster, skip steps 10b to 10f
        self.step("10a")

        if not is_mo_feature_supported:
            logging.info("Motor Open feature not supported, skipping test case")

            # Skipping test steps 10b to 10f
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")
                if step.test_plan_number == "10f":
                    break
            return

        # STEP 10b: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged(6) Test Event
        self.step("10b")

        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostics, command=Clusters.GeneralDiagnostics.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostics.TestEventTriggerRequest(
                enabled_key=self.matter_test_config.get("PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY"),
                event_trigger=self.matter_test_config.get("PIXIT.CLCTRL.TEST_EVENT_TRIGGER"),
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command TestEventTrigger: {e.status}")
            return
        else:
            logging.info("Command TestEventTrigger sent successfully")

        # STEP 10c: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("10c")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            asserts.assert_equal(main_state, Clusters.ClosureControl.Bitmaps.MainState.kDisengaged, "MainState is not Disengaged")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 10d: TH sends command MoveTo with Position = CloseInFull
        self.step("10d")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent invalid state
        if e.status == Status.INVALID_STATE:
            logging.info("Command MoveTo is invalid while MainState is Disengaged")
        else:
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return

        # STEP 10e: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear
        self.step("10e")

        try:
            await self.send_command(endpoint=0, cluster=Clusters.Objects.GeneralDiagnostics, command=Clusters.GeneralDiagnostics.Commands.TestEventTrigger, arguments=Clusters.GeneralDiagnostics.TestEventTriggerRequest(
                enabled_key=self.matter_test_config.get("PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY"),
                event_trigger=self.matter_test_config.get("PIXIT.CLCTRL.TEST_EVENT_TRIGGER"),
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command TestEventTrigger: {e.status}")
            return
        else:
            logging.info("Command TestEventTrigger sent successfully")

        # STEP 10f: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("10f")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            is_stopped = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kStopped
            asserts.assert_true(is_stopped, "MainState is not in the expected state")
            if is_stopped:
                logging.info("MainState is in the stopped state")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 11a: If IS feature is not supported on the cluster, skip steps 11b to 11k
        self.step("11a")

        if not is_is_feature_supported:
            logging.info("Instantaneous feature not supported, skipping test case")

            # Skipping test steps 11b to 11k
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")
                if step.test_plan_number == "11k":
                    break
            return

        # STEP 11b: TH sends command MoveTo with Position = OpenInFull
        self.step("11b")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kOpenInFull,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 11c: If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute
        self.step("11c")

        if attributes.OverallTarget.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTarget)
            logging.info(f"OverallTarget: {overall_target}")
            asserts.assert_equal(overall_target.Position, Clusters.ClosureControl.Bitmaps.Position.kOpenInFull,
                                 "OverallTarget.Position is not OpenInFull")
            if is_lt_feature_supported:
                asserts.assert_false(overall_target.Latch, "OverallTarget.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_target.Speed, 0, 3, "OverallTarget.Speed is out of range")
        else:
            logging.info("OverallTarget attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 11d: TH waits for 2 seconds
        self.step("11d")

        sleep(2)
        logging.info("Waiting for 2 seconds")

        # STEP 11e: If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute
        self.step("11e")

        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallState)
            logging.info(f"OverallState: {overall_state}")
            asserts.assert_equal(overall_state.Position, Clusters.ClosureControl.Bitmaps.OverallState.kOpenInFull,
                                 "OverallState.Position is not OpenInFull")
            if is_lt_feature_supported:
                asserts.assert_false(overall_state.Latch, "OverallState.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_state.Speed, 0, 3, "OverallState.Speed is out of range")
            if overall_state.SecureState != NullValue:
                asserts.assert_false(overall_state.SecureState, "OverallState.SecureState is not False")
            else:
                logging.info("SecureState is NULL.")
        else:
            logging.info("OverallState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 11f: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("11f")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            is_stopped = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kStopped
            asserts.assert_true(is_stopped, "MainState is not in the expected state")
            if is_stopped:
                logging.info("MainState is in the stopped state")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 11g: TH sends command MoveTo with Position = CloseInFull
        self.step("11g")

        try:
            await self.send_command(endpoint=endpoint, cluster=Clusters.Objects.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo, arguments=Clusters.ClosureControl.MoveToRequest(
                position=Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
            ))
        except InteractionModelError as e:
            if e.status == Status.UNSUPPORTED_CLUSTER:
                logging.info("Test step skipped")
                return
            else:
                raise
        # Check if the command was sent successfully
        if not type_matches(e.status, Status.SUCCESS):
            logging.error(f"Failed to send command MoveTo: {e.status}")
            return
        else:
            logging.info("Command MoveTo sent successfully")

        # STEP 11h: If the attribute is supported on the cluster, TH reads from the DUT the OverallTaget attribute
        self.step("11h")

        if attributes.OverallTarget.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTarget)
            logging.info(f"OverallTarget: {overall_target}")
            asserts.assert_equal(overall_target.Position, Clusters.ClosureControl.Bitmaps.Position.kCloseInFull,
                                 "OverallTarget.Position is not CloseInFull")
            if is_lt_feature_supported:
                asserts.assert_false(overall_target.Latch, "OverallTarget.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_target.Speed, 0, 3, "OverallTarget.Speed is out of range")
        else:
            logging.info("OverallTarget attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 11i: TH waits for 2 seconds
        self.step("11i")

        sleep(2)
        logging.info("Waiting for 2 seconds")

        # STEP 11j: If the attribute is supported on the cluster, TH reads from the DUT the OverallState attribute
        self.step("11j")

        if attributes.OverallState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallState)
            logging.info(f"OverallState: {overall_state}")
            asserts.assert_equal(overall_state.Position, Clusters.ClosureControl.Bitmaps.OverallState.kCloseInFull,
                                 "OverallState.Position is not CloseInFull")
            if is_lt_feature_supported:
                asserts.assert_false(overall_state.Latch, "OverallState.Latch is not False")
            if is_sp_feature_supported:
                asserts.assert_range(overall_state.Speed, 0, 3, "OverallState.Speed is out of range")
            if overall_state.SecureState != NullValue:
                asserts.assert_true(overall_state.SecureState, "OverallState.SecureState is not True")
            else:
                logging.info("SecureState is NULL.")
        else:
            logging.info("OverallState attribute is not supported on the cluster, skipping this step.")
            return

        # STEP 11k: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("11k")

        if attributes.MainState.attribute_id in attribute_list:
            main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
            logging.info(f"MainState: {main_state}")
            is_stopped = mainstate == Clusters.ClosureControl.Bitmaps.MainState.kStopped
            asserts.assert_true(is_stopped, "MainState is not in the expected state")
            if is_stopped:
                logging.info("MainState is in the stopped state")
        else:
            logging.info("MainState attribute is not supported on the cluster, skipping this step.")
            return


if __name__ == "__main__":
    default_matter_test_main()
