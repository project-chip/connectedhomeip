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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${AIR_PURIFIER_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace_file json:${TRACE_APP}.json
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
import math
from dataclasses import dataclass
from enum import Enum
import operator
from typing import Any

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_infrastructure.chip.testing.matter_testing import (AttributeValueExpected, ClusterAttributeChangeAccumulator,
                                                                       ComparisonEnum, MatterBaseTest, TestStep, async_test_body,
                                                                       default_matter_test_main)
from mobly import asserts

logger = logging.getLogger(__name__)


class ActionEnum(Enum):
    Write = 1
    Command = 2

class OrderEnum(Enum):
    Ascending = 1
    Descending = 2

@dataclass
class Update:
    action: ActionEnum
    attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor = None
    attribute_value: Any = None
    step: Clusters.FanControl.Commands.Step = None


class TC_FAN_3_5(MatterBaseTest):
    def desc_TC_FAN_3_5(self) -> str:
        return "[TC-FAN-3.5] Optional step functionality with DUT as Server"

    # def steps_TC_FAN_3_5(self):
    #     return [TestStep("1", "[FC] Commissioning already done.", is_commissioning=True),
    #             TestStep("2", "[FC] TH checks the DUT for support of the Step and MultiSpeed features.",
    #                      "If the DUT does not support both features, the test is skipped."),
    #             TestStep("3", "[FC] TH reads from the DUT the SpeedMax attribute.", "Store value for future reference."),

    #             TestStep("4", "[FC] TH subscribes to the FanControl cluster.", "Enables the TH to recei
    #             ]

    async def read_setting(self, attribute: Any) -> Any:
        """
        Asynchronously reads a specified attribute from the FanControl cluster at a given endpoint.

        Args:
            attribute (Any): The attribute to be read.

        Returns:
            Any: The value of the specified attribute if the read operation is successful.

        Raises:
            AssertionError: If the read operation fails.
        """
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def write_setting(self, attribute, value) -> None:
        """
        Writes a specified value to a given attribute on the device under test (DUT).
        It verifies that the write operation is successful by asserting the status of the response.

        Args:
            attribute (Callable): The attribute to be written, represented as a callable that takes the value.
            value (Any): The value to write to the specified attribute.

        Returns:
            None

        Raises:
            AssertionError: If the write operation fails or the response status is not successful.
        """
        logger.info(f"[FC] Writing to the {attribute.__name__} attribute, value: {value}")
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, attribute(value))])
        asserts.assert_equal(result[0].Status, Status.Success, f"[FC] {attribute.__name__} attribute write faield.")

    async def send_step_command(self, step: Clusters.Objects.FanControl.Commands.Step) -> None:
        try:
            logger.info(
                f"[FC] Sending Step command - direction: {step.direction.name}, wrap: {step.wrap}, lowestOff: {step.lowestOff}")
            await self.send_single_cmd(step, endpoint=self.endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, f"[FC] Unexpected error returned ({e})")
            pass

    async def send_on_off_command(self, cmd: Clusters.ClusterObjects.ClusterCommand) -> None:
        try:
            logger.info(
                f"[FC] Sending OnOff command: {cmd}")
            await self.send_single_cmd(cmd, endpoint=self.endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, f"[FC] Unexpected error returned ({e})")
            pass

    def verify_expected_value(self, attribute, expected_value) -> None:
        """
        Gets and verifies a given attribute report value against an expected value.

        Args:
            attribute: The attribute to be verified.
            expected_value: The expected value of the attribute.

        Raises:
            AssertionError: If the current attribute value does not match the expected value.
            AssertionError: If the attribute is not found in the attribute reports.
        """
        # Gets and verifies a given attribute report value against an expected value.
        value_current = self.attribute_subscription.get_last_attribute_report_value(self.endpoint, attribute, self.timeout_sec)
        if value_current is not None:
            asserts.assert_equal(value_current, expected_value,
                                 f"Current {attribute.__name__} attribute value ({value_current}) is not equal to the expected value ({expected_value})")
        else:
            asserts.fail(f"The {attribute.__name__} attribute was not found in the attribute reports")

    async def update_and_verify_attribute_values(self, update: Update, expect_updates: bool, expected_attributes=None) -> None:
        if expect_updates:
            self.attribute_subscription.reset()

        if update.action == ActionEnum.Write:
            await self.write_setting(update.attribute, update.attribute_value)
        elif update.action == ActionEnum.Command:
            await self.send_step_command(update.step)

        if expect_updates:
            self.attribute_subscription.await_all_final_values_reported_threshold(
                value_expectations=expected_attributes, timeout_sec=self.timeout_sec)
        else:
            for expected_attribute in expected_attributes:
                self.verify_expected_value(expected_attribute.attribute, expected_attribute.threshold_value)

    def compute_percent_setting(self, speed_setting):
        """
        Computes the fan's percent based on a given speed setting.

        This method calculates the percentage representation of the current fan speed
        relative to the maximum speed. The result is rounded down to the nearest integer.

        Args:
            speed_setting (float): The current speed setting of the fan.

        Returns:
            int: The corresponding percent value.
        """
        return math.floor((speed_setting / self.speed_max) * 100)

    async def subscribe_to_attributes(self) -> None:
        cluster = Clusters.FanControl
        attr = cluster.Attributes

        self.subscriptions = [
            ClusterAttributeChangeAccumulator(cluster, attr.PercentSetting),
            ClusterAttributeChangeAccumulator(cluster, attr.SpeedSetting),
            ClusterAttributeChangeAccumulator(cluster, attr.FanMode)
        ]

        for sub in self.subscriptions:
            await sub.start(self.default_controller, self.dut_node_id, self.endpoint)

    def verify_attribute_progression(self, step: Clusters.FanControl.Commands.Step) -> None:
        sd_enum = Clusters.FanControl.Enums.StepDirectionEnum
        order_str = "ascending" if step.direction == sd_enum.kIncrease else "descending"

        # Setup
        comp = operator.le if step.direction == sd_enum.kIncrease else operator.ge
        comp_str = "greater" if step.direction == sd_enum.kIncrease else "less"
        shared_str = f"not all attribute values progressed in {order_str} order (current value {comp_str} than previous value)."

        for sub in self.subscriptions:
            values = [q.value for q in sub.attribute_queue.queue]
            correct_progression = all(comp(a, b) for a, b in zip(values, values[1:]))
            asserts.assert_true(correct_progression, f"[FC] {sub._expected_attribute.__name__}: {shared_str}")

    def verify_expected_reports(self, step: Clusters.FanControl.Commands.Step, percent_setting_init) -> tuple:
        percent_setting_report_qty = 0
        fan_mode_report_qty = 0
        speed_setting_report_qty = 0
        cluster = Clusters.FanControl
        sd_enum = cluster.Enums.StepDirectionEnum
        fm_enum = cluster.Enums.FanModeEnum

        # - Count the number of each of the subscription reports
        # - Get the values of the FanMode and SpeedSetting attributes
        for sub in self.subscriptions:
            if sub._expected_attribute == Clusters.FanControl.Attributes.PercentSetting:
                percent_setting_report_qty = len(sub.attribute_queue.queue)
                percent_setting_values_produced = [q.value for q in sub.attribute_queue.queue]
                sub.log_queue()
            if sub._expected_attribute == Clusters.FanControl.Attributes.FanMode:
                fan_mode_report_qty = len(sub.attribute_queue.queue)
                fan_mode_values_produced = [q.value for q in sub.attribute_queue.queue]
                sub.log_queue()
            if sub._expected_attribute == Clusters.FanControl.Attributes.SpeedSetting:
                speed_setting_report_qty = len(sub.attribute_queue.queue)
                sub.log_queue()
                speed_setting_values_produced = [q.value for q in sub.attribute_queue.queue]

        # Get the expected FanMode and SpeedSetting values to verify all are present
        increase_step = step.direction == sd_enum.kIncrease
        speed_max_range = range(0, self.speed_max + 1)
        fan_mode_remove = fm_enum.kOff if percent_setting_init == 0 else fm_enum.kHigh
        speed_setting_remove = 0 if percent_setting_init == 0 else self.speed_max
        trim = slice(None, -1) if (step.direction == sd_enum.kDecrease and not step.lowestOff) else slice(None)
        fan_modes_prep = [x for x in self.fan_modes if x != fan_mode_remove]
        speed_setting_prep = [x for x in speed_max_range if x != speed_setting_remove]
        fan_modes_expected = fan_modes_prep[trim] if increase_step else list(reversed(fan_modes_prep))[trim]
        speed_setting_expected = speed_setting_prep[trim] if increase_step else list(reversed(speed_setting_prep))[trim]

        logging.info(f"[FC] percent_setting_values_produced: {percent_setting_values_produced}")

        logging.info(f"[FC] fan_modes_expected: {fan_modes_expected}")
        logging.info(f"[FC] fan_mode_values_produced: {fan_mode_values_produced}")

        logging.info(f"[FC] speed_setting_expected: {speed_setting_expected}")
        logging.info(f"[FC] speed_setting_values_produced: {speed_setting_values_produced}")

        # If the number of PercentSetting reports is greater than the number of FanMode reports,
        # - Verify that all the expected FanMode values are present in the reports
        if percent_setting_report_qty > fan_mode_report_qty:
            missing_fan_modes = [mode for mode in fan_modes_expected if mode not in fan_mode_values_produced]
            asserts.assert_equal(fan_modes_expected, fan_mode_values_produced,
                                 f"[FC] Some of the expected FanMode values are not present in the reports. Expected: {fan_modes_expected}, missing: {missing_fan_modes}.")

        # If the number of PercentSetting reports is greater or equal than the number of SpeedSetting reports,
        # - Verify that all the expected SpeedSetting values are present in the reports
        if percent_setting_report_qty >= speed_setting_report_qty:
            missing_speed_setting = [speed for speed in speed_setting_expected if speed not in speed_setting_values_produced]
            asserts.assert_equal(speed_setting_expected, speed_setting_values_produced,
                                 f"[FC] Some of the expected SpeedSetting values are not present in the reports. Expected: {speed_setting_expected}, missing: {missing_speed_setting}.")

        # - Saving baseline attribute values both in ascending and descending order
        #   as per the Step command direction to later verify that they're the same
        #   when the Step command is executed in the opposite direction.
        # - We remove the last element of each list as it represents the initialization
        #   value of the sequence from the opposite direction, which is not considered
        #   in the reports.
        if step.direction == sd_enum.kDecrease and step.lowestOff:
            self.baseline_percent_setting_desc = percent_setting_values_produced[:-1]
            self.baseline_fan_mode_desc = fan_mode_values_produced[:-1]
            self.baseline_speed_setting_desc = speed_setting_values_produced[:-1]

        if step.direction == sd_enum.kIncrease and step.lowestOff:
            self.baseline_percent_setting_asc = percent_setting_values_produced[:-1]
            self.baseline_fan_mode_asc = fan_mode_values_produced[:-1]
            self.baseline_speed_setting_asc = speed_setting_values_produced[:-1]

    async def get_fan_modes(self, remove_auto: bool = False):
        # Read FanModeSequence attribute value
        fan_mode_sequence_attr = Clusters.FanControl.Attributes.FanModeSequence
        fm_enum = Clusters.FanControl.Enums.FanModeEnum
        fms_enum = Clusters.FanControl.Enums.FanModeSequenceEnum
        self.fan_mode_sequence = await self.read_setting(fan_mode_sequence_attr)

        # Verify response contains a FanModeSequenceEnum
        asserts.assert_is_instance(self.fan_mode_sequence, fms_enum,
                                   f"[FC] FanModeSequence result isn't of enum type {fms_enum.__name__}")

        fan_modes = None
        if self.fan_mode_sequence == 0:
            fan_modes = [fm_enum.kOff, fm_enum.kLow, fm_enum.kMedium, fm_enum.kHigh]
        elif self.fan_mode_sequence == 1:
            fan_modes = [fm_enum.kOff, fm_enum.kLow, fm_enum.kHigh]
        elif self.fan_mode_sequence == 2:
            fan_modes = [fm_enum.kOff, fm_enum.kLow, fm_enum.kMedium, fm_enum.kHigh, fm_enum.kAuto]
        elif self.fan_mode_sequence == 3:
            fan_modes = [fm_enum.kOff, fm_enum.kLow, fm_enum.kHigh, fm_enum.kAuto]
        elif self.fan_mode_sequence == 4:
            fan_modes = [fm_enum.kOff, fm_enum.kHigh, fm_enum.kAuto]
        elif self.fan_mode_sequence == 5:
            fan_modes = [fm_enum.kOff, fm_enum.kHigh]

        self.fan_modes = [f for f in fan_modes if not (remove_auto and f == fm_enum.kAuto)]

    async def wrap_veirfy(self, step: Clusters.FanControl.Commands.Step, percent_setting_expected, fan_mode_expected = None, speed_setting_expected = None) -> None:
        cluster = Clusters.FanControl
        attr = cluster.Attributes

        await self.send_step_command(step)

        percent_setting = await self.read_setting(attr.PercentSetting)
        asserts.assert_equal(percent_setting, percent_setting_expected,
                                f"[FC] PercentSetting attribute value ({percent_setting}) is not equal to the expected value ({percent_setting_expected}).")

        if fan_mode_expected is not None:
            fan_mode = await self.read_setting(attr.FanMode)
            asserts.assert_equal(fan_mode, fan_mode_expected,
                                    f"[FC] FanMode attribute value ({fan_mode}) is not equal to the expected value ({fan_mode_expected}).")

        if speed_setting_expected is not None:
            speed_setting = await self.read_setting(attr.SpeedSetting)
            asserts.assert_equal(speed_setting, speed_setting_expected,
                                    f"[FC] SpeedSetting attribute value ({speed_setting}) is not equal to the expected value ({speed_setting_expected}).")        

    async def initialize_and_verify_attribtutes(self, step: Clusters.FanControl.Commands.Step) -> None:
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        sd_enum = cluster.Enums.StepDirectionEnum
        fm_enum = cluster.Enums.FanModeEnum

        # Initialize PercentSetting
        percent_setting_max = 100
        percent_setting_init = 0 if step.direction == sd_enum.kIncrease else percent_setting_max
        if step.wrap:
            if step.direction == sd_enum.kDecrease:
                percent_setting_init = self.percent_setting_per_step
            else:
                percent_setting_init = percent_setting_max
        await self.write_setting(attr.PercentSetting, percent_setting_init)

        # Verify PercentSetting initialization value
        percent_setting = await self.read_setting(attr.PercentSetting)
        percent_setting_expected = percent_setting_init
        asserts.assert_equal(percent_setting, percent_setting_expected,
                            f"[FC] PercentSetting attribute value ({percent_setting}) is not equal to the expected value ({percent_setting_expected})")

        # As there's no direct relationship between the Step command and the speed-oriented attributes
        # (PercentSetting, FanMode, and SpeenSetting) due to it being implementation specific, when testing the
        # Step command with Wrap=True, Direction=Decrease, and initializing PercentSetting at 100, the expected
        # FanMode and SpeedSetting values are unknown. In this particular scenario, we skip their verification.
        # 
        # For all other scenarios, verify FanMode and SpeedSetting are at their expected values
        if not step.wrap:
            if not step.direction == sd_enum.kDecrease:
                fan_mode = await self.read_setting(attr.FanMode)
                fan_mode_expected = fm_enum.kHigh if percent_setting_init == percent_setting_max else fm_enum.kOff
                speed_setting = await self.read_setting(attr.SpeedSetting)
                speed_setting_expected = self.speed_max if percent_setting_init == percent_setting_max else 0

                asserts.assert_equal(fan_mode, fan_mode_expected,
                                    f"[FC] FanMode attribute value ({fan_mode}) is not equal to the expected value ({fan_mode_expected})")
                asserts.assert_equal(speed_setting, speed_setting_expected,
                                    f"[FC] SpeedSetting attribute value ({speed_setting}) is not equal to the expected value ({speed_setting_expected})")
        
        return percent_setting_init

    async def wrap_test(self, step: Clusters.FanControl.Commands.Step):
        cluster = Clusters.FanControl
        fm_enum = cluster.Enums.FanModeEnum
        sd_enum = cluster.Enums.StepDirectionEnum

        # Initialize and verify attributes
        await self.initialize_and_verify_attribtutes(step)

        # Reset subscriptions
        for sub in self.subscriptions: sub.reset()

        logging.info(f"[FC]")
        if step.direction == sd_enum.kDecrease and step.lowestOff:
            logging.info(f"[FC] step.direction == sd_enum.kDecrease and step.lowestOff")
            await self.wrap_veirfy(step, percent_setting_expected=0, fan_mode_expected=fm_enum.kOff, speed_setting_expected=0)
            await self.wrap_veirfy(step, percent_setting_expected=100, fan_mode_expected=fm_enum.kHigh, speed_setting_expected=self.speed_max)
        elif step.direction == sd_enum.kDecrease and not step.lowestOff:
            logging.info(f"[FC] step.direction == sd_enum.kDecrease and not step.lowestOff")
            await self.wrap_veirfy(step, percent_setting_expected=100, fan_mode_expected=fm_enum.kHigh, speed_setting_expected=self.speed_max)
        elif step.direction == sd_enum.kIncrease and step.lowestOff:
            logging.info(f"[FC] step.direction == sd_enum.kIncrease and step.lowestOff")
            await self.wrap_veirfy(step, percent_setting_expected=0)
        elif step.direction == sd_enum.kIncrease and not step.lowestOff:
            logging.info(f"[FC] step.direction == sd_enum.kIncrease and not step.lowestOff")
            await self.wrap_veirfy(step, percent_setting_expected=self.percent_setting_per_step)

    async def lowest_off_test(self, step: Clusters.FanControl.Commands.Step):
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        sd_enum = cluster.Enums.StepDirectionEnum
        percent_setting_sub = next((sub for sub in self.subscriptions if sub._expected_attribute == attr.PercentSetting), None)

        # Initialize and verify attributes
        percent_setting_init = await self.initialize_and_verify_attribtutes(step)

        # Reset subscriptions
        for sub in self.subscriptions: sub.reset()

        # Send the Step command iteratively until the expected PercentSetting value is reached
        min_percent_setting = 0 if step.lowestOff else self.percent_setting_per_step
        percent_setting_expected = 100 if step.direction == sd_enum.kIncrease else min_percent_setting
        for i in range(100):
            await self.send_step_command(step)
            percent_setting = percent_setting_sub.get_last_attribute_report_value(self.endpoint, attr.PercentSetting, self.timeout_sec)
            logging.info(f"[FC] PercentSetting attribute report value: {percent_setting}")

            # Calculate the PercentSetting range per Step
            #  - The first run of this function sets PercentSetting to 100,
            #    then sends the Step command with direction = Decrease.
            #  - Within the first iteration of this loop, we determine the
            #    range of PercentSetting values that are covered by a single
            #    Step command execution.
            if self.percent_setting_per_step is None:
                if step.direction == sd_enum.kDecrease:
                    if i == 0:
                        self.percent_setting_per_step = percent_setting_init - percent_setting

            # If the expected PercentSetting value is reached, send an additional Step
            # command to veiryf the PercentSetting value stays at the expected value
            if percent_setting == percent_setting_expected:
                await self.send_step_command(step)
                percent_setting = await self.read_setting(attr.PercentSetting)
                asserts.assert_equal(percent_setting, percent_setting_expected,
                                    f"[FC] PercentSetting attribute value ({percent_setting}) is not equal to the expected value ({percent_setting_expected})")
                break
            else:
                if i == 100:
                    asserts.fail(f"[FC] PercentSetting attribute value never reached ({percent_setting_expected}), last reported value is ({percent_setting}).")

        # Veirfy attribute progression (each successive value is greater or less than the last)
        self.verify_attribute_progression(step)

        # Veirfy all expected reports after the Step commands are present
        return self.verify_expected_reports(step, percent_setting_init)

    def pics_TC_FAN_3_5(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_5(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        cmd = cluster.Commands
        sd_enum = cluster.Enums.StepDirectionEnum
        self.timeout_sec: float = 0.5
        self.percent_setting_per_step = None

        # *** STEP 1 ***
        # Commissioning already done
        # self.step("1")

        # *** STEP 2 ***
        # If the DUT has the OnOff cluster, turn it on
        # self.step("2")
        has_on_off_cluster = await self.cluster_guard(endpoint=self.endpoint, cluster=Clusters.OnOff, skip_step=False)
        if has_on_off_cluster:
            await self.send_on_off_command(Clusters.OnOff.Commands.On())

        # *** STEP 3 ***
        # TH checks the DUT for support of the Step and MultiSpeed features
        #  - If the DUT does not support both features, the test is skipped
        # self.step("3")
        feature_map = await self.read_setting(attr.FeatureMap)
        self.supports_step = bool(feature_map & cluster.Bitmaps.Feature.kStep)
        self.supports_multi_speed = bool(feature_map & cluster.Bitmaps.Feature.kMultiSpeed)
        if not self.supports_step or not self.supports_multi_speed:
            logger.info("Both the Step and MultiSpeed features must be supported by the DUT for this test, skipping test.")
            return

        # *** STEP 4 ***
        # TH reads from the DUT the SpeedMax attribute
        #  - Store value for future reference
        # self.step("4")
        self.speed_max = await self.read_setting(attr.SpeedMax)

        # *** STEP 5 ***
        # TH reads from the DUT the FanModeSequence attribute
        # to determine the supported FanMode values
        #  - Store result for future reference
        # self.step("5")
        await self.get_fan_modes(remove_auto=True)

        # *** STEP 6 ***
        # Subscribe to the PercentSetting, FanMode, and SpeedSetting attributes individually
        # self.step("6")
        await self.subscribe_to_attributes()

        await self.lowest_off_test(cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=True))

        await self.lowest_off_test(cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=False))

        await self.lowest_off_test(cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=True))

        await self.lowest_off_test(cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=False))

        logging.info(f"[FC]")
        logging.info(f"[FC] WRAP TESTING")
        logging.info(f"[FC]")

        await self.wrap_test(cmd.Step(direction=sd_enum.kDecrease, wrap=True, lowestOff=True))

        await self.wrap_test(cmd.Step(direction=sd_enum.kDecrease, wrap=True, lowestOff=False))

        await self.wrap_test(cmd.Step(direction=sd_enum.kIncrease, wrap=True, lowestOff=True))

        await self.wrap_test(cmd.Step(direction=sd_enum.kIncrease, wrap=True, lowestOff=False))

        logging.info(f"[FC] self.baseline_percent_setting_desc: {self.baseline_percent_setting_desc}")
        logging.info(f"[FC] self.baseline_fan_mode_desc: {self.baseline_fan_mode_desc}")
        logging.info(f"[FC] self.baseline_speed_setting_desc: {self.baseline_speed_setting_desc}")

        logging.info(f"[FC] self.baseline_percent_setting_asc: {self.baseline_percent_setting_asc}")
        logging.info(f"[FC] self.baseline_fan_mode_asc: {self.baseline_fan_mode_asc}")
        logging.info(f"[FC] self.baseline_speed_setting_asc: {self.baseline_speed_setting_asc}")

        asserts.assert_equal(self.baseline_percent_setting_desc, list(reversed(self.baseline_percent_setting_asc)),
                             f"[FC] PercentSetting attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_percent_setting_desc}, Ascending: {self.baseline_percent_setting_asc}.")
        asserts.assert_equal(self.baseline_fan_mode_desc, list(reversed(self.baseline_fan_mode_asc)),
                             f"[FC] FanMode attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_fan_mode_desc}, Ascending: {self.baseline_fan_mode_asc}.")
        asserts.assert_equal(self.baseline_speed_setting_desc, list(reversed(self.baseline_speed_setting_asc)),
                             f"[FC] SpeedSetting attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_speed_setting_desc}, Ascending: {self.baseline_speed_setting_asc}.")

if __name__ == "__main__":
    default_matter_test_main()
