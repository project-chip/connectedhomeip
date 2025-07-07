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
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
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


def target_latch_matcher(latch: bool) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallTargetState:
            return False

        return report.value.latch == latch

    return AttributeMatcher.from_callable(description=f"OverallTargetState.Latch is {latch}", matcher=predicate)


def main_state_matcher(main_state: Clusters.ClosureControl.Attributes.MainState) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.MainState:
            return False

        return report.value == main_state

    return AttributeMatcher.from_callable(description=f"MainState is {main_state}", matcher=predicate)


class TC_CLCTRL_4_2(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_4_2(self) -> str:
        return "[TC-CLCTRL-4.2] MoveTo Command Latching Functionality with DUT as Server"

    def steps_TC_CLCTRL_4_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read the FeatureMap attribute to determine supported features",
                     "FeatureMap of the ClosureControl cluster is returned by the DUT"),
            TestStep("2b", "If the LT feature is not supported, skip remaining steps and end test case"),
            TestStep("2c", "Read the LatchControlModes attribute",
                     "LatchControlModes of the ClosureControl cluster is returned by the DUT; Value saved as LatchControlModes"),
            TestStep("2d", "Read the OverallCurrentState attribute",
                     "OverallCurrentState of the ClosureControl cluster is returned by the DUT; Latching field is saved as CurrentLatch"),
            TestStep("2e", "Establish a wildcard subscription to all attributes on the ClosureControl cluster",
                     "Subscription successfully established"),
            TestStep("2f", "Preparing Latch-State: If CurrentLatch is False, skip steps 2g to 2k"),
            TestStep("2g", "If LatchControlModes Bit 1 = 0, skip step 2h"),
            TestStep("2h", "Send MoveTo command with Latch = False", "Receive SUCCESS response from the DUT"),
            TestStep("2i", "If LatchControlModes Bit 1 = 1, skip step 2j"),
            TestStep("2j", "Unlatch the device manually"),
            TestStep("2k", "Wait until a subscription report with OverallCurrentState.Latch is received",
                     "OverallCurrentState.Latch should be False"),
            TestStep("3a", "Handle Latch with LatchControlModes = 0 (Bit 0 = 0, Bit 1 = 0), else skip steps 3b to 3e"),
            TestStep("3b", "Send MoveTo command with Latch = True",
                     "Receive INVALID_IN_STATE error from the DUT"),
            TestStep("3c", "Manually latch the device and wait until a subscription report with OverallCurrentState.Latch is received",
                     "OverallCurrentState.Latch should be True"),
            TestStep("3d", "Send MoveTo command with Latch = False", "Receive INVALID_IN_STATE error from the DUT"),
            TestStep("3e", "Manually unlatch the device and wait until a subscription report with OverallCurrentState.Latch is received",
                     "OverallCurrentState.Latch should be False"),
            TestStep("4a", "Handle Latch with LatchControlModes = 1 (Bit 0 = 1, Bit 1 = 0), else skip steps 4b to 4h"),
            TestStep("4b", "Send MoveTo command with Latch = True", "Receive SUCCESS response from the DUT"),
            TestStep("4c", "Wait until a subscription report with OverallTargetState is received",
                     "OverallTargetState.Latch should be True"),
            TestStep("4d", "Wait until a subscription report with MainState is received", "MainState should be Moving"),
            TestStep("4e", "Wait until a subscription report with OverallCurrentState is received",
                     "OverallCurrentState.Latch should be True"),
            TestStep("4f", "Wait until a subscription report with MainState is received", "MainState should be Stopped"),
            TestStep("4g", "Send MoveTo command with Latch = False", "Receive INVALID_IN_STATE error from the DUT"),
            TestStep("4h", "Manually unlatch the device and wait until a subscription report with OverallCurrentState.Latch is received",
                     "OverallCurrentState.Latch should be False"),
            TestStep("5a", "Handle Latch with LatchControlModes = 2 (Bit 0 = 0, Bit 1 = 1), else skip steps 5b to 5h"),
            TestStep("5b", "Send MoveTo command with Latch = True", "Receive INVALID_IN_STATE error from the DUT"),
            TestStep("5c", "Manually latch the device and wait until a subscription report with OverallCurrentState.Latch is received",
                     "OverallCurrentState.Latch should be True"),
            TestStep("5d", "Send MoveTo command with Latch = False", "Receive SUCCESS response from the DUT"),
            TestStep("5e", "Wait until a subscription report with OverallTargetState.Latch is received",
                     "OverallTargetState.Latch should be False"),
            TestStep("5f", "Wait until a subscription report with MainState is received", "MainState should be Moving"),
            TestStep("5g", "Wait until a subscription report with OverallCurrentState.Latch is received",
                     "OverallCurrentState.Latch should be False"),
            TestStep("5h", "Wait until a subscription report with MainState is received", "MainState should be Stopped"),
            TestStep("6a", "Handle Latch with LatchControlModes = 3 (Bit 0 = 1, Bit 1 = 1), else skip steps 6b to 6k"),
            TestStep("6b", "Send MoveTo command with Latch = True", "Receive SUCCESS response from the DUT"),
            TestStep("6c", "Wait until a subscription report with OverallTargetState is received",
                     "OverallTargetState.Latch should be True"),
            TestStep("6d", "Wait until a subscription report with MainState is received", "MainState should be Moving"),
            TestStep("6e", "Wait until a subscription report with OverallCurrentState is received",
                     "OverallCurrentState.Latch should be True"),
            TestStep("6f", "Wait until a subscription report with MainState is received", "MainState should be Stopped"),
            TestStep("6g", "Send MoveTo command with Latch = False", "Receive SUCCESS response from the DUT"),
            TestStep("6h", "Wait until a subscription report with OverallTargetState is received",
                     "OverallTargetState.Latch should be False"),
            TestStep("6i", "Wait until a subscription report with MainState is received", "MainState should be Moving"),
            TestStep("6j", "Wait until a subscription report with OverallCurrentState is received",
                     "OverallCurrentState.Latch should be False"),
            TestStep("6k", "Wait until a subscription report with MainState is received", "MainState should be Stopped"),
        ]
        return steps

    def pics_TC_CLCTRL_4_2(self) -> list[str]:
        pics = [
            "CLCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_4_2(self):

        endpoint = self.get_endpoint(default=1)
        timeout: uint = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout  # default_timeout = 90 seconds

        self.step(1)
        attributes: typing.List[uint] = Clusters.ClosureControl.Attributes

        self.step("2a")
        feature_map: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        is_latching_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching

        logging.info(f"FeatureMap: {feature_map}")

        self.step("2b")
        if not is_latching_supported:
            logging.info("Latching feature is not supported, skipping remaining steps.")
            self.skip_all_remaining_steps()
            return
        else:
            logging.info("Latching feature is supported, proceeding with the test.")

        self.step("2c")
        latch_control_modes: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)
        logging.info(f"LatchControlModes: {latch_control_modes}")

        self.step("2d")
        overall_current_state: typing.Union[Nullable, Clusters.ClosureControl.Structs.OverallCurrentStateStruct] = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallCurrentState)
        current_latch: bool = None
        if overall_current_state is NullValue:
            current_latch = NullValue
        else:
            current_latch = overall_current_state.latch
        logging.info(f"CurrentLatch: {current_latch}")

        self.step("2e")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureControl)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        self.step("2f")
        if current_latch is False:
            logging.info("CurrentLatch is False, skipping Latch = False preparation steps")
            self.skip_step("2g")
            self.skip_step("2h")
            self.skip_step("2i")
            self.skip_step("2j")
            self.skip_step("2k")
        else:
            logging.info("CurrentLatch is True, proceeding with Latch = False preparation steps")

            self.step("2g")
            # Check if LatchControlModes Bit 1 is 0
            if not latch_control_modes & Clusters.ClosureControl.Bitmaps.LatchControlModesBitmap.kRemoteUnlatching:
                self.skip_step("2h")
                self.step("2i")
                logging.info("LatchControlModes Bit 1 is 0, unlatch device manually")
                self.step("2j")
                self.wait_for_user_input(prompt_msg="Press enter when the device is unlatched")

            else:
                self.step("2h")
                logging.info("LatchControlModes Bit 1 is 1, sending MoveTo command with Latch = False")

                try:
                    await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(latch=False))
                except InteractionModelError as e:
                    asserts.assert_equal(e.status, Status.Success, f"MoveTo command with Latch = False failed: {e}")

                self.step("2i")
                self.skip_step("2j")

            self.step("2k")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)
            sub_handler.reset()

        self.step("3a")
        if latch_control_modes != 0:
            logging.info("LatchControlModes is not 0, skipping steps 3b to 3e")
            self.skip_step("3b")
            self.skip_step("3c")
            self.skip_step("3d")
            self.skip_step("3e")
        else:
            logging.info("LatchControlModes is 0, proceeding with fully manual latch tests")
            self.step("3b")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(latch=True))
                logging.error("MoveTo command with Latch = True sent successfully, but should fail due to LatchControlModes = 0")
                asserts.assert_true(False, "Expected INVALID_IN_STATE error, but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState,
                                     f"MoveTo command with Latch = True failed with unexpected status: {e.status}")
                logging.info("Received INVALID_IN_STATE error as expected")

            self.step("3c")
            self.wait_for_user_input(prompt_msg="Manually latch the device and press enter when done")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(True)], timeout_sec=timeout)

            self.step("3d")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(latch=False))
                logging.error("MoveTo command with Latch = False sent successfully, but should fail due to LatchControlModes = 0")
                asserts.assert_true(False, "Expected INVALID_IN_STATE error, but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState,
                                     f"MoveTo command with Latch = False failed with unexpected status: {e.status}")
                logging.info("Received INVALID_IN_STATE error as expected")
            self.step("3e")
            self.wait_for_user_input(prompt_msg="Manually unlatch the device and press enter when done")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)
            sub_handler.reset()

        self.step("4a")
        if latch_control_modes != 1:
            logging.info("LatchControlModes is not 1, skipping steps 4b to 4h")
            self.skip_step("4b")
            self.skip_step("4c")
            self.skip_step("4d")
            self.skip_step("4e")
            self.skip_step("4f")
            self.skip_step("4g")
            self.skip_step("4h")
        else:
            self.step("4b")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(latch=True))
                logging.info("MoveTo command with Latch = True sent successfully")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, f"MoveTo command with Latch = True failed: {e}")

            self.step("4c")
            sub_handler.await_all_expected_report_matches(expected_matchers=[target_latch_matcher(True)], timeout_sec=timeout)
            self.step("4d")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kMoving)], timeout_sec=timeout)
            self.step("4e")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(True)], timeout_sec=timeout)
            self.step("4f")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kStopped)], timeout_sec=timeout)
            self.step("4g")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(latch=False))
                logging.error("MoveTo command with Latch = False sent successfully, but should fail due to LatchControlModes = 1")
                asserts.assert_true(False, "Expected INVALID_IN_STATE error, but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState,
                                     f"MoveTo command with Latch = False failed with unexpected status: {e.status}")
                logging.info("Received INVALID_IN_STATE error as expected")
            self.step("4h")
            self.wait_for_user_input(prompt_msg="Manually unlatch the device and press enter when done")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)
            sub_handler.reset()

        self.step("5a")
        if latch_control_modes != 2:
            logging.info("LatchControlModes is not 2, skipping steps 5b to 5h")
            self.skip_step("5b")
            self.skip_step("5c")
            self.skip_step("5d")
            self.skip_step("5e")
            self.skip_step("5f")
            self.skip_step("5g")
            self.skip_step("5h")
        else:
            self.step("5b")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(latch=True))
                logging.error("MoveTo command with Latch = True sent successfully, but should fail due to LatchControlModes = 2")
                asserts.assert_true(False, "Expected INVALID_IN_STATE error, but command succeeded")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState,
                                     f"MoveTo command with Latch = True failed with unexpected status: {e.status}")
                logging.info("Received INVALID_IN_STATE error as expected")

            self.step("5c")
            self.wait_for_user_input(prompt_msg="Manually latch the device and press enter when done")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(True)], timeout_sec=timeout)

            self.step("5d")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(latch=False))
                logging.info("MoveTo command with Latch = False sent successfully")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, f"MoveTo command with Latch = False failed: {e}")

            self.step("5e")
            sub_handler.await_all_expected_report_matches(expected_matchers=[target_latch_matcher(False)], timeout_sec=timeout)
            self.step("5f")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kMoving)], timeout_sec=timeout)
            self.step("5g")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)
            self.step("5h")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kStopped)], timeout_sec=timeout)
            sub_handler.reset()

        self.step("6a")
        if latch_control_modes != 3:
            logging.info("LatchControlModes is not 3, skipping steps 6b to 6k")
            self.skip_step("6b")
            self.skip_step("6c")
            self.skip_step("6d")
            self.skip_step("6e")
            self.skip_step("6f")
            self.skip_step("6g")
            self.skip_step("6h")
            self.skip_step("6i")
            self.skip_step("6j")
            self.skip_step("6k")
        else:
            self.step("6b")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(latch=True))
                logging.info("MoveTo command with Latch = True sent successfully")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, f"MoveTo command with Latch = True failed: {e}")

            self.step("6c")
            sub_handler.await_all_expected_report_matches(expected_matchers=[target_latch_matcher(True)], timeout_sec=timeout)
            self.step("6d")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kMoving)], timeout_sec=timeout)
            self.step("6e")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(True)], timeout_sec=timeout)
            self.step("6f")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kStopped)], timeout_sec=timeout)

            self.step("6g")
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=Clusters.ClosureControl.Commands.MoveTo(latch=False))
                logging.info("MoveTo command with Latch = False sent successfully")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, f"MoveTo command with Latch = False failed: {e}")

            self.step("6h")
            sub_handler.await_all_expected_report_matches(expected_matchers=[target_latch_matcher(False)], timeout_sec=timeout)
            self.step("6i")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kMoving)],
                timeout_sec=timeout)
            self.step("6j")
            sub_handler.await_all_expected_report_matches(expected_matchers=[current_latch_matcher(False)], timeout_sec=timeout)
            self.step("6k")
            sub_handler.await_all_expected_report_matches(expected_matchers=[main_state_matcher(
                Clusters.ClosureControl.Enums.MainStateEnum.kStopped)], timeout_sec=timeout)
            sub_handler.reset()


if __name__ == "__main__":
    default_matter_test_main()
