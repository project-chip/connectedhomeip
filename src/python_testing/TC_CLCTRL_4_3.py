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
import typing
import chip.clusters as Clusters

from chip.tlv import uint
from chip.clusters.Types import Nullable, NullValue
from chip.clusters import Globals
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main, AttributeMatcher, AttributeValue, ClusterAttributeChangeAccumulator)
from mobly import asserts


def current_position_matcher(position: Clusters.ClosureControl.Enums.CurrentPositionEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState or not isinstance(report.value, list):
            return False
        for entry in report.value:
            if entry.Position == position:
                return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Position is {position}", matcher=predicate)


def current_latch_matcher(latch: bool) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState or not isinstance(report.value, list):
            return False
        for entry in report.value:
            if entry.Latch == latch:
                return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Latch is {latch}", matcher=predicate)


def current_speed_matcher(speed: Clusters.Globals.ThreeLevelAutoEnum) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.ClosureControl.Attributes.OverallCurrentState or not isinstance(report.value, list):
            return False
        for entry in report.value:
            if entry.Speed == speed:
                return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"OverallCurrentState.Speed is {speed}", matcher=predicate)


class TC_CLCTRL_4_3(MatterBaseTest):
    async def read_clctrl_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ClosureControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_CLCTRL_4_3(self) -> str:
        return "[TC-CLCTRL-4.3] MoveTo Command Field Sanity Check with Server as DUT"

    def steps_TC_CLCTRL_4_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read the FeatureMap attribute to determine supported features",
                     "FeatureMap of the ClosureControl cluster is returned by the DUT"),
            TestStep("2b", "Establish a wildcard subscription to all attributes on the ClosureControl cluster",
                     "Subscription successfully established"),
            TestStep("2c", "Read the LatchControlModes attribute",
                     "LatchControlModes of the ClosureControl cluster is returned by the DUT; Value saved as LatchControlModes"),
            TestStep("3 ", "Send MoveTo command with no fields", "Receive INVALID_COMMAND response from the DUT"),
            TestStep("4a", "Check PS and LT feature support",
                     "Skip steps 4b to 4h if the PS feature is not supported or the LT feature is supported"),
            TestStep("4b", "Read the OverallCurrentState attribute",
                     "OverallCurrentState of the ClosureControl cluster is returned by the DUT; Position field is saved as CurrentPosition"),
            TestStep("4c", "Preparing Position-State: If CurrentPosition is FullyClosed, skip steps 4d and 4e"),
            TestStep("4d", "Send MoveTo command with Position = MoveToFullyClosed", "Receive SUCCESS response from the DUT"),
            TestStep("4e", "Wait until a subscription report with OverallCurrentState.Position is received",
                     "OverallCurrentState.Position should be FullyClosed"),
            TestStep("4f", "Send MoveTo command with Latch = True", "Receive SUCCESS response from the DUT"),
            TestStep("4g", "Send MoveTo command with Latch = True and Position = MoveToFullyOpen",
                     "Receive SUCCESS response from the DUT"),
            TestStep("4h", "Wait until a subscription report with OverallCurrentState.Position is received",
                     "OverallCurrentState.Position should be FullyOpened"),
            TestStep("5a", "Check PS and LT feature support",
                     "Skip steps 5b to 5n if the PS feature is supported or the LT feature is not supported"),
            TestStep("5b", "Read the OverallCurrentState attribute",
                     "OverallCurrentState of the ClosureControl cluster is returned by the DUT; Latching field is saved as CurrentLatch"),
            TestStep("5c", "Preparing Latch-State: If CurrentLatch is False, skip steps 5d to 5h"),
            TestStep("5d", "If LatchControlModes Bit 1 = 0, skip step 5e"),
            TestStep("5e", "Send MoveTo command with Latch = False", "Receive SUCCESS response from the DUT"),
            TestStep("5f", "If LatchControlModes Bit 1 = 1, skip step 5g"),
            TestStep("5g", "Unlatch the device manually"),
            TestStep("5h", "Wait until a subscription report with OverallCurrentState.Latch is received",
                     "OverallCurrentState.Latch should be False"),
            TestStep("5i", "Send MoveTo command with Position = MoveToFullyOpen", "Receive SUCCESS response from the DUT"),
            TestStep("5j", "If LatchControlModes Bit 0 = 0, skip step 5k and 5l"),
            TestStep("5k", "Send MoveTo command with Positon = MoveToFullyOpen and Latch = True",
                     "Receive SUCCESS response from the DUT"),
            TestStep("5l", "Wait until a subscription report with OverallCurrentState.Latch is received",
                     "OverallCurrentState.Latch should be True"),
            TestStep("5m", "If LatchControlModes Bit 0 = 1, skip step 5n"),
            TestStep("5n", "Send MoveTo command with Positon = MoveToFullyOpen and Latch = True",
                     "Receive INVALID_IN_STATE response from the DUT"),
            TestStep("6a", "Check SP and LT feature support",
                     "Skip steps 6b to 6h if the LT feature is supported or the SP feature is not supported"),
            TestStep("6b", "Read the OverallCurrentState attribute",
                     "OverallCurrentState of the ClosureControl cluster is returned by the DUT; Speed field is saved as CurrentSpeed"),
            TestStep("6c", "Preparing Speed-State: If CurrentSpeed is High, skip steps 6d and 6e"),
            TestStep("6d", "Send MoveTo command with Speed = High", "Receive SUCCESS response from the DUT"),
            TestStep("6e", "Wait until a subscription report with OverallCurrentState.Speed is received",
                     "OverallCurrentState.Speed should be High"),
            TestStep("6f", "Send MoveTo command with Latch = True", "Receive SUCCESS response from the DUT"),
            TestStep("6g", "Send MoveTo command with Latch = True and Speed = Low", "Receive SUCCESS response from the DUT"),
            TestStep("6h", "Wait until a subscription report with OverallCurrentState.Speed is received",
                     "OverallCurrentState.Speed should be Low"),
            TestStep("7a", "If the PS feature is supported, send MoveTo command with Position = 6",
                     "Receive CONSTRAINT_ERROR response from the DUT"),
            TestStep("7b", "If the LT feature is supported, send MoveTo command with Latch = 2",
                     "Receive CONSTRAINT_ERROR response from the DUT"),
            TestStep("7c", "If the SP feature is supported, send MoveTo command with Speed = 4",
                     "Receive CONSTRAINT_ERROR response from the DUT"),
            TestStep("7d", "If the SP feature is supported, send MoveTo command with Position = 6 and Speed = High",
                     "Receive CONSTRAINT_ERROR response from the DUT"),
            TestStep("7e", "If the SP feature is supported, send MoveTo command with Position = MoveToFullyClosed and Speed = 4",
                     "Receive CONSTRAINT_ERROR response from the DUT"),
        ]
        return steps

    def pics_TC_CLCTRL_4_3(self) -> list[str]:
        pics = [
            "CLCTRL.S",
        ]
        return pics

    @async_test_body
    async def test_TC_CLCTRL_4_3(self):

        endpoint = self.get_endpoint(default=1)
        timeout: uint = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout  # default_timeout = 90 seconds

        self.step(1)
        attributes: typing.List[uint] = Clusters.ClosureControl.Attributes

        self.step("2a")
        feature_map: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        is_position_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kPositioning
        is_latching_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kMotionLatching
        is_speed_supported: bool = feature_map & Clusters.ClosureControl.Bitmaps.Feature.kSpeed
        logging.info(f"FeatureMap: {feature_map}")

        self.step("2b")
        sub_handler = ClusterAttributeChangeAccumulator(Clusters.ClosureControl)
        await sub_handler.start(self.default_controller, self.dut.node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        self.step("2c")
        latch_control_modes: uint = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.LatchControlModes)
        logging.info(f"LatchControlModes: {latch_control_modes}")

        self.step("3")
        try:
            await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({}))
            logging.error("MoveTo command with no fields should have failed but succeeded")
            asserts.assert_true(False, "MoveTo command with no fields should have failed but succeeded")
        except InteractionModelError as e:
            logging.info(f"MoveTo command with no fields failed: {e}")
            asserts.assert_equal(e.status, Status.InvalidCommand,
                                 f"Expected INVALID_COMMAND status for MoveTo with no fields, but got: {e}")

        self.step("4a")
        if not is_position_supported or is_latching_supported:
            logging.info("Skipping steps 4b to 4h as Positioning feature is not supported or Latching feature is supported")
            self.skip_step("4b")
            self.skip_step("4c")
            self.skip_step("4d")
            self.skip_step("4e")
            self.skip_step("4f")
            self.skip_step("4g")
            self.skip_step("4h")
        else:
            self.step("4b")
            overall_current_state: typing.Union[Nullable, Clusters.ClosureControl.Structs.OverallCurrentStateStruct] = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallCurrentState)
            current_position: Clusters.ClosureControl.Enums.CurrentPositionEnum = None

            if overall_current_state is NullValue:
                current_position = NullValue
            else:
                current_position = overall_current_state.position
            logging.info(f"CurrentPosition: {current_position}")

            self.step("4c")
            if current_position == Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed:
                logging.info("CurrentPosition is FullyClosed, skipping steps 4d and 4e")
                self.skip_step("4d")
                self.skip_step("4e")
            else:
                self.step("4d")
                try:
                    await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Position": Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed}))
                    logging.info("MoveTo command with Position = MoveToFullyClosed sent successfully")
                except InteractionModelError as e:
                    logging.error(f"MoveTo command with Position = MoveToFullyClosed failed: {e}")
                    asserts.assert_equal(e.status, Status.Success,
                                         f"Expected Success status for MoveTo with Position = MoveToFullyClosed, but got: {e}")

                self.step("4e")
                sub_handler.await_all_expected_report_matches(expected_matches=[current_position_matcher(
                    Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyClosed)], timeout_sec=timeout)

            self.step("4f")
            try:
                await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Latch": True}))
                logging.info("MoveTo command with Latch = True sent successfully")
            except InteractionModelError as e:
                logging.error(f"MoveTo command with Latch = True failed: {e}")
                asserts.assert_equal(e.status, Status.Success,
                                     f"Expected Success status for MoveTo with Latch = True, but got: {e}")

            self.step("4g")
            try:
                await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Latch": True, "Position": Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen}))
                logging.info("MoveTo command with Latch = True and Position = MoveToFullyOpen sent successfully")
            except InteractionModelError as e:
                logging.error(f"MoveTo command with Latch = True and Position = MoveToFullyOpen failed: {e}")
                asserts.assert_equal(e.status, Status.Success,
                                     f"Expected Success status for MoveTo with Latch = True and Position = MoveToFullyOpen, but got: {e}")
            self.step("4h")
            sub_handler.await_all_expected_report_matches(expected_matches=[current_position_matcher(
                Clusters.ClosureControl.Enums.CurrentPositionEnum.kFullyOpened)], timeout_sec=timeout)

        self.step("5a")
        if not is_latching_supported or is_position_supported:
            logging.info("Skipping steps 5b to 5n as Latching feature is not supported or Positioning feature is supported")
            self.skip_step("5b")
            self.skip_step("5c")
            self.skip_step("5d")
            self.skip_step("5e")
            self.skip_step("5f")
            self.skip_step("5g")
            self.skip_step("5h")
            self.skip_step("5i")
            self.skip_step("5j")
            self.skip_step("5k")
            self.skip_step("5l")
            self.skip_step("5m")
            self.skip_step("5n")
        else:
            self.step("5b")
            overall_current_state: typing.Union[Nullable, Clusters.ClosureControl.Structs.OverallCurrentStateStruct] = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallCurrentState)
            current_latch: bool = None
            if overall_current_state is NullValue:
                current_latch = NullValue
            else:
                current_latch = overall_current_state.latch
            logging.info(f"CurrentLatch: {current_latch}")

            self.step("5c")
            if not current_latch:
                logging.info("CurrentLatch is False, skipping Latch = False preparation steps")
                self.skip_step("5d")
                self.skip_step("5e")
                self.skip_step("5f")
                self.skip_step("5g")
                self.skip_step("5h")
            else:
                logging.info("CurrentLatch is True, proceeding with Latch = False preparation steps")

                self.step("5d")
                # Check if LatchControlModes Bit 1 is 0
                if int(bin(latch_control_modes)[1]) == 0:
                    self.skip_step("5e")
                    self.step("5f")
                    logging.info("LatchControlModes Bit 1 is 0, unlatch device manually")
                    self.step("5g")
                    self.wait_for_user_input(prompt_msg="Press enter when the device is unlatched")

                else:
                    self.step("5e")
                    logging.info("LatchControlModes Bit 1 is 1, sending MoveTo command with Latch = False")

                    try:
                        await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Latch": False}))
                    except InteractionModelError as e:
                        asserts.assert_equal(e.status, Status.Success, f"MoveTo command with Latch = False failed: {e}")

                    self.step("5f")
                    self.skip_step("5g")

                self.step("5h")
                sub_handler.await_all_expected_report_matches(expected_matches=[current_latch_matcher(False)], timeout_sec=timeout)
                logging.info("Latch is now False, proceeding with unsupported feature checks")

            self.step("5i")
            try:
                await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Position": Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen}))
                logging.info("MoveTo command with Position = MoveToFullyOpen sent successfully")
            except InteractionModelError as e:
                logging.error(f"MoveTo command with Position = MoveToFullyOpen failed: {e}")
                asserts.assert_equal(e.status, Status.Success,
                                     f"Expected Success status for MoveTo with Position = MoveToFullyOpen, but got: {e}")

            self.step("5j")
            if int(bin(latch_control_modes)[0]) == 0:
                logging.info("LatchControlModes Bit 0 is 0, skipping steps 5k and 5l")
                self.skip_step("5k")
                self.skip_step("5l")
            else:
                self.step("5k")
                try:
                    await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Position": Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen, "Latch": True}))
                    logging.info("MoveTo command with Position = MoveToFullyOpen and Latch = True sent successfully")
                except InteractionModelError as e:
                    logging.error(f"MoveTo command with Position = MoveToFullyOpen and Latch = True failed: {e}")
                    asserts.assert_equal(e.status, Status.Success,
                                         f"Expected Success status for MoveTo with Position = MoveToFullyOpen and Latch = True, but got: {e}")

                self.step("5l")
                sub_handler.await_all_expected_report_matches(expected_matches=[current_latch_matcher(True)], timeout_sec=timeout)
            self.step("5m")
            if int(bin(latch_control_modes)[0]) == 1:
                logging.info("LatchControlModes Bit 0 is 1, skipping step 5n")
                self.skip_step("5n")
            else:
                self.step("5n")
                try:
                    await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Position": Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyOpen, "Latch": True}))
                    logging.error("MoveTo command with Position = MoveToFullyOpen and Latch = True should have failed but succeeded")
                    asserts.assert_true(
                        False, "MoveTo command with Position = MoveToFullyOpen and Latch = True should have failed but succeeded")
                except InteractionModelError as e:
                    logging.info(f"Expected exception caught for MoveTo with Position = MoveToFullyOpen and Latch = True: {e}")
                    asserts.assert_equal(e.status, Status.InvalidInState,
                                         f"Expected INVALID_IN_STATE for MoveTo with Position = MoveToFullyOpen and Latch = True but got: {e}")

        self.step("6a")
        if not is_speed_supported or is_latching_supported:
            logging.info("Skipping steps 6b to 6h as Speed feature is not supported or Latching feature is supported")
            self.skip_step("6b")
            self.skip_step("6c")
            self.skip_step("6d")
            self.skip_step("6e")
            self.skip_step("6f")
            self.skip_step("6g")
            self.skip_step("6h")
        else:
            self.step("6b")
            overall_current_state: typing.Union[Nullable, Clusters.ClosureControl.Structs.OverallCurrentStateStruct] = await self.read_clctrl_attribute_expect_success(endpoint=endpoint, attribute=attributes.OverallCurrentState)
            current_speed: Clusters.ClosureControl.Enums.CurrentSpeedEnum = None
            if overall_current_state is NullValue:
                current_speed = NullValue
            else:
                current_speed = overall_current_state.speed
            logging.info(f"CurrentSpeed: {current_speed}")

            self.step("6c")
            if current_speed == Clusters.ClosureControl.Enums.CurrentSpeedEnum.kHigh:
                logging.info("CurrentSpeed is High, skipping steps 6d and 6e")
                self.skip_step("6d")
                self.skip_step("6e")
            else:
                self.step("6d")
                try:
                    await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Speed": Clusters.Globals.ThreeLevelAutoEnum.kHigh}))
                    logging.info("MoveTo command with Speed = High sent successfully")
                except InteractionModelError as e:
                    logging.error(f"MoveTo command with Speed = High failed: {e}")
                    asserts.assert_equal(e.status, Status.Success,
                                         f"Expected Success status for MoveTo with Speed = High, but got: {e}")

                self.step("6e")
                sub_handler.await_all_expected_report_matches(expected_matches=[current_speed_matcher(
                    Clusters.ClosureControl.Enums.CurrentSpeedEnum.kHigh)], timeout_sec=timeout)

            self.step("6f")
            try:
                await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Latch": True}))
                logging.info("MoveTo command with Latch = True sent successfully")
            except InteractionModelError as e:
                logging.error(f"MoveTo command with Latch = True failed: {e}")
                asserts.assert_equal(e.status, Status.Success,
                                     f"Expected Success status for MoveTo with Latch = True, but got: {e}")
            self.step("6g")
            try:
                await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Latch": True, "Speed": Clusters.Globals.ThreeLevelAutoEnum.kLow}))
                logging.info("MoveTo command with Latch = True and Speed = Low sent successfully")
            except InteractionModelError as e:
                logging.error(f"MoveTo command with Latch = True and Speed = Low failed: {e}")
                asserts.assert_equal(e.status, Status.Success,
                                     f"Expected Success status for MoveTo with Latch = True and Speed = Low, but got: {e}")
            self.step("6h")
            sub_handler.await_all_expected_report_matches(expected_matches=[current_speed_matcher(
                Clusters.ClosureControl.Enums.CurrentSpeedEnum.kLow)], timeout_sec=timeout)

        if is_position_supported:
            self.step("7a")
            try:
                await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Position": 6}))
                logging.error("MoveTo command with Position = 6 should have failed but succeeded")
                asserts.assert_true(False, "MoveTo command with Position = 6 should have failed but succeeded")
            except InteractionModelError as e:
                logging.info(f"Expected exception caught for MoveTo with Position = 6: {e}")
                asserts.assert_equal(e.status, Status.ConstraintError,
                                     f"Expected CONSTRAINT_ERROR for MoveTo with Position = 6 but got: {e}")
        else:
            logging.info("Skipping step 7a as Positioning feature is not supported")
            self.skip_step("7a")

        if is_latching_supported:
            self.step("7b")
            try:
                await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Latch": 2}))
                logging.error("MoveTo command with Latch = 2 should have failed but succeeded")
                asserts.assert_true(False, "MoveTo command with Latch = 2 should have failed but succeeded")
            except InteractionModelError as e:
                logging.info(f"Expected exception caught for MoveTo with Latch = 2: {e}")
                asserts.assert_equal(e.status, Status.ConstraintError,
                                     f"Expected CONSTRAINT_ERROR for MoveTo with Latch = 2 but got: {e}")
        else:
            logging.info("Skipping step 7b as Latching feature is not supported")
            self.skip_step("7b")

        if is_speed_supported:
            self.step("7c")
            try:
                await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Speed": 4}))
                logging.error("MoveTo command with Speed = 4 should have failed but succeeded")
                asserts.assert_true(False, "MoveTo command with Speed = 4 should have failed but succeeded")
            except InteractionModelError as e:
                logging.info(f"Expected exception caught for MoveTo with Speed = 4: {e}")
                asserts.assert_equal(e.status, Status.ConstraintError,
                                     f"Expected CONSTRAINT_ERROR for MoveTo with Speed = 4 but got: {e}")
            self.step("7d")
            try:
                await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Position": 6, "Speed": Clusters.Globals.ThreeLevelAutoEnum.kHigh}))
                logging.error("MoveTo command with Position = 6 and Speed = High should have failed but succeeded")
                asserts.assert_true(False, "MoveTo command with Position = 6 and Speed = High should have failed but succeeded")
            except InteractionModelError as e:
                logging.info(f"Expected exception caught for MoveTo with Position = 6 and Speed = High: {e}")
                asserts.assert_equal(e.status, Status.ConstraintError,
                                     f"Expected CONSTRAINT_ERROR for MoveTo with Position = 6 and Speed = High but got: {e}")
            self.step("7e")
            try:
                await self.send_single_cmd(endpoint=endpoint, cluster=Clusters.ClosureControl, command=Clusters.ClosureControl.Commands.MoveTo({"Position": Clusters.ClosureControl.Enums.TargetPositionEnum.kMoveToFullyClosed, "Speed": 4}))
                logging.error("MoveTo command with Position = MoveToFullyClosed and Speed = 4 should have failed but succeeded")
                asserts.assert_true(
                    False, "MoveTo command with Position = MoveToFullyClosed and Speed = 4 should have failed but succeeded")
            except InteractionModelError as e:
                logging.info(f"Expected exception caught for MoveTo with Position = MoveToFullyClosed and Speed = 4: {e}")
                asserts.assert_equal(e.status, Status.ConstraintError,
                                     f"Expected CONSTRAINT_ERROR for MoveTo with Position = MoveToFullyClosed and Speed = 4 but got: {e}")
        else:
            self.skip_step("7c")
            self.skip_step("7d")
            self.skip_step("7e")


if __name__ == "__main__":
    default_matter_test_main()
