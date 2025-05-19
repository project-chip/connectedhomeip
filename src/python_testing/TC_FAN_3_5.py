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
import operator
from typing import Any, Callable, Optional

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_infrastructure.chip.testing.matter_testing import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep,
                                                                       async_test_body, default_matter_test_main)
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_FAN_3_5(MatterBaseTest):
    def desc_TC_FAN_3_5(self) -> str:
        return "[TC-FAN-3.5] Optional step functionality with DUT as Server"

    def steps_TC_FAN_3_5(self):
        return [TestStep(1, "[FC] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[FC] Action_2.", "Check."),
                TestStep(3, "[FC] Action_3.", "Check."),
                TestStep(4, "[FC] Action_4.", "Check."),
                TestStep(5, "[FC] Action_5.", "Check."),
                TestStep(6, "[FC] Action_6.", "Check."),
                TestStep(7, "[FC] Action_7.", "Check."),
                TestStep(8, "[FC] Action_8.", "Check."),
                TestStep(9, "[FC] Action_9.", "Check."),
                TestStep(10, "[FC] Action_10.", "Check."),
                TestStep(11, "[FC] Action_11.", "Check."),
                TestStep(12, "[FC] Action_12.", "Check."),
                TestStep(13, "[FC] Action_13.", "Check."),
                TestStep(14, "[FC] Action_14.", "Check."),
                TestStep(15, "[FC] Action_15.", "Check."),
                TestStep(16, "[FC] Action_16.", "Check."),
                TestStep(17, "[FC] Action_17.", "Check."),
                TestStep(18, "[FC] Action_18.", "Check."),
                TestStep(19, "[FC] Action_19.", "Check."),
                TestStep(20, "[FC] Action_20.", "Check."),
                TestStep(21, "[FC] Action_21.", "Check."),
                TestStep(22, "[FC] Action_22.", "Check."),
                TestStep(23, "[FC] Action_23.", "Check."),
                TestStep(24, "[FC] Action_24.", "Check."),
                TestStep(25, "[FC] Action_25.", "Check."),
                TestStep(26, "[FC] Action_26.", "Check."),
                TestStep(27, "[FC] Action_27.", "Check."),
                TestStep(28, "[FC] Action_28.", "Check."),
                TestStep(29, "[FC] Action_29.", "Check."),
                TestStep(30, "[FC] Action_30.", "Check."),
                TestStep(31, "[FC] Action_31.", "Check."),
                TestStep(32, "[FC] Action_32.", "Check."),
                TestStep(33, "[FC] Action_33.", "Check."),
                TestStep(34, "[FC] Action_34.", "Check."),
                TestStep(35, "[FC] Action_35.", "Check."),
                ]

    async def read_setting(self, attribute: Any) -> Any:
        """Read a specified attribute from the FanControl cluster.

        Args:
            attribute (Any): Attribute to read.

        Returns:
            Any: Value of the specified attribute.

        Raises:
            AssertionError: If the read operation fails.
        """
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def write_setting(self, attribute: Callable[[Any], Any], value: Any) -> None:
        """Write a value to an attribute on the DUT and verify the write is successful.

        Args:
            attribute (Callable[[Any], Any]): Callable representing the attribute to write.
            value (Any): Value to write to the attribute.

        Raises:
            AssertionError: If the write operation fails.
        """
        logger.info(f"[FC] Writing to the {attribute.__name__} attribute, value: {value}")
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, attribute(value))])
        asserts.assert_equal(result[0].Status, Status.Success, f"[FC] {attribute.__name__} attribute write failed.")

    async def send_step_command(self, step: Clusters.Objects.FanControl.Commands.Step) -> None:
        """Sends a Step command to the DUT.

        Args:
            step (Clusters.Objects.FanControl.Commands.Step): Step command to send.

        Raises:
            AssertionError: If an unexpected error occurs during command execution.
        """
        try:
            logger.info(
                f"[FC] Sending Step command - direction: {step.direction.name}, wrap: {step.wrap}, lowestOff: {step.lowestOff}")
            await self.send_single_cmd(step, endpoint=self.endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, f"[FC] Unexpected error returned ({e})")
            pass

    async def send_on_off_command(self, cmd: Clusters.ClusterObjects.ClusterCommand) -> None:
        """Send an On/Off command to the DUT.

        Args:
            cmd (Clusters.ClusterObjects.ClusterCommand): On/Off command to send.

        Raises:
            AssertionError: If an unexpected error occurs during command execution.
        """
        try:
            logger.info(f"[FC] Sending OnOff command: {cmd}")
            await self.send_single_cmd(cmd, endpoint=self.endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, f"[FC] Unexpected error returned ({e})")
            pass

    async def subscribe_to_attributes(self, subscriptions: list) -> None:
        """Subscribe to PercentSetting, SpeedSetting, and FanMode attributes on the DUT."""
        cluster = Clusters.FanControl

        self.subscriptions = subscriptions

        for sub in self.subscriptions:
            await sub.start(self.default_controller, self.dut_node_id, self.endpoint)

    async def get_fan_modes(self, remove_auto: bool = False) -> None:
        """Determine and store supported FanMode values from FanModeSequence attribute.

        Args:
            remove_auto (bool): If True, exclude the 'Auto' FanMode from the result.
        """
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

    async def initialize_and_verify_attribtutes(self, step: Clusters.FanControl.Commands.Step) -> int:
        """Initialize PercentSetting and verify the expected corresponding attribute values.

        Args:
            step (Clusters.FanControl.Commands.Step): Step command parameters.

        Returns:
            int: The initialized PercentSetting value.
        """
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        sd_enum = cluster.Enums.StepDirectionEnum
        fm_enum = cluster.Enums.FanModeEnum
        percent_setting_max = 100

        # Initialize PercentSetting
        percent_setting_init = 0 if step.direction == sd_enum.kIncrease else percent_setting_max
        if step.wrap:
            if step.direction == sd_enum.kDecrease:
                percent_setting_init = self.percent_setting_per_step
            else:
                percent_setting_init = percent_setting_max
        await self.write_setting(attr.PercentSetting, percent_setting_init)

        # Verify PercentSetting initialization value
        await self.verify_expected_attribute_value(attr.PercentSetting, percent_setting_init)

        # - As there's no direct relationship between the Step command and the speed-oriented attributes
        #   (PercentSetting, FanMode, and SpeenSetting) due to it being implementation specific, when testing the
        #   Step command with Wrap=True, Direction=Decrease, and initializing PercentSetting at 100, the expected
        #   FanMode and SpeedSetting values are unknown. In this particular scenario, we skip their verification.
        # - For all other scenarios, verify FanMode and SpeedSetting are at their expected values
        if not step.wrap:
            if not step.direction == sd_enum.kDecrease:
                fan_mode_expected = fm_enum.kHigh if percent_setting_init == percent_setting_max else fm_enum.kOff
                speed_setting_expected = self.speed_max if percent_setting_init == percent_setting_max else 0
                await self.verify_expected_attribute_value(attr.FanMode, fan_mode_expected)
                await self.verify_expected_attribute_value(attr.SpeedSetting, speed_setting_expected)

        return percent_setting_init

    async def verify_expected_attribute_value(self, attribute: Callable, expected_value: Any) -> None:
        """Verify that a given attribute has the expected value.

        Args:
            attribute (Callable): The attribute to verify.
            expected_value (Any): The expected value.

        Raises:
            AssertionError: If the attribute value does not match the expected value.
        """
        # Reads and verifies a given attribute report value against an expected value.
        value_current = await self.read_setting(attribute)
        asserts.assert_equal(
            value_current, expected_value,
            f"Current {attribute.__name__} attribute value ({value_current}) is not equal to the expected value ({expected_value})"
        )

    async def get_initial_reports_and_values(self, step: Clusters.FanControl.Commands.Step) -> None:
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        sd_enum = cluster.Enums.StepDirectionEnum
        percent_setting_sub = next((sub for sub in self.subscriptions if sub._expected_attribute == attr.PercentSetting), None)

        # *** NEXT STEP ***
        # Initialize the PercentSetting attribute and verify the expected
        # attribute changes in acordance with the Step command parameters
        self.step(self.current_step_index + 1)
        percent_setting_init = await self.initialize_and_verify_attribtutes(step)

        # *** NEXT STEP ***
        # Reset the subscriptions to clear any previous attribute reports
        self.step(self.current_step_index + 1)
        for sub in self.subscriptions:
            sub.reset()
        self.percent_current_values_produced = []
        self.speed_current_values_produced = []

        # *** NEXT STEP ***
        # TH sends Step commands iteratively
        #  - During the loop:
        #    - Calculate the PercentSetting range per Step and store for future use.
        #      This only needs to be done once.
        #    - Verify the expected PercentSetting attribute report value is reached.
        #    - If the expected PercentSetting attribute report value is reached, send
        #      an additional Step command to veiryf that the PercentSetting attribute
        #      report value stays at the expected value, otherwise, the test is failed.
        self.step(self.current_step_index + 1)
        min_percent_setting = 0 if step.lowestOff else self.percent_setting_per_step
        percent_setting_expected = 100 if step.direction == sd_enum.kIncrease else min_percent_setting
        for i in range(100):
            await self.send_step_command(step)
            percent_setting = percent_setting_sub.get_last_attribute_report_value(
                self.endpoint, attr.PercentSetting, self.timeout_sec)
            logging.info(f"[FC] PercentSetting attribute report value: {percent_setting}")

            self.percent_current_values_produced.append(await self.read_setting(attr.PercentCurrent))
            self.speed_current_values_produced.append(await self.read_setting(attr.SpeedCurrent))

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

            # Send extra Step command to verify that the PercentSetting attribute
            # report value stays at the expected value
            if percent_setting == percent_setting_expected:
                await self.send_step_command(step)
                percent_setting = await self.read_setting(attr.PercentSetting)
                asserts.assert_equal(
                    percent_setting, percent_setting_expected,
                    f"[FC] PercentSetting attribute value ({percent_setting}) is not equal to the expected value ({percent_setting_expected})"
                )
                break
            else:
                if i == 100:
                    asserts.fail(
                        f"[FC] PercentSetting attribute value never reached ({percent_setting_expected}), last reported value is ({percent_setting})."
                    )

        return percent_setting_init

    async def lowest_off_test(self, step: Clusters.FanControl.Commands.Step) -> None:
        """Tests the `lowestOff` flag for the given Step command.

        Args:
            step (Clusters.FanControl.Commands.Step): Step command parameters.

        Returns:
            None

        Raises:
            AssertionError: If expected conditions are not met during test steps.
        """

        percent_setting_init = await self.get_initial_reports_and_values(step)

        logging.info(f"[FC] precent_current_values: {self.precent_current_values}")
        logging.info(f"[FC] speed_current_values: {self.speed_current_values}")

        # *** NEXT STEP ***
        #  Read the resulting attribute reports from each subscription after the Step commands
        #   - Verify that the attribute report values from each subscription are in the
        #     expected order in acordance with the Step command direction parameter.
        #   - If the number of PercentSetting reports is greater than the number of FanMode reports:
        #     -- Verify that all the expected FanMode values are present in the reports
        #        in acordance with the FanModeSequence attribute value.
        #   - If the number of PercentSetting reports is greater or equal than the number of SpeedSetting reports:
        #     -- Verify that all the expected SpeedSetting values are present in the reports
        #        in acordance with the SpeedMax attribute value.
        #   - Save the resulting attribute report values from each subscription as a baseline
        #     for future comparisons.
        self.step(self.current_step_index + 1)
        percent_setting_values, fan_mode_values, speed_setting_values = self.verify_expected_reports_and_progression(
            step, percent_setting_init)
        self.save_baseline_values(step, percent_setting_values, fan_mode_values, speed_setting_values)

    def verify_attribute_progression(self, step: Clusters.FanControl.Commands.Step, expected_attribute: Clusters.FanControl.Attributes, values: list) -> None:
        """Verifies that the resulting attribute report values progress in the expected order.

        Args:
            step (Clusters.FanControl.Commands.Step): Step command parameters.

        Raises:
            AssertionError: If attribute values do not progress correctly.
        """
        sd_enum = Clusters.FanControl.Enums.StepDirectionEnum
        order_str = "ascending" if step.direction == sd_enum.kIncrease else "descending"

        # Setup
        comp = operator.le if step.direction == sd_enum.kIncrease else operator.ge
        comp_str = "greater" if step.direction == sd_enum.kIncrease else "less"
        shared_str = f"not all attribute values progressed in {order_str} order (current value {comp_str} than previous value)."

        # Verify that the resulting attribute report values from each subscription are in expected order
        correct_progression = all(comp(a, b) for a, b in zip(values, values[1:]))
        asserts.assert_true(correct_progression, f"[FC] {expected_attribute.__name__}: {shared_str}")

    def verify_expected_reports_and_progression(self, step: Clusters.FanControl.Commands.Step, percent_setting_init: int) -> None:
        """Verify all expected FanMode and SpeedSetting reports are acounted for after a Step command.

        Args:
            step (Clusters.FanControl.Commands.Step): Step command parameters.
            percent_setting_init (int): Initial PercentSetting value.

        Raises:
            AssertionError: If expected reports are missing.
        """
        percent_setting_report_qty = 0
        fan_mode_report_qty = 0
        speed_setting_report_qty = 0
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        sd_enum = cluster.Enums.StepDirectionEnum
        fm_enum = cluster.Enums.FanModeEnum

        # - Count the number of each of the subscription attribute reports
        # - Get the values of the PercentSetting, FanMode, and SpeedSetting attributes
        for sub in self.subscriptions:
            if sub._expected_attribute == attr.PercentSetting:
                percent_setting_report_qty = len(sub.attribute_queue.queue)
                percent_setting_values_produced = [q.value for q in sub.attribute_queue.queue]
                self.verify_attribute_progression(step, attr.PercentSetting, percent_setting_values_produced)
                sub.log_queue()
            if sub._expected_attribute == attr.FanMode:
                fan_mode_report_qty = len(sub.attribute_queue.queue)
                fan_mode_values_produced = [q.value for q in sub.attribute_queue.queue]
                self.verify_attribute_progression(step, attr.FanMode, fan_mode_values_produced)
                sub.log_queue()
            if sub._expected_attribute == attr.SpeedSetting:
                speed_setting_report_qty = len(sub.attribute_queue.queue)
                speed_setting_values_produced = [q.value for q in sub.attribute_queue.queue]
                self.verify_attribute_progression(step, attr.SpeedSetting, speed_setting_values_produced)
                sub.log_queue()

        self.verify_attribute_progression(step, attr.PercentCurrent, self.percent_current_values_produced)
        self.verify_attribute_progression(step, attr.SpeedCurrent, self.speed_current_values_produced)

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
            asserts.assert_equal(
                fan_modes_expected, fan_mode_values_produced,
                f"[FC] Some of the expected FanMode values are not present in the reports. Expected: {fan_modes_expected}, missing: {missing_fan_modes}."
            )

        asserts.assert_equal(self.percent_current_values_produced, percent_setting_values_produced,
                             f"[FC] PercentCurrent attribute values do not match PercentSetting values. Expected: {percent_setting_values_produced}, produced: {self.percent_current_values_produced}.")

        # If the number of PercentSetting reports is greater or equal than the number of SpeedSetting reports,
        # - Verify that all the expected SpeedSetting values are present in the reports
        if percent_setting_report_qty >= speed_setting_report_qty:
            missing_speed_setting = [speed for speed in speed_setting_expected if speed not in speed_setting_values_produced]
            asserts.assert_equal(
                speed_setting_expected, speed_setting_values_produced,
                f"[FC] Some of the expected SpeedSetting values are not present in the reports. Expected: {speed_setting_expected}, missing: {missing_speed_setting}."
            )

        asserts.assert_equal(self.speed_current_values_produced, speed_setting_values_produced,
                             f"[FC] SpeedCurrent attribute values do not match SpeedSetting values. Expected: {speed_setting_values_produced}, produced: {self.speed_current_values_produced}.")

        return percent_setting_values_produced, fan_mode_values_produced, speed_setting_values_produced

    def save_baseline_values(self, step: Clusters.FanControl.Commands.Step, percent_setting_values_produced: list, fan_mode_values_produced: list, speed_setting_values_produced: list) -> None:
        """This method saves the baseline PercentSetting, FanMode, and SpeedSetting values 
        in both ascending and descending orders based on the Step command direction for
        future verification.
        The last element of each list is removed as it represents the initialization 
        value from the opposite direction, which is not considered in the reports.

        Args:
            step (Clusters.FanControl.Commands.Step): Step command parameters, including direction and lowestOff flag.
            percent_setting_values_produced (list): List of PercentSetting values produced during the Step command.
            fan_mode_values_produced (list): List of FanMode values produced during the Step command.
            speed_setting_values_produced (list): List of SpeedSetting values produced during the Step command.
        """
        sd_enum = Clusters.FanControl.Enums.StepDirectionEnum

        if step.direction == sd_enum.kDecrease and step.lowestOff:
            self.baseline_percent_setting_desc = percent_setting_values_produced[:-1]
            self.baseline_fan_mode_desc = fan_mode_values_produced[:-1]
            self.baseline_speed_setting_desc = speed_setting_values_produced[:-1]
            self.baseline_percent_current_desc = self.percent_current_values_produced[:-1]
            self.baseline_speed_current_desc = self.speed_current_values_produced[:-1]

        if step.direction == sd_enum.kIncrease and step.lowestOff:
            self.baseline_percent_setting_asc = percent_setting_values_produced[:-1]
            self.baseline_fan_mode_asc = fan_mode_values_produced[:-1]
            self.baseline_speed_setting_asc = speed_setting_values_produced[:-1]
            self.baseline_percent_current_asc = self.percent_current_values_produced[:-1]
            self.baseline_speed_current_asc = self.speed_current_values_produced[:-1]

    def verify_baseline_values(self):
        """
        Verifies that the baseline values for PercentSetting, FanMode, and SpeedSetting 
        attributes are consistent after performing Step command decrease/increase operations.

        This method checks if the descending baseline values are the reverse of the 
        ascending baseline values for each attribute. If the values do not match, 
        an assertion error is raised with a detailed message indicating the mismatch.

        Raises:
            AssertionError: If the descending baseline values do not match the reversed 
                            ascending baseline values for any of the attributes.
        """
        asserts.assert_equal(
            self.baseline_percent_setting_desc, list(reversed(self.baseline_percent_setting_asc)),
            f"[FC] PercentSetting attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_percent_setting_desc}, Ascending: {self.baseline_percent_setting_asc}."
        )
        asserts.assert_equal(
            self.baseline_fan_mode_desc, list(reversed(self.baseline_fan_mode_asc)),
            f"[FC] FanMode attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_fan_mode_desc}, Ascending: {self.baseline_fan_mode_asc}."
        )
        asserts.assert_equal(
            self.baseline_speed_setting_desc, list(reversed(self.baseline_speed_setting_asc)),
            f"[FC] SpeedSetting attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_speed_setting_desc}, Ascending: {self.baseline_speed_setting_asc}."
        )
        asserts.assert_equal(
            self.baseline_percent_current_desc, list(reversed(self.baseline_percent_current_asc)),
            f"[FC] PercentCurrent attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_percent_current_desc}, Ascending: {self.baseline_percent_current_asc}."
        )
        asserts.assert_equal(
            self.baseline_speed_current_desc, list(reversed(self.baseline_speed_current_asc)),
            f"[FC] SpeedCurrent attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_speed_current_desc}, Ascending: {self.baseline_speed_current_asc}."
        )

    async def wrap_test(self, step: Clusters.FanControl.Commands.Step) -> None:
        """Tests the `wrap` flag for the given Step command.

        Args:
            step (Clusters.FanControl.Commands.Step): Step command parameters.
        """
        cluster = Clusters.FanControl
        fm_enum = cluster.Enums.FanModeEnum
        sd_enum = cluster.Enums.StepDirectionEnum

        # *** NEXT STEP ***
        # Initialize the PercentSetting attribute and verify the expected
        # attribute changes in acordance with the Step command parameters
        self.step(self.current_step_index + 1)
        await self.initialize_and_verify_attribtutes(step)

        # *** NEXT STEP ***
        # Reset the subscriptions to clear any previous attribute reports
        self.step(self.current_step_index + 1)
        for sub in self.subscriptions:
            sub.reset()

        # *** NEXT STEP ***
        # TH sends a Step command with the reqested parameters
        #  - Verify the expected attribute values are produced
        self.step(self.current_step_index + 1)
        if step.direction == sd_enum.kDecrease and step.lowestOff:
            # - Verify that the attribute values all go to Off values
            await self.wrap_veirfy(step, percent_setting_expected=0, fan_mode_expected=fm_enum.kOff, speed_setting_expected=0)
            # - Verify that the attribute values all go to High values
            await self.wrap_veirfy(step, percent_setting_expected=100, fan_mode_expected=fm_enum.kHigh, speed_setting_expected=self.speed_max)
        elif step.direction == sd_enum.kDecrease and not step.lowestOff:
            # - Verify that the attribute values all go to High values
            await self.wrap_veirfy(step, percent_setting_expected=100, fan_mode_expected=fm_enum.kHigh, speed_setting_expected=self.speed_max)
        elif step.direction == sd_enum.kIncrease and step.lowestOff:
            # - Verify that the PercentSetting attribute value goes to 0
            await self.wrap_veirfy(step, percent_setting_expected=0)
        elif step.direction == sd_enum.kIncrease and not step.lowestOff:
            # - Verify that the PercentSetting attribute value goes to the minimum Step value above 0
            await self.wrap_veirfy(step, percent_setting_expected=self.percent_setting_per_step)

    async def wrap_veirfy(
        self,
        step: Clusters.FanControl.Commands.Step,
        percent_setting_expected: int,
        fan_mode_expected: Optional[Any] = None,
        speed_setting_expected: Optional[int] = None
    ) -> None:
        """Sends a Step command and verifies the resulting attribute values match expectations.

        Args:
            step (Clusters.FanControl.Commands.Step): Step command parameters.
            percent_setting_expected (int): Expected PercentSetting after wrap.
            fan_mode_expected (Optional[Any]): Expected FanMode (if any).
            speed_setting_expected (Optional[int]): Expected SpeedSetting (if any).

        Raises:
            AssertionError: If any attribute value does not match its expected value.
        """
        cluster = Clusters.FanControl
        attr = cluster.Attributes

        await self.send_step_command(step)

        percent_setting = await self.read_setting(attr.PercentSetting)
        asserts.assert_equal(
            percent_setting, percent_setting_expected,
            f"[FC] PercentSetting attribute value ({percent_setting}) is not equal to the expected value ({percent_setting_expected})."
        )

        if fan_mode_expected is not None:
            fan_mode = await self.read_setting(attr.FanMode)
            asserts.assert_equal(
                fan_mode, fan_mode_expected,
                f"[FC] FanMode attribute value ({fan_mode}) is not equal to the expected value ({fan_mode_expected})."
            )

        if speed_setting_expected is not None:
            speed_setting = await self.read_setting(attr.SpeedSetting)
            asserts.assert_equal(
                speed_setting, speed_setting_expected,
                f"[FC] SpeedSetting attribute value ({speed_setting}) is not equal to the expected value ({speed_setting_expected})."
            )

    def pics_TC_FAN_3_5(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_5(self) -> None:
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        cmd = cluster.Commands
        sd_enum = cluster.Enums.StepDirectionEnum
        self.timeout_sec: float = 0.5
        self.percent_setting_per_step: Optional[int] = None
        self.precent_current_values = []
        self.speed_current_values = []

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH checks the DUT for the prescence of the OnOff cluster
        # - If the cluster is present, set it to On
        self.step(2)
        has_on_off_cluster = await self.cluster_guard(endpoint=self.endpoint, cluster=Clusters.OnOff, skip_step=False)
        if has_on_off_cluster:
            await self.send_on_off_command(Clusters.OnOff.Commands.On())

        # *** STEP 3 ***
        # TH checks the DUT for support of the Step and MultiSpeed features
        #  - If the DUT does not support both features, the test is skipped
        self.step(3)
        feature_map = await self.read_setting(attr.FeatureMap)
        self.supports_step = bool(feature_map & cluster.Bitmaps.Feature.kStep)
        self.supports_multi_speed = bool(feature_map & cluster.Bitmaps.Feature.kMultiSpeed)
        if not self.supports_step or not self.supports_multi_speed:
            logger.info("Both the Step and MultiSpeed features must be supported by the DUT for this test, skipping test.")
            return

        # *** STEP 4 ***
        # TH reads from the DUT the SpeedMax attribute
        #  - Store value for future reference
        self.step(4)
        self.speed_max = await self.read_setting(attr.SpeedMax)

        # *** STEP 5 ***
        # TH reads from the DUT the FanModeSequence attribute
        #  - Store result for future reference
        self.step(5)
        await self.get_fan_modes(remove_auto=True)

        # *** STEP 6 ***
        # TH subscribes to the PercentSetting, FanMode, and SpeedSetting attributes individually
        self.step(6)
        subscriptions = [
            ClusterAttributeChangeAccumulator(cluster, attr.PercentSetting),
            ClusterAttributeChangeAccumulator(cluster, attr.SpeedSetting),
            ClusterAttributeChangeAccumulator(cluster, attr.FanMode)
        ]
        await self.subscribe_to_attributes(subscriptions)

        # LowestOff tests
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=True))
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=False))
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=True))
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=False))

        # Wrap tests
        await self.wrap_test(cmd.Step(direction=sd_enum.kDecrease, wrap=True, lowestOff=True))
        await self.wrap_test(cmd.Step(direction=sd_enum.kDecrease, wrap=True, lowestOff=False))
        await self.wrap_test(cmd.Step(direction=sd_enum.kIncrease, wrap=True, lowestOff=True))
        await self.wrap_test(cmd.Step(direction=sd_enum.kIncrease, wrap=True, lowestOff=False))

        # *** STEP 35 ***
        # Read both the descending and ascending baseline attribute report values
        #  - Verify that the descending baseline values are the exact reverse of the
        #    ascending baseline values for each of the attributes
        # self.step(self.current_step_index + 1)
        self.step(35)
        self.verify_baseline_values()

        # # *** STEP 36 ***
        # # TH subscribes to the PercentCurrent and SpeedCurrent attributes individually
        # self.step(36)
        # subscriptions = [
        #     ClusterAttributeChangeAccumulator(cluster, attr.PercentSetting),
        #     ClusterAttributeChangeAccumulator(cluster, attr.PercentCurrent),
        #     ClusterAttributeChangeAccumulator(cluster, attr.SpeedCurrent)
        # ]
        # await self.subscribe_to_attributes(subscriptions)


if __name__ == "__main__":
    default_matter_test_main()
