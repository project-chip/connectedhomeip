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
#     app: ${CLOSURE_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --timeout 30
#       --endpoint 1
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import (AttributeMatcher, AttributeValue, ClusterAttributeChangeAccumulator, MatterBaseTest,
                                         TestStep, async_test_body, default_matter_test_main)
from mobly import asserts


def main_state_matcher(main_state: Clusters.ClosureControl.Enums.MainStateEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.MainState:
            return False
        if report.value == main_state:
            return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"MainState is {main_state}", matcher=predicate)


def current_position_matcher(current_position: Clusters.ClosureControl.Enums.CurrentPositionEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False
        if report.value.position == current_position:
            return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Position is {current_position}", matcher=predicate)


def current_latch_matcher(current_latch: bool) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False
        if report.value.latch == current_latch:
            return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Latch is {current_latch}", matcher=predicate)


class TC_CLCTRL_5_1(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_5_1(self) -> str:
        return "[TC_CLCTRL_5_1] Stop Command Primary Functionality with DUT as Server"

    def steps_TC_CLCTRL_5_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", is_commissioning=True),
            TestStep("2a", "TH reads from the DUT the (0xFFFC) FeatureMap attribute."),
            TestStep("2b", "If the IS feature is supported on the cluster, skip remaining steps and end test case."),
            TestStep("2c", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster."),
            TestStep("2d", "TH reads from the DUT the (0xFFFB) AttributeList attribute."),
            TestStep("2e", "TH establishes a wildcard subscription to all attributes on the Closure Control Cluster, with MinIntervalFloor = 0, MaxIntervalCeiling = 30 and KeepSubscriptions = false."),
            TestStep("3a", "If the LT feature is not supported on the cluster, skip steps 3a to 3j."),
            TestStep("3b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState.Latch attribute."),
            TestStep("3c", "If the attribute is supported on the cluster, TH reads from the DUT the LatchControlModes attribute."),
            TestStep("3d", "If CurrentLatch = True, skip steps 3e to 3j."),
            TestStep("3e", "If LatchControlModes Bit 0 = 0 (RemoteLatching = False), skip step 3f."),
            TestStep("3f", "TH sends command MoveTo with Latch = True."),
            TestStep("3g", "If LatchControlModes Bit 0 = 1 (RemoteLatching = True), skip step 3h."),
            TestStep("3h", "Latch the DUT manually to set OverallCurrentState.Latch to True."),
            TestStep("3i", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute."),
            TestStep("3j", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("4a", "If the PS feature is supported on the cluster or the LT feature is not supported on the cluster, skip steps 4b to 4g."),
            TestStep("4b", "If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip steps 4c to 4d."),
            TestStep("4c", "TH sends command MoveTo with Latch = False."),
            TestStep("4d", "TH sends command Stop to DUT."),
            TestStep("4e", "If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 4f."),
            TestStep("4f", "Unlatch the DUT manually to set OverallCurrentState.Latch to False."),
            TestStep("4g", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState.Latch attribute."),
            TestStep("5a", "If the PS feature is not supported on the cluster, skip steps 5b to 5e."),
            TestStep("5b", "If the attribute is supported on the cluster. TH reads from the DUT the OverallCurrentState attribute."),
            TestStep("5c", "If CurrentPosition is FullyClosed, skip steps 5d and 5e."),
            TestStep("5d", "TH sends command MoveTo with Position = MoveToFullyClosed."),
            TestStep("5e", "Wait until TH receives a subscription report with OverallCurrentState.Position = FullyClosed."),
            TestStep("6a", "If the PS feature is not supported on the cluster, skip steps 6b to 6e"),
            TestStep("6b", "TH sends command MoveTo to DUT, with Position = MoveToFullyOpen"),
            TestStep("6c", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute"),
            TestStep("6d", "TH sends command Stop to DUT"),
            TestStep("6e", "Wait until TH receives a subscription report with MainState = Stopped."),
            TestStep("7a", "If the CL feature is not supported on the cluster, skip steps 7b to 7e"),
            TestStep("7b", "TH sends command Calibrate to DUT"),
            TestStep("7c", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute"),
            TestStep("7d", "TH sends command Stop to DUT"),
            TestStep("7e", "Wait until TH receives a subscription report with MainState = Stopped."),
            TestStep("8a", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is SetupRequired Test Event"),
            TestStep("8b", "TH reads from the DUT the MainState attribute"),
            TestStep("8c", "TH sends command Stop to DUT"),
            TestStep("8d", "TH reads from the DUT the MainState attribute"),
            TestStep("9a", "If the PT feature is not supported on the cluster, skip steps 9b to 9e"),
            TestStep("9b", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Protected Test Event"),
            TestStep("9c", "TH reads from the DUT the MainState attribute"),
            TestStep("9d", "TH sends command Stop to DUT"),
            TestStep("9e", "TH reads from the DUT the MainState attribute"),
            TestStep("10a", "If the MO feature is not supported on the cluster, skip steps 10b to 10e"),
            TestStep("10b", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged Test Event"),
            TestStep("10c", "TH reads from the DUT the MainState attribute"),
            TestStep("10d", "TH sends command Stop to DUT"),
            TestStep("10e", "TH reads from the DUT the MainState attribute"),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear"),
        ]
        return steps

    def pics_TC_CLCTRL_5_1(self) -> list[str]:
        pics = [
            "CLCTRL.S"
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_5_1(self):
        endpoint = self.get_endpoint(default=1)
        dev_controller = self.default_controller
        attributes = Clusters.ClosureControl.Attributes
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureControl.Attributes

        # STEP 2a: TH reads from the DUT the (0xFFFC) FeatureMap attribute.
        self.step("2a")

        feature_map = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        logging.info(f"FeatureMap: {feature_map}")
        is_is_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kInstantaneous
        is_ps_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPositioning
        is_cl_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kCalibration
        is_lt_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching
        is_pt_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kProtection
        is_mo_feature_supported = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kManuallyOperable

        # STEP 2b: If the IS feature is supported on the cluster, skip remaining steps and end test case.
        self.step("2b")

        if is_is_feature_supported:
            logging.info("Instantanous feature is supported, skipping test case")

            # Skipping all remainig steps
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")

            return

        # STEP 2c: TH reads TestEventTrigger attribute from the General Diagnostic Cluster
        self.step("2c")

        logging.info("Checking if TestEventTrigger attribute is enabled")
        self.check_test_event_triggers_enabled()

        # STEP 2d: TH reads from the DUT the (0xFFFB) AttributeList attribute
        self.step("2d")

        attribute_list = await self.read_clctrl_attribute_expect_success(endpoint, attributes.AttributeList)
        logging.info(f"AttributeList: {attribute_list}")

        # STEP 2e: TH establishes a wildcard subscription to all attributes on the Closure Control Cluster, with MinIntervalFloor = 0, MaxIntervalCeiling = 30 and KeepSubscriptions = false
        self.step("2e")

        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureControl)
        await sub_handler.start(dev_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        # STEP 3a:If the LT feature is not supported on the cluster, skip steps 3a to 3j.
        self.step("3a")

        if is_lt_feature_supported:
            logging.info("Motion Latching feature supported.")

            # STEP 3b: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
            self.step("3b")

            if attributes.OverallCurrentState.attribute_id in attribute_list:
                overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                logging.info(f"OverallCurrentState: {overall_current_state}")
                if overall_current_state is None:
                    logging.error("OverallCurrentState is None")

                CurrentLatch = overall_current_state.latch
                asserts.assert_in(CurrentLatch, [True, False], "OverallCurrentState.latch is not in the expected range")
            else:
                asserts.assert_true(False, "OverallCurrentState attribute is not supported.")
                return

            # STEP 3c: If the attribute is supported on the cluster, TH reads from the DUT the LatchControlModes attribute
            self.step("3c")

            if attributes.LatchControlModes.attribute_id in attribute_list:
                LatchControlModes = await self.read_clctrl_attribute_expect_success(endpoint, attributes.LatchControlModes)
                logging.info(f"LatchControlModes: {LatchControlModes}")
                if LatchControlModes is None:
                    logging.error("LatchControlModes is None")
            else:
                asserts.assert_true(False, "LatchControlModes attribute is not supported.")
                return

            # STEP 3d: If CurrentLatch = True, skip steps 3e to 3i
            self.step("3d")

            if CurrentLatch:
                logging.info("CurrentLatch is True, skipping steps 3e to 3i")

                # Skipping steps 3e to 3j
                self.skip_step("3e")
                self.skip_step("3f")
                self.skip_step("3g")
                self.skip_step("3h")
                self.skip_step("3i")
            else:
                logging.info("CurrentLatch is False, proceeding to steps 3e to 3i")

                # STEP 3e: If LatchControlModes Bit 0 = 0 (RemoteLatching = False), skip step 3o
                self.step("3e")
                if (int(bin(LatchControlModes), 2) & 1) == 1:
                    logging.info("RemoteLatching is True, proceeding to step 3f")

                    # STEP 3f: TH sends command MoveTo with Latch = True
                    self.step("3f")

                    try:
                        await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                            latch=True
                        ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                    except InteractionModelError as e:
                        asserts.assert_equal(
                            e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                        pass
                    self.step("3g")
                    self.skip_step("3h")
                elif (int(bin(LatchControlModes), 2) & 1) == 0:
                    self.skip_step("3f")
                    # STEP 3g: If LatchControlModes Bit 0 = 1 (RemoteLatching = True), skip step 3h
                    self.step("3g")

                    logging.info("RemoteLatching is False, proceeding to step 3h")

                    # STEP 3h: Latch the DUT manually to set OverallCurrentState.Latch to True
                    self.step("3h")

                    logging.info("Latch the DUT manually to set OverallCurrentState.Latch to True")
                    # Simulating manual latching by waiting for user input
                    input("Press Enter after latching the DUT...")
                    logging.info("Manual latching completed.")

                # STEP 3i: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
                self.step("3i")

                if attributes.OverallCurrentState.attribute_id in attribute_list:
                    overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                    logging.info(f"OverallCurrentState: {overall_current_state}")
                    if overall_current_state is None:
                        logging.error("OverallCurrentState is None")

                    asserts.assert_true(overall_current_state.latch, "OverallCurrentState.latch is not True")
                else:
                    asserts.assert_true(False, "OverallCurrentState attribute is not supported.")
                    return

            # STEP 3j: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
            self.step("3j")

            if attributes.MainState.attribute_id in attribute_list:
                main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
                logging.info(f"MainState: {main_state}")
                asserts.assert_true(main_state == Clusters.ClosureControl.Enums.MainStateEnum.kStopped,
                                    "MainState is not in the expected state")
            else:
                asserts.assert_true(False, "MainState attribute is not supported.")
                return

        else:
            logging.info("Motion Latching feature not supported, skipping steps 3b to 3j")

            # Skipping steps 3b to 3j
            self.skip_step("3b")
            self.skip_step("3c")
            self.skip_step("3d")
            self.skip_step("3e")
            self.skip_step("3f")
            self.skip_step("3g")
            self.skip_step("3h")
            self.skip_step("3i")
            self.skip_step("3j")
        # STEP 4a: If the PS feature is supported on the cluster or the LT feature is not supported on the cluster, skip steps 4b to 4g.
        self.step("4a")

        if is_ps_feature_supported or not is_lt_feature_supported:
            logging.info("Motion Latching feature not supported or Positioning feature supported, skipping steps 4b to 4g")

            # Skipping steps 4b to 4g
            self.skip_step("4b")
            self.skip_step("4c")
            self.skip_step("4d")
            self.skip_step("4e")
            self.skip_step("4f")
            self.skip_step("4g")
        else:

            # STEP 4b: If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 5b
            self.step("4b")

            if (int(bin(LatchControlModes), 2) & (1 << 1)) == 2:
                logging.info("RemoteUnlatching is True, proceeding steps 4c to 4d")

                # STEP 4c: TH sends command MoveTo with Latch = False
                self.step("4c")

                try:
                    await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                        latch=False
                    ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                    pass

                # STEP 4d: TH sends command Stop to DUT.
                self.step("4d")

                try:
                    await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Stop(), endpoint=endpoint, timedRequestTimeoutMs=1000)
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, f"Failed to send command Stop: {e.status}")
                    pass
                self.step("4e")
                self.skip_step("4f")
            elif (int(bin(LatchControlModes), 2) & (1 << 1)) == 0:
                self.skip_step("4c")
                self.skip_step("4d")

                # STEP 4e: If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 4f
                self.step("4e")

                logging.info("RemoteUnlatching is False, proceeding to step 4f")

                # STEP 4f: Unlatch the DUT manually to set OverallCurrentState.Latch to False
                self.step("4f")

                logging.info("Unlatch the DUT manually to set OverallCurrentState.Latch to False")
                # Simulating manual unlatching by waiting for user input
                input("Press Enter after unlatching the DUT...")
                logging.info("Manual unlatching completed.")

            # STEP 4g: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
                self.step("4g")

                if attributes.OverallCurrentState.attribute_id in attribute_list:
                    overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                    logging.info(f"OverallCurrentState: {overall_current_state}")
                    if overall_current_state is None:
                        logging.error("OverallCurrentState is None")

                    asserts.assert_false(overall_current_state.latch, "OverallCurrentState.latch is not False")
                else:
                    asserts.assert_true(False, "OverallCurrentState attribute is not supported.")
                    return

        # STEP 5a: If the PS feature is not supported on the cluster, skip steps 5b to 5e.
        self.step("5a")

        if not is_ps_feature_supported:
            logging.info("Positioning Latching feature not supported, skipping steps 5b to 5e")

            # Skipping steps 5b to 5e
            self.skip_step("5b")
            self.skip_step("5c")
            self.skip_step("5d")
            self.skip_step("5e")
        else:
            # STEP 5b: If the attribute is supported on the cluster. TH reads from the DUT the OverallCurrentState attribute.
            self.step("5b")

            if attributes.OverallCurrentState.attribute_id in attribute_list:
                overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                logging.info(f"OverallCurrentState: {overall_current_state}")
                if overall_current_state is None:
                    logging.error("OverallCurrentState is None")

                CurrentPosition = overall_current_state.position
                asserts.assert_in(CurrentPosition, Clusters.ClosureControl.Enums.CurrentPositionEnum,
                                  "OverallCurrentState.position is not in the expected range")
            else:
                asserts.assert_true(False, "OverallCurrentState attribute is not supported.")
                return

            # STEP 5c: If CurrentPosition is FullyClosed, skip steps 5d and 5e.
            self.step("5c")

            if CurrentPosition == Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed:
                logging.info("CurrentPosition is FullyClosed, skipping steps 5d to 5e")

                # Skipping steps 5d and 5e
                self.skip_step("5d")
                self.skip_step("5e")
            else:
                # STEP 5d: TH sends command MoveTo with Position = MoveToFullyClosed
                self.step("5d")

                sub_handler.reset()
                try:
                    await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                        position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed,
                    ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                    pass

                # STEP 5e: Wait until TH receives a subscription report with OverallCurrentState.Position = FullyClosed
                self.step("5e")

                logging.info("Waiting for OverallCurrentState.Position to be FullyClosed")
                sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed)],
                                                              timeout_sec=timeout)

        # STEP 6a: If the PS feature is not supported on the cluster, skip steps 6b to 6e
        self.step("6a")

        if not is_ps_feature_supported:
            logging.info("Positioning Latching feature not supported, skipping steps 6b to 6e")

            # Skipping steps 6b to 6e
            self.skip_step("6b")
            self.skip_step("6c")
            self.skip_step("6d")
            self.skip_step("6e")
        else:
            # STEP 6b: TH sends command MoveTo to DUT, with Position = MoveToFullyOpen
            self.step("6b")

            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 6c: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
            self.step("6c")

            if attributes.MainState.attribute_id in attribute_list:
                main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
                logging.info(f"MainState: {main_state}")
                asserts.assert_true((main_state == Clusters.ClosureControl.Enums.MainStateEnum.kMoving or main_state == Clusters.ClosureControl.Enums.MainStateEnum.kWaitingForMotion),
                                    "MainState is not in the expected state")
            else:
                asserts.assert_true(False, "MainState attribute is not supported.")
                return

            # STEP 6d: TH sends command Stop to DUT.
            self.step("6d")

            sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Stop(), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command Stop: {e.status}")
                pass

            # STEP 6e: Wait until TH receives a subscription report with MainState = Stopped
            self.step("6e")

            logging.info("Waiting for MainState to be Stopped")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                          timeout_sec=timeout)

        # STEP 7a: If the CL feature is not supported on the cluster, skip steps 7b to 7e
        self.step("7a")

        if not is_cl_feature_supported:
            logging.info("Calibration Latching feature not supported, skipping steps 7b to 7e")

            # Skipping steps 7b to 7e
            self.skip_step("7b")
            self.skip_step("7c")
            self.skip_step("7d")
            self.skip_step("7e")
        else:
            # STEP 7b: TH sends command Calibrate to DUT
            self.step("7b")

            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Calibrate(), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command Calibrate: {e.status}")
                pass

            # STEP 7c: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
            self.step("7c")

            if attributes.MainState.attribute_id in attribute_list:
                main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
                logging.info(f"MainState: {main_state}")
                asserts.assert_true(main_state == Clusters.ClosureControl.Enums.MainStateEnum.kCalibrating,
                                    "MainState is not in the expected state")
            else:
                asserts.assert_true(False, "MainState attribute is not supported.")
                return

            # STEP 7d: TH sends command Stop to DUT.
            self.step("7d")

            sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Stop(), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command Stop: {e.status}")
                pass

            # STEP 7e: Wait until TH receives a subscription report with MainState = Stopped
            self.step("7e")

            logging.info("Waiting for MainState to be Stopped")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                          timeout_sec=timeout)

        # STEP 8a: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is SetupRequired Test Event
        self.step("8a")

        try:
            await self.send_test_event_triggers(eventTrigger=0x0104000000000003)  # SetupRequired
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
            pass

        # STEP 8b: TH reads from the DUT the MainState attribute
        self.step("8b")

        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        asserts.assert_not_equal(mainstate, NullValue, "MainState attribute is NullValue")
        # Check if the MainState attribute has the expected values
        asserts.assert_equal(mainstate, Clusters.ClosureControl.Enums.MainStateEnum.kSetupRequired,
                             "MainState is not in the expected state")

        # STEP 8c: TH sends command Stop to DUT
        self.step("8c")

        try:
            await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Stop(), endpoint=endpoint, timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command Stop: {e.status}")
            pass

        # STEP 8d: TH reads from the DUT the MainState attribute
        self.step("8d")

        mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
        asserts.assert_not_equal(mainstate, NullValue, "MainState attribute is NullValue")
        # Check if the MainState attribute has the expected values
        asserts.assert_equal(mainstate, Clusters.ClosureControl.Enums.MainStateEnum.kSetupRequired,
                             "MainState is not in the expected state")

        # STEP 9a: If the PT feature is not supported on the cluster, skip steps 9b to 9e
        self.step("9a")

        if not is_pt_feature_supported:
            logging.info("Protected feature not supported, skipping steps 9b to 9e")

            # Skipping steps 9b to 9e
            self.skip_step("9b")
            self.skip_step("9c")
            self.skip_step("9d")
            self.skip_step("9e")
        else:
            # STEP 9b: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Protected Test Event
            self.step("9b")

            try:
                await self.send_test_event_triggers(eventTrigger=0x0104000000000001)  # Protected
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
                pass

            # STEP 9c: TH reads from the DUT the MainState attribute
            self.step("9c")

            mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
            asserts.assert_not_equal(mainstate, NullValue, "MainState attribute is NullValue")
            # Check if the MainState attribute has the expected values
            asserts.assert_equal(mainstate, Clusters.ClosureControl.Enums.MainStateEnum.kProtected,
                                 "MainState is not in the expected state")

            # STEP 9d: TH sends command Stop to DUT
            self.step("9d")

            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Stop(), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command Stop: {e.status}")
                pass

            # STEP 9e: TH reads from the DUT the MainState attribute
            self.step("9e")

            mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
            asserts.assert_not_equal(mainstate, NullValue, "MainState attribute is NullValue")
            # Check if the MainState attribute has the expected values
            asserts.assert_equal(mainstate, Clusters.ClosureControl.Enums.MainStateEnum.kProtected,
                                 "MainState is not in the expected state")

        # STEP 10a: If the MO feature is not supported on the cluster, skip steps 10b to 10e
        self.step("10a")

        if not is_mo_feature_supported:
            logging.info("Manual Operable feature not supported, skipping steps 10b to 10e")

            # Skipping steps 10b to 10e
            self.skip_step("10b")
            self.skip_step("10c")
            self.skip_step("10d")
            self.skip_step("10e")
        else:
            # STEP 10b: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged Test Event
            self.step("10b")

            try:
                await self.send_test_event_triggers(eventTrigger=0x0104000000000002)  # Disengaged
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
                pass

            # STEP 10c: TH reads from the DUT the MainState attribute
            self.step("10c")

            mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
            asserts.assert_not_equal(mainstate, NullValue, "MainState attribute is NullValue")
            # Check if the MainState attribute has the expected values
            asserts.assert_equal(mainstate, Clusters.ClosureControl.Enums.MainStateEnum.kDisengaged,
                                 "MainState is not in the expected state")

            # STEP 10d: TH sends command Stop to DUT
            self.step("10d")

            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Stop(), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command Stop: {e.status}")
                pass

            # STEP 10e: TH reads from the DUT the MainState attribute
            self.step("10e")

            mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
            asserts.assert_not_equal(mainstate, NullValue, "MainState attribute is NullValue")
            # Check if the MainState attribute has the expected values
            asserts.assert_equal(mainstate, Clusters.ClosureControl.Enums.MainStateEnum.kDisengaged,
                                 "MainState is not in the expected state")

        # STEP 11: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear
        self.step("11")

        try:
            await self.send_test_event_triggers(eventTrigger=0x0104000000000004)  # Test Event Clear
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
            pass


if __name__ == "__main__":
    default_matter_test_main()
