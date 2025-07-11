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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import typing

import chip.clusters as Clusters
from chip.clusters.Types import Nullable, NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.event_attribute_reporting import ClusterAttributeChangeAccumulator
from chip.testing.matter_testing import (AttributeMatcher, AttributeValue, MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main)
from chip.tlv import uint
from mobly import asserts


def current_latch_matcher(latch: bool) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False

        return report.value.latch == latch

    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Latch is {latch}", matcher=predicate)


def current_position_matcher(position: Clusters.ClosureControl.Enums.CurrentPositionEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState:
            return False

        return report.value.position == position

    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Position is {position}", matcher=predicate)


def main_state_matcher(main_state: Clusters.ClosureControl.Attributes.MainState) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.MainState:
            return False

        return report.value == main_state

    return AttributeMatcher.from_callable(description=f"MainState is {main_state}", matcher=predicate)


class TC_CLCTRL_4_4(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_4_4(self) -> str:
        return "[TC-CLCTRL-4.4] MoveTo Command CountdownTime with Server as DUT"

    def steps_TC_CLCTRL_4_4(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read the AttributeList attribute to determine supported attributes",
                     "AttributeList of the ClosureControl cluster is returned by the DUT"),
            TestStep("2b", "Check if CountdownTime attribute is supported",
                     "CountdownTime attribute should be present in the AttributeList"),
            TestStep("2c", "Establish a wildcard subscription to all attributes on the ClosureControl cluster",
                     "Subscription successfully established"),
            TestStep("2d", "Read the FeatureMap attribute to determine supported features",
                     "FeatureMap of the ClosureControl cluster is returned by the DUT"),
            TestStep("2e", "Check if LT feature is supported", "Skip steps 2f to 2m if LT feature is not supported"),
            TestStep("2f", "Read the LatchControlModes attribute",
                     "LatchControlModes of the ClosureControl cluster is returned by the DUT; Value saved as LatchControlModes"),
            TestStep("2g", "Read the OverallCurrentState attribute",
                     "OverallCurrentState of the ClosureControl cluster is returned by the DUT; Latching field is saved as CurrentLatch"),
            TestStep("2h", "Preparing Latch-State: If CurrentLatch is False, skip steps 2i to 2m"),
            TestStep("2i", "If LatchControlModes Bit 1 = 0, skip step 2j"),
            TestStep("2j", "Send MoveTo command with Latch = False", "Receive SUCCESS response from the DUT"),
            TestStep("2k", "If LatchControlModes Bit 1 = 1, skip step 2l"),
            TestStep("2l", "Unlatch the device manually"),
            TestStep("2m", "Wait until a subscription report with OverallCurrentState.Latch is received",
                     "OverallCurrentState.Latch should be False"),
            TestStep(3, "Read the CountdownTime attribute when no operation is in progress", "CountdownTime should be 0 or null"),
            TestStep("4a", "Read the OverallCurrentState attribute",
                     "OverallCurrentState of the ClosureControl cluster is returned by the DUT; Position field is saved as CurrentPosition"),
            TestStep("4b", "Preparing Position-State: If CurrentPosition is FullyClosed, skip steps 4c and 4d"),
            TestStep("4c", "Send MoveTo command with Position = MoveToFullyClosed", "Receive SUCCESS response from the DUT"),
            TestStep("4d", "Wait until a subscription report with OverallCurrentState.Position is received",
                     "OverallCurrentState.Position should be FullyClosed"),
            TestStep("4e", "Send MoveTo command with Position = MoveToFullyOpen", "Receive SUCCESS response from the DUT"),
            TestStep("4f", "Wait until a subscription report with MainState is received", "MainState should be Moving"),
            TestStep("4g", "Read the CountdownTime attribute",
                     "CountdownTime should be between 1 and countdown_time_max, or null; Value saved as CurrentCountdownTime"),
            TestStep("4h", "Wait until a subscription report with MainState is received", "MainState should be Stopped"),
            TestStep("4i", "If CurrentCountdownTime is null skip step 4j"),
            TestStep("4j", "Read the CountdownTime attribute", "CountdownTime should be 0"),
            TestStep("5a", "If CurrentCountdownTime is null, skip steps 5b to 5f"),
            TestStep("5b", "Send MoveTo command with Position = MoveToFullyOpen", "Receive SUCCESS response from the DUT"),
            TestStep("5c", "Read the CountdownTime attribute", "CountdownTime should be between 1 and countdown_time_max"),
            TestStep("5d", "Send Stop command", "Receive SUCCESS response from the DUT"),
            TestStep("5e", "Wait until a subscription report with MainState is received", "MainState should be Stopped"),
            TestStep("5f", "Read the CountdownTime attribute", "CountdownTime should be 0"),
        ]
        return steps

    def pics_TC_CLCTRL_4_4(self) -> list[str]:
        pics = [
            "CLCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_4_4(self):
        countdown_time_max: uint = 259200

        endpoint: int = self.get_endpoint(default=1)
        timeout: uint = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else countdown_time_max

        self.step(1)
        attributes: typing.List[uint] = Clusters.ClosureControl.Attributes

        self.step("2a")
        attribute_list: typing.List[uint] = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
        logging.info(f"AttributeList: {attribute_list}")

        self.step("2b")
        is_countdown_time_supported: bool = attributes.CountdownTime.attribute_id in attribute_list

        if not is_countdown_time_supported:
            logging.info("CountdownTime attribute not supported, skipping test")
            self.skip_all_remaining_steps()
            return

        self.step("2c")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureControl)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        self.step("2d")
        feature_map: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        is_latching_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching

        self.step("2e")
        if not is_latching_supported:
            logging.info("Latching feature not supported, skipping steps related to latching")
            self.skip_step("2f")
            self.skip_step("2g")
            self.skip_step("2h")
            self.skip_step("2i")
            self.skip_step("2j")
            self.skip_step("2k")
            self.skip_step("2l")
            self.skip_step("2m")

        else:
            logging.info("Latching feature supported, proceeding with latch preparation steps")

            self.step("2f")
            latch_control_modes: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)
            logging.info(f"LatchControlModes: {latch_control_modes}")

            self.step("2g")
            overall_current_state: typing.Union[Nullable, Clusters.ClosureControl.Structs.OverallCurrentStateStruct] = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallCurrentState)
            current_latch: bool = None

            if overall_current_state is NullValue:
                current_latch: bool = NullValue
            else:
                current_latch: bool = overall_current_state.latch
            logging.info(f"CurrentLatch: {current_latch}")

            self.step("2h")
            if current_latch is False:
                logging.info("CurrentLatch is False, skipping Latch = False preparation steps")
                self.skip_step("2i")
                self.skip_step("2j")
                self.skip_step("2k")
                self.skip_step("2l")
                self.skip_step("2m")
            else:
                logging.info("CurrentLatch is True, proceeding with Latch = False preparation steps")

                self.step("2i")
                # Check if LatchControlModes Bit 1 is 0
                if not latch_control_modes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                    self.skip_step("2j")
                    self.step("2k")
                    logging.info("LatchControlModes Bit 1 is 0, unlatch device manually")
                    self.step("2l")
                    self.wait_for_user_input(prompt_msg="Press enter when the device is unlatched")

                else:
                    self.step("2j")
                    logging.info("LatchControlModes Bit 1 is 1, sending MoveTo command with Latch = False")

                    try:
                        await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(latch=False), timedRequestTimeoutMs=1000)
                    except InteractionModelError as e:
                        asserts.assert_equal(e.status, Status.Success, f"MoveTo command with Latch = False failed: {e}")

                    self.step("2k")
                    self.skip_step("2l")

                self.step("2m")
                sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)
                logging.info("Latch is now False, proceeding with CountdownTime checks")
                sub_handler.reset()

        # STEP 3: Verify the CountdownTime when no operation is in progress
        self.step(3)
        countdown_time: typing.Union[NullValue, uint] = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
        asserts.assert_true(countdown_time == 0 or countdown_time == NullValue,
                            f"CountdownTime should be 0 or null when no operation is in progress, got: {countdown_time}.")

        # STEP 4: Verify the CountdownTime when an operation is triggered
        self.step("4a")
        overall_current_state: typing.Union[Nullable, Clusters.ClosureControl.Structs.OverallCurrentStateStruct] = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallCurrentState)
        current_position: Clusters.ClosureControl.Enums.CurrentPositionEnum = None

        if overall_current_state is NullValue:
            current_position = NullValue
        else:
            current_position = overall_current_state.position
        logging.info(f"CurrentPosition: {current_position}")

        self.step("4b")
        if current_position == Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed:
            logging.info("CurrentPosition is FullyClosed, skipping steps 4c and 4d.")
            self.skip_step("4c")
            self.skip_step("4d")
        else:
            logging.info("CurrentPosition is not FullyClosed, proceeding with Position = FullyClosed preparation steps")

            self.step("4c")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed), timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, f"MoveTo command to FullyClosed position failed: {e}")

            self.step("4d")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_position_matcher(
                Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed)], timeout_sec=timeout)

        self.step("4e")
        try:
            await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen), timedRequestTimeoutMs=1000)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, f"MoveTo command to FullyOpened position failed: {e}")

        self.step("4f")
        sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
            Clusters.ClosureControl.Enums.MainStateEnum.kMoving)], timeout_sec=timeout)

        self.step("4g")
        current_countdown_time: typing.Union[NullValue, uint] = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
        asserts.assert_true(countdown_time == NullValue or (1 <= current_countdown_time <= countdown_time_max),
                            f"CountdownTime should be between 1 and {countdown_time_max}, or null, got: {current_countdown_time}.")
        logging.info(f"CurrentCountdownTime: {current_countdown_time}")

        self.step("4h")
        sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
            Clusters.ClosureControl.Enums.MainStateEnum.kStopped)], timeout_sec=timeout)

        self.step("4i")
        if current_countdown_time is NullValue:
            logging.info("CurrentCountdownTime is Null, skipping step 4j.")
            self.skip_step("4j")
        else:
            self.step("4j")
            countdown_time_after_operation: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
            asserts.assert_equal(countdown_time_after_operation, 0,
                                 f"CountdownTime should be 0 after operation completes, got: {countdown_time_after_operation}.")
            logging.info(f"CountdownTime after operation: {countdown_time_after_operation}")
        sub_handler.reset()

        # STEP 5: Verify the CountdownTime behavior when an operation is interrupted
        self.step("5a")
        if current_countdown_time is NullValue:
            logging.info("CurrentCountdownTime is Null, skipping steps 5b to 5f.")
            self.skip_step("5b")
            self.skip_step("5c")
            self.skip_step("5d")
            self.skip_step("5e")
            self.skip_step("5f")
        else:
            self.step("5b")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(position=Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed), timedRequestTimeoutMs=1000)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, f"MoveTo command to FullyClosed position failed: {e}")

            self.step("5c")
            countdown_time_before_interruption: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
            asserts.assert_true(countdown_time_before_interruption > 0 and countdown_time_before_interruption <
                                countdown_time_max, f"CountdownTime before interruption: {countdown_time_before_interruption}.")
            logging.info(f"CountdownTime before interruption: {countdown_time_before_interruption}")

            self.step("5d")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.Stop())
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Stop command failed: {e}")
            logging.info("Stop command sent, waiting for MainState to become Stopped")

            self.step("5e")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kStopped)], timeout_sec=timeout)

            self.step("5f")
            countdown_time_after_interruption: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.CountdownTime)
            asserts.assert_true(countdown_time_after_interruption == 0,
                                f"CountdownTime after interruption not 0, but: {countdown_time_after_interruption}.")
            logging.info(f"CountdownTime after interruption not 0, but: {countdown_time_after_interruption}")
            sub_handler.reset()


if __name__ == "__main__":
    default_matter_test_main()
