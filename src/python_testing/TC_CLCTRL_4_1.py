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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CLOSURE_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --enable-key 000102030405060708090a0b0c0d0e0f --trace-to json:${TRACE_APP}.json
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
import time

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.event_attribute_reporting import ClusterAttributeChangeAccumulator
from chip.testing.matter_testing import (AttributeMatcher, AttributeValue, MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main)
from mobly import asserts

triggerProtected = 0x0104000000000001
triggerDisengaged = 0x0104000000000002
triggerClear = 0x0104000000000004


def main_state_matcher(main_state: Clusters.ClosureControl.Enums.MainStateEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.MainState:
            return False
        return report.value == main_state
    return AttributeMatcher.from_callable(description=f"MainState is {main_state}", matcher=predicate)


def current_position_matcher(current_position: Clusters.ClosureControl.Enums.CurrentPositionEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False
        return report.value.position == current_position
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Position is {current_position}", matcher=predicate)


def current_speed_matcher(current_speed: Clusters.Globals.Enums.ThreeLevelAutoEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False
        return report.value.speed == current_speed
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.speed is {current_speed}", matcher=predicate)


def current_latch_matcher(current_latch: bool) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False
        return report.value.latch == current_latch
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Latch is {current_latch}", matcher=predicate)


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
            TestStep("2c", "TH reads TestEventTrigger attribute from the General Diagnostic Cluster."),
            TestStep("2d", "TH reads from the DUT the (0xFFFB) AttributeList attribute."),
            TestStep("2e", "TH establishes a wildcard subscription to all attributes on the Closure Control Cluster, with MinIntervalFloor = 0, MaxIntervalCeiling = 30 and KeepSubscriptions = false."),
            TestStep("3a", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute."),
            TestStep("3b", "If CurrentPosition = FullyClosed, skip steps 3c to 3d."),
            TestStep("3c", "TH sends command MoveTo with Position = MoveToFullyClosed."),
            TestStep("3d", "Wait until TH receives a subscription report with OverallCurrentState.Position = FullyClosed."),
            TestStep("3e", "If the LT feature is not supported on the cluster, skip steps 3f to 5e."),
            TestStep("3f", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute."),
            TestStep("3g", "If the attribute is supported on the cluster, TH reads from the DUT the LatchControlModes attribute."),
            TestStep("3h", "If CurrentLatch = True, skip steps 3i to 3m."),
            TestStep("3i", "If LatchControlModes Bit 0 = 0 (RemoteLatching = False), skip step 3j."),
            TestStep("3j", "TH sends command MoveTo with Latch = True."),
            TestStep("3k", "If LatchControlModes Bit 0 = 1 (RemoteLatching = True), skip step 3l."),
            TestStep("3l", "Latch the DUT manually to set OverallCurrentState.Latch to True."),
            TestStep("3m", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute."),
            TestStep("3n", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("4a", "TH sends command MoveTo with Position = MoveToFullyOpen."),
            TestStep("5a", "If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 5b."),
            TestStep("5b", "TH sends command MoveTo with Latch = False."),
            TestStep("5c", "If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 5d."),
            TestStep("5d", "Unlatch the DUT manually to set OverallCurrentState.Latch to False."),
            TestStep("5e", "Wait until TH receives a subscription report with OverallCurrentState.Latch = False."),
            TestStep("6a", "If the SP feature is not supported on the cluster, skip steps 6b to 6e."),
            TestStep("6b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute."),
            TestStep("6c", "If CurrentSpeed = Low, skip steps 6d to 6e."),
            TestStep("6d", "TH sends command MoveTo with Speed = Low."),
            TestStep("6e", "Wait until TH receives a subscription report with OverallCurrentState.Speed = Low."),
            TestStep("7a", "TH sends command MoveTo with Position = MoveToSignaturePosition."),
            TestStep("7b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute."),
            TestStep("7c", "Wait until TH receives a subscription report with OverallCurrentState.Position = OpenedAtSignature."),
            TestStep("7d", "Wait until TH receives a subscription report with MainState = Stopped."),
            TestStep("8a", "If VT feature is not supported on the cluster, skip steps 8b to 8e."),
            TestStep("8b", "TH sends command MoveTo with Position = MoveToVentilationPosition."),
            TestStep("8c", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute."),
            TestStep("8d", "Wait until TH receives a subscription report with OverallCurrentState.Position = OpenedForVentilation."),
            TestStep("8e", "Wait until TH receives a subscription report with MainState = Stopped."),
            TestStep("9a", "If PD feature is not supported on the cluster, skip steps 9b to 9e."),
            TestStep("9b", "TH sends command MoveTo with Position = MoveToPedestrianPosition."),
            TestStep("9c", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute."),
            TestStep("9d", "Wait until TH receives a subscription report with OverallCurrentState.Position = OpenedForPedestrian."),
            TestStep("9e", "Wait until TH receives a subscription report with MainState = Stopped."),
            TestStep("10a", "TH sends command MoveTo with Position = MoveToFullyOpen."),
            TestStep("10b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute."),
            TestStep("10c", "Wait until TH receives a subscription report with OverallCurrentState.Position = FullyOpened."),
            TestStep("10d", "Wait until TH receives a subscription report with MainState = Stopped."),
            TestStep("11a", "TH sends command MoveTo with Position = MoveToFullyClosed."),
            TestStep("11b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute."),
            TestStep("11c", "Wait until TH receives a subscription report with OverallCurrentState.Position = FullyClosed."),
            TestStep("11d", "Wait until TH receives a subscription report with MainState = Stopped."),
            TestStep("12a", "If SP feature is not supported on the cluster, skip steps 12b to 12g."),
            TestStep("12b", "TH sends command MoveTo with Speed = High."),
            TestStep("12c", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute."),
            TestStep("12d", "Wait until TH receives a subscription report with OverallCurrentState.Speed = High."),
            TestStep("12e", "TH sends command MoveTo with Speed = Low."),
            TestStep("12f", "If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute."),
            TestStep("12g", "Wait until TH receives a subscription report with OverallCurrentState.Speed = Low."),
            TestStep("13a", "If PT feature is not supported on the cluster, skip steps 13b to 13f."),
            TestStep("13b", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Protected Test Event."),
            TestStep("13c", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("13d", "TH sends command MoveTo with Position = MoveToFullyOpen."),
            TestStep("13e", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear."),
            TestStep("13f", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("14a", "If MO feature is not supported on the cluster, skip steps 14b to 14f."),
            TestStep("14b", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged Test Event."),
            TestStep("14c", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("14d", "TH sends command MoveTo with Position = MoveToFullyOpen."),
            TestStep("14e", "TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear."),
            TestStep("14f", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute."),
            TestStep("15a", "If IS feature is not supported on the cluster, skip steps 15b to 15i."),
            TestStep("15b", "TH sends command MoveTo with Position = MoveToFullyOpen."),
            TestStep("15c", "If attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute."),
            TestStep("15d", "If the attribute is supported on the cluster, wait 2 seconds and TH reads from the DUT the OverallCurrentState.Position = FullyOpened."),
            TestStep("15e", "Wait until TH receives a subscription report with MainState = Stopped."),
            TestStep("15f", "TH sends command MoveTo with Position = MoveToFullyClosed."),
            TestStep("15g", "If attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute."),
            TestStep("15h", "If the attribute is supported on the cluster, wait 2 seconds and TH reads from the DUT the OverallCurrentState.Position = FullyClosed."),
            TestStep("15i", "Wait until TH receives a subscription report with MainState = Stopped."),
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
        dev_controller = self.default_controller
        attributes = Clusters.ClosureControl.Attributes
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)
        attributes = Clusters.ClosureControl.Attributes

        # STEP 2a: TH reads from the DUT the (0xFFFC) FeatureMap attribute
        self.step("2a")

        feature_map = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        logging.info(f"FeatureMap: {feature_map}")
        is_ps_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPositioning
        is_vt_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kVentilation
        is_pd_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPedestrian
        is_sp_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kSpeed
        is_pt_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kProtection
        is_mo_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kManuallyOperable
        is_is_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kInstantaneous
        is_lt_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching

        # STEP 2b: If the PS feature is not supported on the cluster, skip remaining steps and end test case
        self.step("2b")

        if not is_ps_feature_supported:
            logging.info("Position feature not supported, skipping test case")
            self.mark_all_remaining_steps_skipped("2c")
            return

        # STEP 2c: TH reads TestEventTriggerEnabled attribute from the General Diagnostic Cluster
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

        # STEP 3a: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
        self.step("3a")

        if attributes.OverallCurrentState.attribute_id in attribute_list:
            overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
            logging.info(f"OverallCurrentState: {overall_current_state}")

            if overall_current_state is NullValue:
                asserts.assert_true(False, "OverallCurrentState is NullValue.")

            CurrentPosition = overall_current_state.position
            asserts.assert_in(CurrentPosition, Clusters.ClosureControl.Enums.CurrentPositionEnum,
                              "OverallCurrentState.position is not in the expected range")
        else:
            asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

        # STEP 3b: If CurrentPosition = FullyClosed, skip steps 3c to 3d
        self.step("3b")

        if CurrentPosition == Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed:
            logging.info("CurrentPosition is FullyClosed, skipping steps 3c to 3d")

            # Skipping steps 3c and 3d
            self.skip_step("3c")
            self.skip_step("3d")
        else:
            # STEP 3c: TH sends command MoveTo with Position = MoveToFullyClosed
            self.step("3c")

            sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 3d: Wait until TH receives a subscription report with OverallCurrentState.Position = FullyClosed
            self.step("3d")

            logging.info("Waiting for OverallCurrentState.Position to be FullyClosed")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed)],
                                                          timeout_sec=timeout)

        # STEP 3e: If the LT feature is not supported on the cluster, skip steps 3f to 5e
        self.step("3e")

        if is_lt_feature_supported:
            logging.info("Motion Latching feature supported.")

            # STEP 3f: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
            self.step("3f")

            if attributes.OverallCurrentState.attribute_id in attribute_list:
                overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                logging.info(f"OverallCurrentState: {overall_current_state}")
                if overall_current_state is NullValue:
                    asserts.assert_true(False, "OverallCurrentState is NullValue.")

                CurrentLatch = overall_current_state.latch
            else:
                asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

            # STEP 3g: If the attribute is supported on the cluster, TH reads from the DUT the LatchControlModes attribute
            self.step("3g")

            if attributes.LatchControlModes.attribute_id in attribute_list:
                LatchControlModes = await self.read_clctrl_attribute_expect_success(endpoint, attributes.LatchControlModes)
                logging.info(f"LatchControlModes: {LatchControlModes}")

                if LatchControlModes is NullValue:
                    asserts.assert_true(False, "LatchControlModes is NullValue.")
            else:
                asserts.assert_true(False, "LatchControlModes attribute is not supported.")

            # STEP 3h: If CurrentLatch = True, skip steps 3i to 3m
            self.step("3h")

            if CurrentLatch:
                logging.info("CurrentLatch is True, skipping steps 3i to 3m")

                # Skipping steps 3i to 3m
                self.mark_step_range_skipped("3i", "3m")
            else:
                logging.info("CurrentLatch is False, proceeding to steps 3i to 3m")

                # STEP 3i: If LatchControlModes Bit 0 = 0 (RemoteLatching = False), skip step 3j
                self.step("3i")
                if not LatchControlModes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteLatching:
                    logging.info("LatchControlModes Bit 0 is 0 (RemoteLatching = False), skipping step 3j")
                    self.skip_step("3j")

                    # STEP 3k: If LatchControlModes Bit 0 = 1 (RemoteLatching = True), skip step 3l
                    self.step("3k")

                    logging.info("RemoteLatching is False, proceeding to step 3l")

                    # STEP 3l: Latch the DUT manually to set OverallCurrentState.Latch to True
                    self.step("3l")

                    logging.info("Latch the DUT manually to set OverallCurrentState.Latch to True")
                    # Simulating manual latching by waiting for user input
                    self.wait_for_user_input(promt_msg="Press Enter after latching the DUT...")
                    logging.info("Manual latching completed.")
                else:
                    logging.info("LatchControlModes Bit 0 is 1 (RemoteLatching = True), proceeding to step 3j")

                    # STEP 3j: TH sends command MoveTo with Latch = True
                    self.step("3j")

                    try:
                        await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                            latch=True
                        ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                    except InteractionModelError as e:
                        asserts.assert_equal(
                            e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                        pass
                    self.step("3k")
                    self.skip_step("3l")

                # STEP 3m: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
                self.step("3m")

                if attributes.OverallCurrentState.attribute_id in attribute_list:
                    overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                    logging.info(f"OverallCurrentState: {overall_current_state}")

                    if overall_current_state is NullValue:
                        asserts.assert_true(False, "OverallCurrentState is NullValue.")

                    asserts.assert_true(overall_current_state.latch, "OverallCurrentState.latch is not True")
                else:
                    asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

            # STEP 3n: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
            self.step("3n")

            if attributes.MainState.attribute_id in attribute_list:
                main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
                logging.info(f"MainState: {main_state}")
                asserts.assert_true(main_state == Clusters.ClosureControl.Enums.MainStateEnum.kStopped,
                                    "MainState is not in the expected state")
            else:
                asserts.assert_true(False, "MainState attribute is not supported.")

            # STEP 4a: TH sends command MoveTo with Position = MoveToFullyOpen
            self.step("4a")

            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                asserts.fail("Expected InvalidInState error, but command succeeded.")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.InvalidInState, f"The MoveTo command sends an incorrect state: {e.status}")
                pass

            # STEP 5a: If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 5b
            self.step("5a")

            sub_handler.reset()
            if not LatchControlModes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                logging.info("LatchControlModes Bit 1 is 0 (RemoteUnlatching = False), skipping step 5b")
                self.skip_step("5b")

                # STEP 5c: If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 5d
                self.step("5c")

                logging.info("RemoteUnlatching is False, proceeding to step 5d")

                # STEP 5d: Unlatch the DUT manually to set OverallCurrentState.Latch to False
                self.step("5d")

                logging.info("Unlatch the DUT manually to set OverallCurrentState.Latch to False")
                # Simulating manual unlatching by waiting for user input
                self.wait_for_user_input(promt_msg="Press Enter after unlatching the DUT...")
                logging.info("Manual unlatching completed.")
            else:
                logging.info("LatchControlModes Bit 1 is 1 (RemoteUnlatching = True), proceeding to step 5b")

                # STEP 5b: TH sends command MoveTo with Latch = False
                self.step("5b")

                try:
                    await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                        latch=False
                    ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                    pass

                self.step("5c")
                self.skip_step("5d")

            # STEP 5e: Wait until TH receives a subscription report with OverallCurrentState.Latch = False
            self.step("5e")

            logging.info("Waiting for OverallCurrentState.Latch to be False")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(False)],
                                                          timeout_sec=timeout)
        else:
            logging.info("Motion Latching feature not supported, skipping steps 3k to 5e")

            # Skipping steps 3k to 5e
            self.mark_step_range_skipped("3k", "5e")

        # STEP 6a: If the SP feature is not supported on the cluster, skip steps 6b to 6e
        self.step("6a")

        if not is_sp_feature_supported:
            logging.info("Speed feature not supported, skipping steps 6b to 6e")

            # Skipping steps 6b to 6e
            self.mark_step_range_skipped("6b", "6e")
        else:
            logging.info("Speed feature supported.")

            # STEP 6b: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
            self.step("6b")

            if attributes.OverallCurrentState.attribute_id in attribute_list:
                overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                logging.info(f"OverallCurrentState: {overall_current_state}")
                if overall_current_state is NullValue:
                    asserts.assert_true(False, "OverallCurrentState is NullValue.")

                CurrentSpeed = overall_current_state.speed
                asserts.assert_in(CurrentSpeed, Clusters.Globals.Enums.ThreeLevelAutoEnum,
                                  "OverallCurrentState.speed is not in the expected range")
            else:
                asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

            # STEP 6c: If CurrentSpeed = Low, skip steps 6d to 6e
            self.step("6c")

            if CurrentSpeed == Clusters.Globals.Enums.ThreeLevelAutoEnum.kLow:
                logging.info("CurrentSpeed is Low, skipping steps 6d to 6e")

                # Skipping steps 6d and 6e
                self.skip_step("6d")
                self.skip_step("6e")
            else:
                logging.info("CurrentSpeed is not Low, proceeding to steps 6d and 6e")

                # STEP 6d: TH sends command MoveTo with Speed = Low
                self.step("6d")

                sub_handler.reset()
                try:
                    await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                        speed=Clusters.Globals.Enums.ThreeLevelAutoEnum.kLow
                    ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                except InteractionModelError as e:
                    asserts.assert_equal(
                        e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                    pass

                # STEP 6e: Wait until TH receives a subscription report with OverallCurrentState.Speed = Low
                self.step("6e")

                logging.info("Waiting for OverallCurrentState.Speed to be Low")
                sub_handler.await_all_expected_report_matches(expected_matchers=[current_speed_matcher(Clusters.Globals.Enums.ThreeLevelAutoEnum.kLow)],
                                                              timeout_sec=timeout)

        # STEP 7a: TH sends command MoveTo with Position = MoveToSignaturePosition
        self.step("7a")

        sub_handler.reset()
        try:
            await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToSignaturePosition,
            ), endpoint=endpoint, timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
            pass

        # STEP 7b: If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute
        self.step("7b")

        if attributes.OverallTargetState.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTargetState)
            logging.info(f"OverallTargetState: {overall_target}")
            asserts.assert_equal(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToSignaturePosition,
                                 "OverallTarget.position is not MoveToSignaturePosition")
        else:
            asserts.assert_true(False, "OverallTargetState attribute is not supported.")

        # STEP 7c: Wait until TH receives a subscription report with OverallCurrentState.Position = OpenedAtSignature
        self.step("7c")

        logging.info("Waiting for OverallCurrentState.Position to be OpenedAtSignature")
        sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kOpenedAtSignature)],
                                                      timeout_sec=timeout)

        # STEP 7d: Wait until TH receives a subscription report with MainState = Stopped
        self.step("7d")

        logging.info("Waiting for MainState to be Stopped")
        sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                      timeout_sec=timeout)

        # STEP 8a: If VT feature is not supported on the cluster, skip steps 8b to 8e
        self.step("8a")

        if not is_vt_feature_supported:
            logging.info("Ventilation feature not supported, skipping steps 8b to 8e")

            # Skipping steps 8b to 8e
            self.mark_step_range_skipped("8b", "8e")
        else:
            # STEP 8b: TH sends command MoveTo with Position = MoveToVentilationPosition
            self.step("8b")

            sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToVentilationPosition,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 8c: If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute
            self.step("8c")

            if attributes.OverallTargetState.attribute_id in attribute_list:
                overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTargetState)
                logging.info(f"OverallTargetState: {overall_target}")
                asserts.assert_equal(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToVentilationPosition,
                                     "OverallTargetState.position is not MoveToVentilationPosition")
            else:
                asserts.assert_true(False, "OverallTargetState attribute is not supported.")

            # STEP 8d: Wait until TH receives a subscription report with OverallCurrentState.Position = OpenedForVentilation
            self.step("8d")

            logging.info("Waiting for OverallCurrentState.Position to be OpenedForVentilation")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kOpenedForVentilation)],
                                                          timeout_sec=timeout)

            # STEP 8e: Wait until TH receives a subscription report with MainState = Stopped
            self.step("8e")

            logging.info("Waiting for MainState to be Stopped")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                          timeout_sec=timeout)

        # STEP 9a: If PD feature is not supported on the cluster, skip steps 9b to 9e
        self.step("9a")

        if not is_pd_feature_supported:
            logging.info("Pedestrian feature not supported, skipping steps 9b to 9e")

            # Skipping steps 9b to 9e
            self.mark_step_range_skipped("9b", "9e")
        else:
            # STEP 9b: TH sends command MoveTo with Position = MoveToPedestrianPosition
            self.step("9b")

            sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToPedestrianPosition,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 9c: If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute
            self.step("9c")

            if attributes.OverallTargetState.attribute_id in attribute_list:
                overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTargetState)
                logging.info(f"OverallTargetState: {overall_target}")
                asserts.assert_equal(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToPedestrianPosition,
                                     "OverallTarget.position is not MoveToPedestrianPosition")
            else:
                asserts.assert_true(False, "OverallTargetState attribute is not supported.")

            # STEP 9d: Wait until TH receives a subscription report with OverallCurrentState.Position = OpenedForPedestrian
            self.step("9d")

            logging.info("Waiting for OverallCurrentState.Position to be OpenedForPedestrian")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kOpenedForPedestrian)],
                                                          timeout_sec=timeout)

            # STEP 9e: Wait until TH receives a subscription report with MainState = Stopped
            self.step("9e")

            logging.info("Waiting for MainState to be Stopped")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                          timeout_sec=timeout)

        # STEP 10a: TH sends command MoveTo with Position = MoveToFullyOpen
        self.step("10a")

        sub_handler.reset()
        try:
            await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
            ), endpoint=endpoint, timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
            pass

        # STEP 10b: If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute
        self.step("10b")

        if attributes.OverallTargetState.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTargetState)
            logging.info(f"OverallTarget: {overall_target}")
            asserts.assert_equal(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
                                 "OverallTargetState.position is not MoveToFullyOpen")
        else:
            asserts.assert_true(False, "OverallTargetState attribute is not supported.")

        # STEP 10c: Wait until TH receives a subscription report with OverallCurrentState.Position = FullyOpened
        self.step("10c")

        logging.info("Waiting for OverallCurrentState.Position to be FullyOpened")
        sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyOpened)],
                                                      timeout_sec=timeout)

        # STEP 10d: Wait until TH receives a subscription report with MainState = Stopped
        self.step("10d")

        logging.info("Waiting for MainState to be Stopped")
        sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                      timeout_sec=timeout)

        # STEP 11a: TH sends command MoveTo with Position = MoveToFullyClosed
        self.step("11a")

        sub_handler.reset()
        try:
            await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed,
            ), endpoint=endpoint, timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
            pass

        # STEP 11b: If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute
        self.step("11b")

        if attributes.OverallTargetState.attribute_id in attribute_list:
            overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTargetState)
            logging.info(f"OverallTargetState: {overall_target}")
            asserts.assert_equal(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed,
                                 "OverallTargetState.position is not MoveToFullyClosed")
        else:
            asserts.assert_true(False, "OverallTargetState attribute is not supported.")

        # STEP 11c: Wait until TH receives a subscription report with OverallCurrentState.Position = FullyClosed
        self.step("11c")

        logging.info("Waiting for OverallCurrentState.Position to be FullyClosed")
        sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed)],
                                                      timeout_sec=timeout)

        # STEP 11d: Wait until TH receives a subscription report with MainState = Stopped
        self.step("11d")

        logging.info("Waiting for MainState to be Stopped")
        sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                      timeout_sec=timeout)

        # STEP 12a: If SP feature is not supported on the cluster, skip steps 12b to 12g
        self.step("12a")

        if not is_sp_feature_supported:
            logging.info("Speed feature not supported, skipping steps 12b to 12g")

            # Skipping steps 12b to 12g
            self.mark_step_range_skipped("12b", "12g")
        else:
            # STEP 12b: TH sends command MoveTo with Speed = High
            self.step("12b")

            sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    speed=Clusters.Globals.Enums.ThreeLevelAutoEnum.kHigh
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 12c: If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute
            self.step("12c")

            if attributes.OverallTargetState.attribute_id in attribute_list:
                overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTargetState)
                logging.info(f"OverallTarget: {overall_target}")
                asserts.assert_equal(overall_target.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum.kHigh,
                                     "OverallTargetState.speed is not High")
            else:
                asserts.assert_true(False, "OverallTargetState attribute is not supported.")

            # STEP 12d: Wait until TH receives a subscription report with OverallCurrentState.Speed = High
            self.step("12d")

            logging.info("Waiting for OverallCurrentState.Speed to be High")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_speed_matcher(Clusters.Globals.Enums.ThreeLevelAutoEnum.kHigh)],
                                                          timeout_sec=timeout)

            # STEP 12e: TH sends command MoveTo with Speed = Low
            self.step("12e")

            sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    speed=Clusters.Globals.Enums.ThreeLevelAutoEnum.kLow
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 12f: If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute
            self.step("12f")

            if attributes.OverallTargetState.attribute_id in attribute_list:
                overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTargetState)
                logging.info(f"OverallTargetState: {overall_target}")
                asserts.assert_equal(overall_target.speed, Clusters.Globals.Enums.ThreeLevelAutoEnum.kLow,
                                     "OverallTargetState.speed is not Low")
            else:
                asserts.assert_true(False, "OverallTargetState attribute is not supported.")

            # STEP 12g: Wait until TH receives a subscription report with OverallCurrentState.Speed = Low
            self.step("12g")

            logging.info("Waiting for OverallCurrentState.Speed to be Low")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_speed_matcher(Clusters.Globals.Enums.ThreeLevelAutoEnum.kLow)],
                                                          timeout_sec=timeout)

        # STEP 13a: If PT feature is not supported on the cluster, skip steps 13b to 13f
        self.step("13a")

        if is_pt_feature_supported:
            # STEP 13b: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Protected Test Event
            self.step("13b")

            try:
                await self.send_test_event_triggers(eventTrigger=triggerProtected)  # Protected
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
                pass

            # STEP 13c: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
            self.step("13c")

            if attributes.MainState.attribute_id in attribute_list:
                main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
                logging.info(f"MainState: {main_state}")
                asserts.assert_equal(main_state, Clusters.ClosureControl.Enums.MainStateEnum.kProtected,
                                     "MainState is not Protected")
            else:
                asserts.assert_true(False, "MainState attribute is not supported.")

            # STEP 13d: TH sends command MoveTo with Position = MoveToFullyOpen
            self.step("13d")

            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                asserts.fail("Expected InvalidInState error, but command succeeded.")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.InvalidInState, f"The MoveTo command sends an incorrect state: {e.status}")
                pass

            # STEP 13e: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear
            self.step("13e")

            try:
                await self.send_test_event_triggers(eventTrigger=triggerClear)  # Test Event Clear
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
                pass

            # STEP 13f: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
            self.step("13f")

            if attributes.MainState.attribute_id in attribute_list:
                main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
                logging.info(f"MainState: {main_state}")
                is_stopped = main_state == Clusters.ClosureControl.Enums.MainStateEnum.kStopped
                asserts.assert_true(is_stopped, "MainState is not in the expected state")
            else:
                asserts.assert_true(False, "MainState attribute is not supported.")
        else:
            logging.info("Position feature not supported, skipping steps 13b to 13f")
            # Skipping steps 13b to 13f
            self.mark_step_range_skipped("13b", "13f")

        # STEP 14a: If MO feature is not supported on the cluster, skip steps 14b to 14f
        self.step("14a")

        if is_mo_feature_supported:
            # STEP 14b: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState is Disengaged Test Event
            self.step("14b")

            try:
                await self.send_test_event_triggers(eventTrigger=triggerDisengaged)  # Disengaged
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
                pass

            # STEP 14c: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
            self.step("14c")

            if attributes.MainState.attribute_id in attribute_list:
                main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
                logging.info(f"MainState: {main_state}")
                asserts.assert_equal(main_state, Clusters.ClosureControl.Enums.MainStateEnum.kDisengaged,
                                     "MainState is not Disengaged")
            else:
                asserts.assert_true(False, "MainState attribute is not supported.")

            # STEP 14d: TH sends command MoveTo with Position = MoveToFullyOpen
            self.step("14d")

            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                asserts.fail("Expected InvalidInState error, but command succeeded.")
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.InvalidInState, f"The MoveTo command sends an incorrect state: {e.status}")
                pass

            # STEP 14e: TH sends TestEventTrigger command to General Diagnostic Cluster on Endpoint 0 with EnableKey field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.CLCTRL.TEST_EVENT_TRIGGER for MainState Test Event Clear
            self.step("14e")

            try:
                await self.send_test_event_triggers(eventTrigger=triggerClear)  # Test Event Clear
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command TestEventTrigger: {e.status}")
                pass

            # STEP 14f: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
            self.step("14f")

            if attributes.MainState.attribute_id in attribute_list:
                main_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.MainState)
                logging.info(f"MainState: {main_state}")
                is_stopped = main_state == Clusters.ClosureControl.Enums.MainStateEnum.kStopped
                asserts.assert_true(is_stopped, "MainState is not in the expected state")
            else:
                asserts.assert_true(False, "MainState attribute is not supported.")
        else:
            logging.info("ManuallyOperable feature not supported, skipping steps 14b to 14f")
            # Skipping steps 14b to 14f
            self.mark_step_range_skipped("14b", "14f")

        # STEP 15a: If IS feature is not supported on the cluster, skip steps 15b to 15i
        self.step("15a")

        if is_is_feature_supported:
            # STEP 15b: TH sends command MoveTo with Position = MoveToFullyOpen
            self.step("15b")

            sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    postion=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 15c: If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute
            self.step("15c")

            if attributes.OverallTargetState.attribute_id in attribute_list:
                overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTargetState)
                logging.info(f"OverallTargetState: {overall_target}")
                asserts.assert_equal(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen,
                                     "OverallTargetState.position is not MoveToFullyOpen")
            else:
                asserts.assert_true(False, "OverallTargetState attribute is not supported.")

            # STEP 15d: If the attribute is supported on the cluster, wait 2 seconds and TH reads from the DUT the OverallCurrentState.Position = FullyOpened.
            self.step("15d")

            time.sleep(2)

            if attributes.OverallCurrentState.attribute_id in attribute_list:
                overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                logging.info(f"OverallCurrentState: {overall_current_state}")

                if overall_current_state is NullValue:
                    asserts.assert_true(False, "OverallCurrentState is NullValue.")

                logging.info(f"OverallCurrentState: {overall_current_state}")
                asserts.assert_equal(overall_current_state.position, Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyOpened,
                                     "OverallCurrentState.position is not FullyOpened")
            else:
                asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

            # STEP 15e: Wait until TH receives a subscription report with MainState = Stopped.
            self.step("15e")

            logging.info("Waiting for MainState to be Stopped")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                          timeout_sec=timeout)

            # STEP 15f: TH sends command MoveTo with Position = MoveToFullyClosed
            self.step("15f")

            sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 15g: If the attribute is supported on the cluster, TH reads from the DUT the OverallTargetState attribute
            self.step("15g")

            if attributes.OverallTargetState.attribute_id in attribute_list:
                overall_target = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallTargetState)
                logging.info(f"OverallTarget: {overall_target}")
                asserts.assert_equal(overall_target.position, Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed,
                                     "OverallTargetState.position is not MoveToFullyClosed")
            else:
                asserts.assert_true(False, "OverallTargetState attribute is not supported.")

            # STEP 15h: If the attribute is supported on the cluster, wait 2 secounds and TH reads from the DUT the OverallCurrentState.Position = FullyClosed.
            self.step("15h")

            time.sleep(2)
            if attributes.OverallCurrentState.attribute_id in attribute_list:
                overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                logging.info(f"OverallCurrentState: {overall_current_state}")

                if overall_current_state is NullValue:
                    asserts.assert_true(False, "OverallCurrentState is NullValue.")

                logging.info(f"OverallCurrentState: {overall_current_state}")
                asserts.assert_equal(overall_current_state.position, Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed,
                                     "OverallCurrentState.position is not FullyClosed")
            else:
                asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

            # STEP 15i: Wait until TH receives a subscription report with MainState = Stopped.
            self.step("15i")

            logging.info("Waiting for MainState to be Stopped")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                          timeout_sec=timeout)

        else:
            logging.info("Instantaneous feature not supported, skipping steps 15b to 15i")
            # Skipping steps 15b to 15i
            self.mark_step_range_skipped("15b", "15i")


if __name__ == "__main__":
    default_matter_test_main()
