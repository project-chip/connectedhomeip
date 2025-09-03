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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --timeout 30
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import (AttributeMatcher, AttributeValue, MatterBaseTest, TestStep, async_test_body,
                                           default_matter_test_main)


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


def current_latch_matcher(current_latch: bool) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False
        return report.value.latch == current_latch
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Latch is {current_latch}", matcher=predicate)


class TC_CLCTRL_3_1(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_3_1(self) -> str:
        return "[TC_CLCTRL_3_1] Calibrate Command Primary Functionality with DUT as Server"

    def steps_TC_CLCTRL_3_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", is_commissioning=True),
            TestStep("2a", "TH reads from the DUT the (0xFFFC) FeatureMap attribute"),
            TestStep("2b", "If the CL feature is not supported on the cluster, skip remaining steps and end test case."),
            TestStep("2c", "TH establishes a wildcard subscription to all attributes on the Closure Control Cluster, with MinIntervalFloor = 0, MaxIntervalCeiling = 30 and KeepSubscriptions = false."),
            TestStep("2d", "TH reads from the DUT the (0xFFFB) AttributeList attribute"),
            TestStep("3a", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute"),
            TestStep("3b", "TH sends command Calibrate to DUT"),
            TestStep("3c", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute"),
            TestStep("3d", "Wait until the TH receives a subscription report for the MainState attribute"),
            TestStep("4a", "TH sends command Calibrate to DUT"),
            TestStep("4b", "If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute"),
            TestStep("4c", "TH sends command Calibrate to DUT"),
            TestStep("4d", "Wait until the TH receives a subscription report for the MainState attribute"),
            TestStep("5a", "If the LT feature is not supported on the cluster, skip steps 5b to 5i."),
            TestStep("5b", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute."),
            TestStep("5c", "If the attribute is supported on the cluster, TH reads from the DUT the LatchControlModes attribute."),
            TestStep("5d", "If CurrentLatch = False, skip steps 5e to 5i."),
            TestStep("5e", "If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 5f."),
            TestStep("5f", "TH sends command MoveTo with Latch = False."),
            TestStep("5g", "If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 5h."),
            TestStep("5h", "Unlatch the DUT manually to set OverallCurrentState.Latch to False."),
            TestStep("5i", "Wait until TH receives a subscription report with OverallCurrentState.Latch = False."),
            TestStep("5j", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute."),
            TestStep("5k", "If CurrentPosition = FullyClosed, skip steps 5l to 5m."),
            TestStep("5l", "TH sends command MoveTo with Position = MoveToFullyClosed."),
            TestStep("5m", "Wait until TH receives a subscription report with OverallCurrentState.Position = FullyClosed."),
            TestStep("6a", "TH sends command MoveTo to DUT with Position = MoveToFullyOpen"),
            TestStep("6b", "Wait until the TH receives a subscription report for the OverallCurrentState attribute"),
            TestStep("6c", "If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute"),
            TestStep("6d", "TH sends command MoveTo to DUT with Position = MoveToFullyClosed"),
            TestStep("6e", "If attribute is supported on the cluster, TH reads from the DUT the MainState attribute"),
            TestStep("6f", "TH sends command Calibrate to DUT"),
        ]
        return steps

    def pics_TC_CLCTRL_3_1(self) -> list[str]:
        pics = [
            "CLCTRL.S"
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_3_1(self):
        endpoint = self.get_endpoint(default=1)
        dev_controller = self.default_controller
        attributes = Clusters.ClosureControl.Attributes
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.step(1)

        # STEP 2a: TH reads from the DUT the (0xFFFC) FeatureMap attribute
        self.step("2a")

        logging.info("Reading FeatureMap attribute from the DUT")
        feature_map = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        logging.info(f"FeatureMap: {feature_map}")
        is_cl_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kCalibration
        is_lt_feature_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching

        # STEP 2b: If the CL feature is not supported on the cluster, skip remaining steps and end test case.
        self.step("2b")

        if not is_cl_feature_supported:
            logging.info("The feature Calibration is not supported by the DUT")
            self.mark_all_remaining_steps_skipped("2c")
            return

        # STEP 2c: TH establishes a wildcard subscription to all attributes on the Closure Control Cluster, with MinIntervalFloor = 0, MaxIntervalCeiling = 30 and KeepSubscriptions = false.
        self.step("2c")

        logging.info("Establishing a wildcard subscription")

        sub_handler = AttributeSubscriptionHandler(expected_cluster=Clusters.ClosureControl)
        await sub_handler.start(dev_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        # STEP 2d: TH reads from the DUT the (0xFFFB) AttributeList attribute
        self.step("2d")

        attribute_list = await self.read_clctrl_attribute_expect_success(endpoint, attributes.AttributeList)
        logging.info(f"AttributeList: {attribute_list}")

        # STEP 3a: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("3a")

        if attributes.MainState.attribute_id in attribute_list:
            mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
            # Check if the MainState attribute has the expected values
            is_setup_required = mainstate == Clusters.ClosureControl.Enums.MainStateEnum.kSetupRequired
            is_stopped = mainstate == Clusters.ClosureControl.Enums.MainStateEnum.kStopped
            logging.info(f"Mainstate: {mainstate}")
            asserts.assert_true(is_setup_required or is_stopped, "MainState is not in the expected state")

        # STEP 3b: TH sends command Calibrate to DUT
        self.step("3b")

        sub_handler.reset()
        try:
            await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Calibrate(), endpoint=endpoint, timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command Calibrate: {e.status}")
            pass

        # STEP 3c: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("3c")

        if attributes.MainState.attribute_id in attribute_list:
            mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
            # Check if the MainState attribute has the expected values
            asserts.assert_equal(mainstate, Clusters.ClosureControl.Enums.MainStateEnum.kCalibrating,
                                 "MainState is not in the expected state")
            logging.info(f"Mainstate: {mainstate}")

        # STEP 3d: Wait until the TH receives a subscription report for the MainState attribute
        self.step("3d")

        # Wait for the mainstate to be updated
        logging.info("Waiting for MainState attribute to be updated")
        sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                      timeout_sec=timeout)

        # STEP 4a: TH sends command Calibrate to DUT
        self.step("4a")

        try:
            await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Calibrate(), endpoint=endpoint, timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command Calibrate: {e.status}")
            pass

        # STEP 4b: If the attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("4b")

        if attributes.MainState.attribute_id in attribute_list:
            mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
            # Check if the MainState attribute has the expected values
            asserts.assert_equal(mainstate, Clusters.ClosureControl.Enums.MainStateEnum.kCalibrating,
                                 "MainState is not in the expected state")

        # STEP 4c: TH sends command Calibrate to DUT
        self.step("4c")

        sub_handler.reset()
        try:
            await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Calibrate(), endpoint=endpoint, timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command Calibrate: {e.status}")
            pass

        # STEP 4d: Wait until the TH receives a subscription report for the MainState attribute
        self.step("4d")

        # Wait for the mainstate to be updated
        logging.info("Waiting for MainState attribute to be updated")
        sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(Clusters.ClosureControl.Enums.MainStateEnum.kStopped)],
                                                      timeout_sec=timeout)

        # STEP 5a: If the LT feature is not supported on the cluster, skip steps 5b to 5i
        self.step("5a")

        if is_lt_feature_supported:
            logging.info("Motion Latching feature supported.")

            # STEP 5b: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
            self.step("5b")

            if attributes.OverallCurrentState.attribute_id in attribute_list:
                overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
                logging.info(f"OverallCurrentState: {overall_current_state}")

                CurrentLatch = overall_current_state.latch
            else:
                asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

            # STEP 5c: If the attribute is supported on the cluster, TH reads from the DUT the LatchControlModes attribute
            self.step("5c")

            if attributes.LatchControlModes.attribute_id in attribute_list:
                LatchControlModes = await self.read_clctrl_attribute_expect_success(endpoint, attributes.LatchControlModes)
                logging.info(f"LatchControlModes: {LatchControlModes}")

            else:
                asserts.assert_true(False, "LatchControlModes attribute is not supported.")

            # STEP 5d: If CurrentLatch = False, skip steps 5e to 5i
            self.step("5d")

            if not CurrentLatch:
                logging.info("CurrentLatch is False, skipping steps 5e to 5i")

                # Skipping steps 5e to 5i
                self.mark_step_range_skipped("5e", "5i")
            else:
                logging.info("CurrentLatch is True, proceeding to steps 5e to 5i")

                # STEP 5e: If LatchControlModes Bit 1 = 0 (RemoteUnlatching = False), skip step 5f
                self.step("5e")

                sub_handler.reset()
                if not LatchControlModes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                    logging.info("LatchControlModes Bit 1 is 0 (RemoteUnlatching = False), skipping step 5f")
                    self.skip_step("5f")

                    # STEP 5g: If LatchControlModes Bit 1 = 1 (RemoteUnlatching = True), skip step 5h
                    self.step("5g")

                    logging.info("RemoteUnlatching is False, proceeding to step 5h")

                    # STEP 5h: Unlatch the DUT manually to set OverallCurrentState.Latch to False
                    self.step("5h")

                    logging.info("Unlatch the DUT manually to set OverallCurrentState.Latch to False")
                    # Simulating manual unlatching by waiting for user input
                    self.wait_for_user_input(prompt_msg="Press Enter after unlatching the DUT...")
                    logging.info("Manual unlatching completed.")
                else:
                    logging.info("LatchControlModes Bit 1 is 1 (RemoteUnlatching = True), proceeding to step 5f")

                    # STEP 5f: TH sends command MoveTo with Latch = False
                    self.step("5f")

                    try:
                        await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                            latch=False
                        ), endpoint=endpoint, timedRequestTimeoutMs=1000)
                    except InteractionModelError as e:
                        asserts.assert_equal(
                            e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                        pass

                    self.step("5g")
                    self.skip_step("5h")

                # STEP 5i: Wait until TH receives a subscription report with OverallCurrentState.Latch = False
                self.step("5i")

                logging.info("Waiting for OverallCurrentState.Latch to be False")
                sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(False)],
                                                              timeout_sec=timeout)
        else:
            logging.info("Motion Latching feature not supported, skipping steps 5b to 5i")

            # Skipping steps 5b to 5i
            self.mark_step_range_skipped("5b", "5i")

        # STEP 5j: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
        self.step("5j")

        if attributes.OverallCurrentState.attribute_id in attribute_list:
            overall_current_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
            logging.info(f"OverallCurrentState: {overall_current_state}")

            if overall_current_state is None:
                logging.error("OverallCurrentState is None")

            CurrentPosition = overall_current_state.position
        else:
            asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

        # STEP 5k: If CurrentPosition = FullyClosed, skip steps 5l to 5m
        self.step("5k")

        if CurrentPosition == Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed:
            logging.info("CurrentPosition is FullyClosed, skipping steps 5l to 5m")

            # Skipping steps 5l and 5m
            self.skip_step("5l")
            self.skip_step("5m")
        else:
            # STEP 5l: TH sends command MoveTo with Position = MoveToFullyClosed
            self.step("5l")

            sub_handler.reset()
            try:
                await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                    position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed,
                ), endpoint=endpoint, timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
                pass

            # STEP 5m: Wait until TH receives a subscription report with OverallCurrentState.Position = FullyClosed
            self.step("5m")

            logging.info("Waiting for OverallCurrentState.Position to be FullyClosed")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed)],
                                                          timeout_sec=timeout)

        # STEP 6a: TH sends command MoveTo to DUT with Position = MoveToFullyOpen
        self.step("6a")

        sub_handler.reset()

        try:
            await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen
            ), endpoint=endpoint, timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
            pass

        # STEP 6b: Wait until the TH receives a subscription report for the OverallCurrentState attribute
        self.step("6b")

        # Wait for the OverallCurrentState to be updated
        logging.info("Waiting for OverallCurrentState attribute to be updated")
        sub_handler.await_all_expected_report_matches(
            expected_matchers=[current_position_matcher(Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyOpened)],
            timeout_sec=timeout)

        # STEP 6c: If the attribute is supported on the cluster, TH reads from the DUT the OverallCurrentState attribute
        self.step("6c")

        if attributes.OverallCurrentState.attribute_id in attribute_list:
            overall_state = await self.read_clctrl_attribute_expect_success(endpoint, attributes.OverallCurrentState)
            logging.info(f"OverallCurrentState: {overall_state}")
            asserts.assert_equal(overall_state.position, Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyOpened,
                                 "OverallCurrentState.position is not FullyOpened")
        else:
            asserts.assert_true(False, "OverallCurrentState attribute is not supported.")

        # STEP 6d: TH sends command MoveTo to DUT with Position = MoveToFullyClosed
        self.step("6d")

        try:
            await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.MoveTo(
                position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed
            ), endpoint=endpoint, timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.Success, f"Failed to send command MoveTo: {e.status}")
            pass

        # STEP 6e: If attribute is supported on the cluster, TH reads from the DUT the MainState attribute
        self.step("6e")

        if attributes.MainState.attribute_id in attribute_list:
            mainstate = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.MainState)
            # Check if the MainState attribute has the expected values
            asserts.assert_equal(mainstate, Clusters.ClosureControl.Enums.MainStateEnum.kMoving,
                                 "MainState is not in the expected state")

        # STEP 6f: TH sends command Calibrate to DUT
        self.step("6f")

        try:
            await self.send_single_cmd(cmd=Clusters.ClosureControl.Commands.Calibrate(), endpoint=endpoint, timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.InvalidInState, f"The Calibrate command sent in an incorrect state: {e.status}")
            pass


if __name__ == "__main__":
    default_matter_test_main()
