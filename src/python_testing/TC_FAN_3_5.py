#
#    Copyright (c) 2023-2025 Project CHIP Authors
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
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import operator
from typing import Any, Callable, Optional

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects as ClusterObjects
from matter.interaction_model import InteractionModelError, Status
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches

logger = logging.getLogger(__name__)


class TC_FAN_3_5(MatterBaseTest):
    def desc_TC_FAN_3_5(self) -> str:
        return "[TC-FAN-3.5] Optional step functionality with DUT as Server"

    def steps_TC_FAN_3_5(self):
        return [TestStep("1", "[FC] Commissioning already done.", is_commissioning=True),
                TestStep("2", "[FC] TH checks the DUT for the presence of the OnOff cluster.",
                         "If the cluster is present, set it to On."),
                TestStep("3", "[FC] TH reads from the DUT the SpeedMax attribute.",
                         "Store value for future reference."),
                TestStep("4", "[FC] TH reads from the DUT the FanModeSequence attribute.",
                         "Store value for future reference."),
                TestStep("5", "[FC] TH determines the PercentSetting range per Step command.",
                         "- Set PercentSetting to 0. - Send Step command with direction=Increase, lowestOff=True, and wrap=False. - Read the resulting PercentSetting attribute report value. - Calculate the PercentSetting range per Step command. - Store value for future reference."),

                TestStep("6", """[FC] LowestOff field test
                                            Iteratively send step commands with LowestOff set to True, Direction set to Decrease, and Wrap set to False.
                                            Starting form the maximum fan attribute values and ending at the minimum fan attribute values (descending order).
                                            Monitoring the Setting attribute values primarily (PercentSetting, SpeedSetting).""", """
                                        SETUP
                                            - Initialize the PercentSetting attribute to 100
                                                - Verify that the SpeedSetting attribute value is set to SpeedMax
                                                - Verify that the FanMode attribute value is set to High
                                            - Subscribe to the PercentSetting, SpeedSetting, and FanMode attributes
                                            - Step: LowestOff=True, Direction=Decrease, Wrap=False"""),

                TestStep("6a", """TH sends Step commands iteratively""", """
                                        LowestOff Check
                                            - Verify that the PercentSetting attribute value reaches 0
                                        No Wrap Check
                                            - When the PercentSetting attribute value reaches 0:
                                                - Send an additional Step command and verify that PercentSetting stays at 0"""),

                TestStep("6b", """Read the resulting attribute reports from each subscription""", """
                                        - Verify that the attribute report values from each subscription are in descending order
                                        - If the number of PercentSetting reports is greater than the number of FanMode reports:
                                            - Verify that all the expected FanMode values are present in the reports in accordance with the FanModeSequence attribute value
                                        - If the number of PercentSetting reports is greater or equal than the number of SpeedSetting reports:
                                            - Verify that all the expected SpeedSetting values are present in the reports in accordance with the SpeedMax attribute value.
                                        [Save the resulting descending attribute report values from each subscription as a baseline for comparison with the ascending values from the next steps]"""),

                TestStep("7", """LowestOff field test
                                        Iteratively send step commands with LowestOff set to True, Direction set to Increase, and Wrap set to False.
                                        Starting form the minimum fan attribute values and ending at the maximum fan attribute values (ascending order).
                                        Monitoring the Setting attribute values primarily (PercentSetting, SpeedSetting).""", """
                                    SETUP
                                    - Initialize the PercentSetting attribute to 0
                                        - Verify that the SpeedSetting attribute value is set to 0
                                        - Verify that the FanMode attribute value is set to Off
                                    - Subscribe to the PercentSetting, SpeedSetting, and FanMode attributes
                                    - Step: LowestOff=True, Direction=Increase, Wrap=False"""),

                TestStep("7a", """TH sends Step commands iteratively""", """
                                        Max PercentSetting Check
                                            - Verify that the PercentSetting attribute value reaches 100
                                        No Wrap Check
                                            - When the PercentSetting attribute value reaches 100:
                                                - Send an additional Step command to verify that PercentSetting stays at 100"""),

                TestStep("7b", """Read the resulting attribute reports from each subscription""", """
                                        - Verify that the attribute report values from each subscription are in ascending order
                                        - If the number of PercentSetting reports is greater than the number of FanMode reports:
                                            - Verify that all the expected FanMode values are present in the reports in accordance with the FanModeSequence attribute value
                                        - If the number of PercentSetting reports is greater or equal than the number of SpeedSetting reports:
                                            - Verify that all the expected SpeedSetting values are present in the reports in accordance with the SpeedMax attribute value.
                                        [Save the resulting ascending attribute report values from each subscription as a baseline for comparison with the descending values from the previous steps]"""),

                TestStep("8", """Compare the descending baseline values with the ascending baseline values""",
                         """Verify that the descending baseline values are the reverse of the ascending baseline values for each attribute"""),

                TestStep("9", """LowestOff field test
                                        Iteratively send step commands with LowestOff set to True, Direction set to Decrease, and Wrap set to False.
                                        Starting form the maximum fan attribute values and ending at the minimum fan attribute values (descending order).
                                        Monitoring the Current attribute values primarily (PercentCurrent, SpeedCurrent).""", """
                                    SETUP
                                    - Initialize the PercentSetting attribute to 100
                                        - Verify that the PercentCurrent attribute value is set to 100
                                        - Verify that the SpeedCurrent attribute value is set to SpeedMax
                                    - Subscribe to the PercentSetting, PercentCurrent, and SpeedCurrent attributes
                                    - Step: LowestOff=True, Direction=Decrease, Wrap=False"""),

                TestStep("9a", """TH sends Step commands iteratively""", """
                                    LowestOff Check
                                        - Verify that the PercentSetting attribute value reaches 0
                                    No Wrap Check
                                        - When the PercentSetting attribute value reaches 0:
                                            - Send an additional Step command to verify that PercentSetting stays at 0"""),

                TestStep("9b", """Read the resulting attribute reports from each subscription""", """
                                    - Verify that the attribute report values from each subscription are in descending order
                                    - If the number of PercentSetting reports is equal to the number of PercentCurrent reports:
                                        - Verify that all the expected PercentCurrent values are present in the reports
                                    - If the number of PercentSetting reports is greater or equal than the number of SpeedCurrent reports:
                                        - Verify that all the expected SpeedCurrent values are present in the reports in accordance with the SpeedMax attribute value.
                                    [Save the resulting descending attribute report values from each subscription as a baseline for comparison with the ascending values from the next steps]"""),

                TestStep("10", """LowestOff field test
                                        Iteratively send step commands with LowestOff set to True, Direction set to Increase, and Wrap set to False.
                                        Starting form the minimum fan attribute values and ending at the maximum fan attribute values (ascending order).
                                        Monitoring the Current attribute values primarily (PercentCurrent, SpeedCurrent).""", """
                                    SETUP
                                    - Initialize the PercentCurrent attribute to 0
                                        - Verify that the SpeedCurrent attribute value is set to 0
                                    - Subscribe to the PercentSetting, PercentCurrent, and SpeedCurrent attributes
                                    - Step: LowestOff=True, Direction=Increase, Wrap=False"""),

                TestStep("10a", """TH sends Step commands iteratively""", """
                                    Max PercentSetting Check
                                        - Verify that the PercentSetting attribute value reaches 100
                                    No Wrap Check
                                        - When the PercentSetting attribute value reaches 100:
                                            - Send an additional Step command to verify that PercentSetting stays at 100"""),

                TestStep("10b", """Read the resulting attribute reports from each subscription""", """
                                    - Verify that the attribute report values from each subscription are in ascending order
                                    - If the number of PercentSetting reports is equal to the number of PercentCurrent reports:
                                        - Verify that all the expected PercentCurrent values are present in the reports
                                    - If the number of PercentSetting reports is greater or equal than the number of SpeedCurrent reports:
                                        - Verify that all the expected SpeedCurrent values are present in the reports in accordance with the SpeedMax attribute value.
                                    [Save the resulting descending attribute report values from each subscription as a baseline for comparison with the descending values from the previous steps]"""),

                TestStep("11", """Compare the descending baseline values with the ascending baseline values""",
                         """Verify that the descending baseline values are the reverse of the ascending baseline values for each attribute"""),

                TestStep("12", """LowestOff field test
                                        Iteratively send step commands with LowestOff set to False, Direction set to Decrease, and Wrap set to False.
                                        Starting form the maximum fan attribute values and ending at the minimum PercentSetting attribute value above 0 (descending order).
                                        Monitoring the Setting attribute values primarily (PercentSetting, SpeedSetting).""", """
                                    SETUP
                                    - Initialize the PercentSetting attribute to 100
                                        - Verify that the SpeedSetting attribute value is set to SpeedMax
                                        - Verify that the FanMode attribute value is set to High
                                    - Subscribe to the PercentSetting, SpeedSetting, and FanMode attributes
                                    - Step: LowestOff=False, Direction=Decrease, Wrap=False"""),

                TestStep("12a", """TH sends Step commands iteratively""", """
                                    LowestOff Check
                                        - Verify that the PercentSetting attribute value reaches the minimum PercentSetting attribute value above 0
                                    No Wrap Check
                                        - When the PercentSetting attribute value reaches the minimum PercentSetting attribute value above 0:
                                            -Send an additional Step command to verify that PercentSetting stays at the minimum PercentSetting attribute value above 0"""),

                TestStep("12b", """Read the resulting attribute reports from each subscription""", """
                                    - Verify that the attribute report values from each subscription are in descending order
                                    - If the number of PercentSetting reports is greater than the number of FanMode reports:
                                        - Verify that all the expected FanMode values are present in the reports in accordance with the FanModeSequence attribute value
                                    - If the number of PercentSetting reports is greater or equal than the number of SpeedSetting reports:
                                        - Verify that all the expected SpeedSetting values are present in the reports in accordance with the SpeedMax attribute value.
                                    [Save the resulting descending attribute report values from each subscription as a baseline for comparison with the ascending values from the next steps]"""),

                TestStep("13", """LowestOff field test
                                        Iteratively send step commands with LowestOff set to False, Direction set to Increase, and Wrap set to False.
                                        Starting form the minimum fan attribute values and ending at the maximum fan attribute values (ascending order).
                                        Monitoring the Setting attribute values primarily (PercentSetting, SpeedSetting).""", """
                                    SETUP
                                    - Initialize the PercentSetting attribute to 0
                                        - Verify that the SpeedSetting attribute value is set to 0
                                        - Verify that the FanMode attribute value is set to Off
                                    - Subscribe to the PercentSetting, SpeedSetting, and FanMode attributes
                                    - Step: LowestOff=True, Direction=Increase, Wrap=False"""),

                TestStep("13a", """TH sends Step commands iteratively""", """
                                    Max PercentSetting Check
                                        - Verify that the PercentSetting attribute value reaches 100
                                    No Wrap Check
                                        - When the PercentSetting attribute value reaches 100:
                                            - Send an additional Step command to verify that PercentSetting stays at 100"""),

                TestStep("13b", """Read the resulting attribute reports from each subscription""", """
                                    - Verify that the attribute report values from each subscription are in ascending order
                                    - If the number of PercentSetting reports is greater than the number of FanMode reports:
                                        - Verify that all the expected FanMode values are present in the reports in accordance with the FanModeSequence attribute value
                                    - If the number of PercentSetting reports is greater or equal than the number of SpeedSetting reports:
                                        - Verify that all the expected SpeedSetting values are present in the reports in accordance with the SpeedMax attribute value.
                                    [Save the resulting ascending attribute report values from each subscription as a baseline for comparison with the descending values from the previous steps]"""),

                TestStep("14", """Compare the descending baseline values with the ascending baseline values""",
                         """Verify that the descending baseline values are the reverse of the ascending baseline values for each attribute"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),

                TestStep("ssssss", """aaaaa""", """ccccc"""),










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

    async def subscribe_to_attributes(self, handle_current_values: bool) -> None:
        cluster = Clusters.FanControl
        attr = cluster.Attributes

        if hasattr(self, 'subscriptions') and self.subscriptions is not None:
            for sub in self.subscriptions:
                sub.cancel()

        if not handle_current_values:
            self.subscriptions = [
                AttributeSubscriptionHandler(cluster, attr.PercentSetting),
                AttributeSubscriptionHandler(cluster, attr.FanMode),
                AttributeSubscriptionHandler(cluster, attr.SpeedSetting)
            ]
        else:
            self.subscriptions = [
                AttributeSubscriptionHandler(cluster, attr.PercentSetting),
                AttributeSubscriptionHandler(cluster, attr.PercentCurrent),
                AttributeSubscriptionHandler(cluster, attr.SpeedCurrent)
            ]

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

    async def get_percent_setting_range_per_step(self):
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        cmd = cluster.Commands
        sd_enum = cluster.Enums.StepDirectionEnum
        percent_setting_zero = 0

        # Set PercentSetting to 0
        await self.write_setting(attr.PercentSetting, percent_setting_zero)

        # Subscribe to PercentSetting attribute
        percent_setting_sub = AttributeSubscriptionHandler(cluster, attr.PercentSetting)
        await percent_setting_sub.start(self.default_controller, self.dut_node_id, self.endpoint)

        # Send Step command with direction=Increase
        await self.send_step_command(cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=True))

        # Get the resulting PercentSetting attribute report value from the queue
        self.percent_setting_per_step = percent_setting_sub.get_attribute_value_from_queue(endpoint=self.endpoint)

        await percent_setting_sub.cancel()
        logging.info(f"[FC] PercentSetting range per Step: {self.percent_setting_per_step}")

    def get_expected_percent_setting(self, step: Clusters.FanControl.Commands.Step) -> int:
        cluster = Clusters.FanControl
        sd_enum = cluster.Enums.StepDirectionEnum

        min_percent_setting = 0 if step.lowestOff else self.percent_setting_per_step
        percent_setting_expected = 100 if step.direction == sd_enum.kIncrease else min_percent_setting

        return percent_setting_expected

    async def lowest_off_field_conditions_test(self, step: Clusters.FanControl.Commands.Step) -> None:
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        percent_setting_sub = next((sub for sub in self.subscriptions if sub._expected_attribute == attr.PercentSetting), None)
        self.percent_setting_from_queue = []

        # Get the expected final PercentSetting value based on the Step command parameters
        percent_setting_expected = self.get_expected_percent_setting(step)
        if step.direction == cluster.Enums.StepDirectionEnum.kDecrease and not step.wrap and not step.lowestOff:
            logging.info(f"[FC] Step command: {step}, percent_setting_expected: {percent_setting_expected}")

        # The minimum PercentSetting increment per step is 1. The loop is written to handle that case, but it
        # won't necessarily run 100 iterations, only as many can fit within the 0–100 PercentSetting range.
        # Example: if each step increases PercentSetting by 10, the loop will run 10 iterations.
        for i in range(101):
            # Send the Step command
            await self.send_step_command(step)

            # Read the resulting PercentSetting attribute report value from the queue
            percent_setting = percent_setting_sub.get_attribute_value_from_queue(endpoint=self.endpoint)

            # Detect if the step increase resulted in a change in PercentSetting, if so,
            # add it to the `percent_setting_from_queue` list for subsequent comparisons
            if percent_setting is not None:
                percent_setting_last = percent_setting
                self.percent_setting_from_queue.append(percent_setting)
            else:
                percent_setting = percent_setting_last

            logging.info(f"[FC] PercentSetting attribute report value: {percent_setting}")

            # Once PercentSetting reaches the expected value, send an extra Step command to verify
            # that the PercentSetting attribute report value stays at the expected value (no wrap)
            if percent_setting == percent_setting_expected:
                # Send additional Step command
                await self.send_step_command(step)

                # Read the resulting PercentSetting attribute value
                # and verify it's still equal to the expected value
                percent_setting = await self.read_setting(attr.PercentSetting)
                asserts.assert_equal(
                    percent_setting, percent_setting_expected,
                    f"[FC] PercentSetting attribute value ({percent_setting}) is not equal to the expected value ({percent_setting_expected})"
                )
                break
            else:
                # If we reach here, it means the expected PercentSetting
                # attribute value was never reached
                if i == 100:
                    asserts.fail(
                        f"[FC] The expected PercentSetting attribute value ({percent_setting_expected}) was never reached, the last reported value is ({percent_setting})."
                    )

            logging.info(f"[FC] percent_setting_from_queue: {self.percent_setting_from_queue}")

    async def lowest_off_test(self, step: Clusters.FanControl.Commands.Step, handle_current_values: bool) -> None:
        """Tests the `lowestOff` flag for the given Step command.

        Args:
            step (Clusters.FanControl.Commands.Step): Step command parameters.

        Returns:
            None

        Raises:
            AssertionError: If expected conditions are not met during test steps.
        """
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        sd_enum = cluster.Enums.StepDirectionEnum

        # *** NEXT STEP ***
        # Initialize the PercentSetting attribute and verify the expected
        # attribute changes in accordance with the Step command parameters
        self.step(self.current_step_index + 1)
        percent_setting_init = 0 if step.direction == sd_enum.kIncrease else self.percent_setting_max
        if step.wrap:
            if step.direction == sd_enum.kDecrease:
                percent_setting_init = self.percent_setting_per_step
            else:
                percent_setting_init = self.percent_setting_max
        await self.write_setting(attr.PercentSetting, percent_setting_init)

        # *** NEXT STEP ***
        # Subscribe to the requested attributes
        self.step(self.current_step_index + 1)
        await self.subscribe_to_attributes(handle_current_values=handle_current_values)

        # *** NEXT STEP ***
        # TH sends Step commands iteratively
        #  - During the loop:
        #    - Verify the expected PercentSetting attribute report value is reached.
        #    - If the expected PercentSetting attribute report value is reached, send
        #      an additional Step command to veiryf that the PercentSetting attribute
        #      report value stays at the expected value.
        self.step(self.current_step_index + 1)
        await self.lowest_off_field_conditions_test(step)

        # *** NEXT STEP ***
        #  Read the resulting attribute reports from each subscription after the Step commands
        #   - Verify that the attribute report values from each subscription are in the
        #     expected order in accordance with the Step command direction parameter.
        #   - Verify that all the expected attribute values are present in the reports.
        #   - Save the resulting attribute report values from each subscription as a baseline
        #     for future comparisons.
        self.step(self.current_step_index + 1)
        self.verify_all_expected_reports_and_value_progression(step, percent_setting_init, handle_current_values)

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

    def verify_all_expected_reports_and_value_progression(self, step: Clusters.FanControl.Commands.Step, percent_setting_init: int, handle_current_values: bool) -> None:
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
        # - Get the values of the speed oriented attributes
        for sub in self.subscriptions:
            if sub._expected_attribute == attr.PercentSetting:
                percent_setting_report_qty = len(self.percent_setting_from_queue)
                percent_setting_values_produced = self.percent_setting_from_queue
                self.verify_attribute_progression(step, attr.PercentSetting, percent_setting_values_produced)
                sub.log_queue()
            if not handle_current_values:
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
            else:
                if sub._expected_attribute == attr.PercentCurrent:
                    percent_current_report_qty = len(sub.attribute_queue.queue)
                    percent_current_values_produced = [q.value for q in sub.attribute_queue.queue]
                    self.verify_attribute_progression(step, attr.PercentCurrent, percent_current_values_produced)
                    sub.log_queue()
                if sub._expected_attribute == attr.SpeedCurrent:
                    speed_current_report_qty = len(sub.attribute_queue.queue)
                    speed_current_values_produced = [q.value for q in sub.attribute_queue.queue]
                    self.verify_attribute_progression(step, attr.SpeedCurrent, speed_current_values_produced)
                    sub.log_queue()

        if not handle_current_values:
            logging.info(f"[FC] fan_mode_values_produced: {fan_mode_values_produced}")
            logging.info(f"[FC] speed_setting_values_produced: {speed_setting_values_produced}")
        else:
            logging.info(f"[FC] percent_current_values_produced: {percent_current_values_produced}")
            logging.info(f"[FC] speed_current_values_produced: {speed_current_values_produced}")

        if not handle_current_values:
            dependent_values1 = fan_mode_values_produced
            dependent_values2 = speed_setting_values_produced
        else:
            dependent_values1 = percent_current_values_produced
            dependent_values2 = speed_current_values_produced

        # Get SpeedMax range
        speed_max_range = range(0, self.speed_max + 1)

        # Determine the initialization attribute values to remove
        if not handle_current_values:
            fan_mode_remove = fm_enum.kOff if percent_setting_init == 0 else fm_enum.kHigh
            speed_setting_remove = 0 if percent_setting_init == 0 else self.speed_max
        else:
            percent_current_remove = 0 if percent_setting_init == 0 else 100
            speed_current_remove = 0 if percent_setting_init == 0 else self.speed_max

        # Remove initialization attribute values from the full attribute value ranges
        if not handle_current_values:
            fan_modes_init_removed = [x for x in self.fan_modes if x != fan_mode_remove]
            speed_setting_init_removed = [x for x in speed_max_range if x != speed_setting_remove]
        else:
            percent_current_init_removed = [x for x in list(
                reversed(percent_current_values_produced)) if x != percent_current_remove]
            speed_current_init_removed = [x for x in speed_max_range if x != speed_current_remove]

        # When the Step command has direction=decrease and lowestOff=False, the zero or Off state will never be reached,
        # therefore we also prepare removal of that element from the full attribute value ranges for that case
        trim = slice(None, -1) if (step.direction == sd_enum.kDecrease and not step.lowestOff) else slice(None)

        # Determine the final expected attribute values lists for comparison with the produced values
        if not handle_current_values:
            fan_modes_expected = fan_modes_init_removed[trim] if step.direction == sd_enum.kIncrease else list(
                reversed(fan_modes_init_removed))[trim]
            speed_setting_expected = speed_setting_init_removed[trim] if step.direction == sd_enum.kIncrease else list(
                reversed(speed_setting_init_removed))[trim]
        else:
            percent_current_expected = list(reversed(percent_current_init_removed))[
                trim] if step.direction == sd_enum.kIncrease else list(reversed(percent_current_init_removed))
            speed_current_expected = speed_current_init_removed[trim] if step.direction == sd_enum.kIncrease else list(
                reversed(speed_current_init_removed))[trim]

        if not handle_current_values:
            logging.info(f"[FC] fan_modes_expected: {fan_modes_expected}")
            logging.info(f"[FC] speed_setting_expected: {speed_setting_expected}")
        else:
            logging.info(f"[FC] percent_current_expected: {percent_current_expected}")
            logging.info(f"[FC] speed_current_expected: {speed_current_expected}")

        if not handle_current_values:
            # If the number of PercentSetting reports is greater than the number of FanMode reports,
            # - Verify that all the expected FanMode values are present in the reports
            if percent_setting_report_qty > fan_mode_report_qty:
                missing_fan_modes = [mode for mode in fan_modes_expected if mode not in fan_mode_values_produced]
                asserts.assert_equal(
                    fan_modes_expected, fan_mode_values_produced,
                    f"[FC] Some of the expected FanMode values are not present in the reports. Expected: {fan_modes_expected}, missing: {missing_fan_modes}."
                )

            # If the number of PercentSetting reports is greater or equal than the number of SpeedSetting reports,
            # - Verify that all the expected SpeedSetting values are present in the reports
            if percent_setting_report_qty >= speed_setting_report_qty:
                missing_speed_setting = [speed for speed in speed_setting_expected if speed not in speed_setting_values_produced]
                asserts.assert_equal(
                    speed_setting_expected, speed_setting_values_produced,
                    f"[FC] Some of the expected SpeedSetting values are not present in the reports. Expected: {speed_setting_expected}, missing: {missing_speed_setting}."
                )
        else:
            # If the number of PercentCurrent reports is greater or equal than the number of SpeedCurrent reports,
            # - Verify that all the expected PercentCurrent values are present in the reports
            if percent_setting_report_qty == percent_current_report_qty:
                asserts.assert_equal(
                    percent_current_expected, percent_current_values_produced,
                    f"[FC] PercentCurrent attribute values not equal to PercentSetting attribute values. PercentCurrent: {percent_current_values_produced}, PercentSetting: {percent_setting_values_produced}."
                )
            else:
                missing_percent_current = [
                    percent for percent in percent_current_expected if percent not in percent_current_values_produced]
                asserts.fail(
                    f"[FC] Some of the expected PercentCurrent values are not present in the reports. Expected: {percent_current_expected}, missing: {missing_percent_current}.")

            # If the number of PercentCurrent reports is greater or equal than the number of SpeedCurrent reports,
            # - Verify that all the expected SpeedCurrent values are present in the reports
            if percent_setting_report_qty >= speed_current_report_qty:
                missing_speed_current = [speed for speed in speed_current_expected if speed not in speed_current_values_produced]
                asserts.assert_equal(
                    speed_current_expected, speed_current_values_produced,
                    f"[FC] Some of the expected SpeedCurrent values are not present in the reports. Expected: {speed_current_expected}, missing: {missing_speed_current}."
                )

        # Save the resulting attribute report values from each subscription as a baseline for future comparisons.
        self.save_baseline_values(step, percent_setting_values_produced, dependent_values1,
                                  dependent_values2, handle_current_values)

    def save_baseline_values(self, step: Clusters.FanControl.Commands.Step, percent_setting_values_produced: list, dependent_values_produced1: list, dependent_values_produced2: list, handle_current_values: bool) -> None:
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

        # if step.direction == sd_enum.kDecrease and step.lowestOff:
        if step.direction == sd_enum.kDecrease:
            trim = slice(None) if not step.lowestOff else slice(None, -1)
            self.baseline_percent_setting_desc = percent_setting_values_produced[trim]
            if not handle_current_values:
                self.baseline_fan_mode_desc = dependent_values_produced1[trim]
                self.baseline_speed_setting_desc = dependent_values_produced2[trim]
            else:
                self.baseline_percent_current_desc = dependent_values_produced1[trim]
                self.baseline_speed_current_desc = dependent_values_produced2[trim]

        # if step.direction == sd_enum.kIncrease and step.lowestOff:
        if step.direction == sd_enum.kIncrease:
            self.baseline_percent_setting_asc = percent_setting_values_produced[:-1]
            if not handle_current_values:
                self.baseline_fan_mode_asc = dependent_values_produced1[:-1]
                self.baseline_speed_setting_asc = dependent_values_produced2[:-1]
            else:
                self.baseline_percent_current_asc = dependent_values_produced1[:-1]
                self.baseline_speed_current_asc = dependent_values_produced2[:-1]

    def verify_baseline_values(self, handle_current_values: bool) -> None:
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
        if not handle_current_values:
            asserts.assert_equal(
                self.baseline_fan_mode_desc, list(reversed(self.baseline_fan_mode_asc)),
                f"[FC] FanMode attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_fan_mode_desc}, Ascending: {self.baseline_fan_mode_asc}."
            )
            asserts.assert_equal(
                self.baseline_speed_setting_desc, list(reversed(self.baseline_speed_setting_asc)),
                f"[FC] SpeedSetting attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_speed_setting_desc}, Ascending: {self.baseline_speed_setting_asc}."
            )
        else:
            asserts.assert_equal(
                self.baseline_percent_current_desc, list(reversed(self.baseline_percent_current_asc)),
                f"[FC] PercentCurrent attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_percent_current_desc}, Ascending: {self.baseline_percent_current_asc}."
            )
            asserts.assert_equal(
                self.baseline_speed_current_desc, list(reversed(self.baseline_speed_current_asc)),
                f"[FC] SpeedCurrent attribute baseline values do not match after the Step command decrease/increase runs. Descending: {self.baseline_speed_current_desc}, Ascending: {self.baseline_speed_current_asc}."
            )

    async def wrap_test(self, step: Clusters.FanControl.Commands.Step, handle_current_values: bool) -> None:
        """Tests the `wrap` flag for the given Step command.

        Args:
            step (Clusters.FanControl.Commands.Step): Step command parameters.
        """
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        sd_enum = cluster.Enums.StepDirectionEnum

        # *** NEXT STEP ***
        # Initialize the PercentSetting attribute and verify the expected
        # attribute changes in accordance with the Step command parameters
        self.step(self.current_step_index + 1)
        percent_setting_init = 0 if step.direction == sd_enum.kIncrease else self.percent_setting_max
        if step.wrap:
            if step.direction == sd_enum.kDecrease:
                percent_setting_init = self.percent_setting_per_step
            else:
                percent_setting_init = self.percent_setting_max
        await self.write_setting(attr.PercentSetting, percent_setting_init)

        # *** NEXT STEP ***
        # Subscribe to the requested attributes
        self.step(self.current_step_index + 1)
        await self.subscribe_to_attributes(handle_current_values=handle_current_values)

        # *** NEXT STEP ***
        # TH sends a Step command with the reqested parameters
        #  - Verify the expected attribute values are produced
        self.step(self.current_step_index + 1)
        await self.perform_wrap_step_commands_and_verify(step, handle_current_values)

    async def perform_wrap_step_commands_and_verify(self, step: Clusters.FanControl.Commands.Step, handle_current_values: bool) -> None:
        cluster = Clusters.FanControl
        fm_enum = cluster.Enums.FanModeEnum
        sd_enum = cluster.Enums.StepDirectionEnum

        if step.direction == sd_enum.kDecrease and step.lowestOff:
            if not handle_current_values:
                # - Verify that the attribute values all go to Off values
                await self.wrap_veirfy(step, percent_setting_expected=0, fan_mode_expected=fm_enum.kOff, speed_setting_expected=0)
                # - Verify that the attribute values all go to High values
                await self.wrap_veirfy(step, percent_setting_expected=100, fan_mode_expected=fm_enum.kHigh, speed_setting_expected=self.speed_max)
            else:
                # - Verify that the attribute values all go to Off values
                await self.wrap_veirfy(step, percent_setting_expected=0, percent_current_expected=0, speed_current_expected=0)
                # - Verify that the attribute values all go to High values
                await self.wrap_veirfy(step, percent_setting_expected=100, percent_current_expected=100, speed_current_expected=self.speed_max)
        elif step.direction == sd_enum.kDecrease and not step.lowestOff:
            # - Verify that the attribute values all go to High values
            if not handle_current_values:
                await self.wrap_veirfy(step, percent_setting_expected=100, fan_mode_expected=fm_enum.kHigh, speed_setting_expected=self.speed_max)
            else:
                await self.wrap_veirfy(step, percent_setting_expected=100, percent_current_expected=100, speed_current_expected=self.speed_max)
        elif step.direction == sd_enum.kIncrease and step.lowestOff:
            # - Verify that the attribute values all go to Off values
            if not handle_current_values:
                await self.wrap_veirfy(step, percent_setting_expected=0, fan_mode_expected=fm_enum.kOff, speed_setting_expected=0)
            else:
                await self.wrap_veirfy(step, percent_setting_expected=0, percent_current_expected=0, speed_current_expected=0)
        elif step.direction == sd_enum.kIncrease and not step.lowestOff:
            # - Verify that the PercentSetting attribute value goes to the minimum Step value above 0
            if not handle_current_values:
                await self.wrap_veirfy(step, percent_setting_expected=self.percent_setting_per_step)
            else:
                await self.wrap_veirfy(step, percent_setting_expected=self.percent_setting_per_step, percent_current_expected=self.percent_setting_per_step)

    async def wrap_veirfy(
        self,
        step: Clusters.FanControl.Commands.Step,
        percent_setting_expected: int,
        fan_mode_expected: Optional[Any] = None,
        speed_setting_expected: Optional[int] = None,
        percent_current_expected: Optional[Any] = None,
        speed_current_expected: Optional[int] = None
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
        if percent_current_expected is not None:
            percent_current_sub = next((sub for sub in self.subscriptions if sub._expected_attribute == attr.PercentCurrent), None)
            percent_current = percent_current_sub.get_attribute_value_from_queue(endpoint=self.endpoint)

            asserts.assert_equal(
                percent_current, percent_current_expected,
                f"[FC] PercentCurrent attribute value ({percent_current}) is not equal to the expected value ({percent_current_expected})."
            )
        if speed_current_expected is not None:
            speed_current_sub = next((sub for sub in self.subscriptions if sub._expected_attribute == attr.SpeedCurrent), None)
            speed_current = speed_current_sub.get_attribute_value_from_queue(endpoint=self.endpoint)

            asserts.assert_equal(
                speed_current, speed_current_expected,
                f"[FC] SpeedCurrent attribute value ({speed_current}) is not equal to the expected value ({speed_current_expected})."
            )

    def pics_TC_FAN_3_5(self) -> list[str]:
        return ["FAN.S"]

    @run_if_endpoint_matches(
        has_feature(Clusters.FanControl, Clusters.FanControl.Bitmaps.Feature.kStep)
        and has_feature(Clusters.FanControl, Clusters.FanControl.Bitmaps.Feature.kMultiSpeed)
    )
    async def test_TC_FAN_3_5(self) -> None:
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        cmd = cluster.Commands
        sd_enum = cluster.Enums.StepDirectionEnum
        self.timeout_sec: float = 1
        self.percent_setting_per_step: Optional[int] = None
        self.percent_setting_max = 100

        # *** STEP 1 ***
        # Commissioning already done
        self.step("1")

        # *** STEP 2 ***
        # TH checks the DUT for the presence of the OnOff cluster
        # - If the cluster is present, set it to On
        self.step("2")
        has_on_off_cluster = await self.cluster_guard(endpoint=self.endpoint, cluster=Clusters.OnOff, skip_step=False)
        if has_on_off_cluster:
            await self.send_on_off_command(Clusters.OnOff.Commands.On())

        # *** STEP 3 ***
        # TH reads from the DUT the SpeedMax attribute
        #  - Store value for future reference
        self.step("3")
        self.speed_max = await self.read_setting(attr.SpeedMax)

        # *** STEP 4 ***
        # TH reads from the DUT the FanModeSequence attribute
        #  - Store value for future reference
        self.step("4")
        await self.get_fan_modes(remove_auto=True)

        # *** STEP 5 ***
        # TH determines the PercentSetting range per Step command
        #  - Set PercentSetting to 0
        #  - Send Step command with direction=Increase, lowestOff=True, and wrap=False
        #  - Read the resulting PercentSetting attribute report value
        #  - Calculate the PercentSetting range per Step command
        #  - Store value for future reference
        self.step("5")
        await self.get_percent_setting_range_per_step()

        # LowestOff Tests

        # *** NEXT STEP ***
        # TH tests Step command behavior of the LowestOff field
        #  - Setup a Step command with: -- Direction=Decrease -- Wrap=False -- LowestOff=True
        #  - Verify Attributes PercentSetting, FanMode, and SpeedSetting
        self.step(self.current_step_index + 1)
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=True), handle_current_values=False)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the LowestOff field
        #  - Setup a Step command with: -- Direction=Increase -- Wrap=False -- LowestOff=True
        #  - Verify Attributes PercentSetting, FanMode, and SpeedSetting
        self.step(self.current_step_index + 1)
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=True), handle_current_values=False)

        # ** NEXT STEP ***
        # Compare the descending baseline values with the ascending baseline values
        #  - Verify that the descending baseline values are the reverse of the ascending baseline values for each attribute
        self.step(self.current_step_index + 1)
        self.verify_baseline_values(handle_current_values=False)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the LowestOff field
        #  - Setup a Step command with: -- Direction=Decrease -- Wrap=False -- LowestOff=True
        #  - Verify Attributes PercentSetting, PercentCurrent, and SpeedCurrent
        self.step(self.current_step_index + 1)
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=True), handle_current_values=True)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the LowestOff field
        #  - Setup a Step command with: -- Direction=Increase -- Wrap=False -- LowestOff=True
        #  - Verify Attributes PercentSetting, PercentCurrent, and SpeedCurrent
        self.step(self.current_step_index + 1)
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=True), handle_current_values=True)

        # ** NEXT STEP ***
        # Compare the descending baseline values with the ascending baseline values
        #  - Verify that the descending baseline values are the reverse of the ascending baseline values for each attribute
        self.step(self.current_step_index + 1)
        self.verify_baseline_values(handle_current_values=False)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the LowestOff field
        #  - Setup a Step command with: -- Direction=Decrease -- Wrap=False -- LowestOff=False
        #  - Verify Attributes PercentSetting, FanMode, and SpeedSetting
        self.step(self.current_step_index + 1)
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=False), handle_current_values=False)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the LowestOff field
        #  - Setup a Step command with: -- Direction=Increase -- Wrap=False -- LowestOff=False
        #  - Verify Attributes PercentSetting, FanMode, and SpeedSetting
        self.step(self.current_step_index + 1)
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=False), handle_current_values=False)

        # ** NEXT STEP ***
        # Compare the descending baseline values with the ascending baseline values
        #  - Verify that the descending baseline values are the reverse of the ascending baseline values for each attribute
        self.step(self.current_step_index + 1)
        self.verify_baseline_values(handle_current_values=False)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the LowestOff field
        #  - Setup a Step command with: -- Direction=Decrease -- Wrap=False -- LowestOff=False
        #  - Verify Attributes PercentSetting, PercentCurrent, and SpeedCurrent
        self.step(self.current_step_index + 1)
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=False), handle_current_values=True)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the LowestOff field
        #  - Setup a Step command with: -- Direction=Increase -- Wrap=False -- LowestOff=False
        #  - Verify Attributes PercentSetting, PercentCurrent, and SpeedCurrent
        self.step(self.current_step_index + 1)
        await self.lowest_off_test(cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=False), handle_current_values=True)

        # ** NEXT STEP ***
        # Compare the descending baseline values with the ascending baseline values
        #  - Verify that the descending baseline values are the reverse of the ascending baseline values for each attribute
        self.step(self.current_step_index + 1)
        self.verify_baseline_values(handle_current_values=True)

        # Wrap Tests

        # *** NEXT STEP ***
        # TH tests Step command behavior of the Wrap field
        #  - Setup a Step command with: -- Direction=Decrease -- Wrap=True -- LowestOff=True
        #  - Verify Attributes PercentSetting, FanMode, and SpeedSetting
        self.step(self.current_step_index + 1)
        await self.wrap_test(cmd.Step(direction=sd_enum.kDecrease, wrap=True, lowestOff=True), handle_current_values=False)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the Wrap field
        #  - Setup a Step command with: -- Direction=Decrease -- Wrap=True -- LowestOff=False
        #  - Verify Attributes PercentSetting, FanMode, and SpeedSetting
        self.step(self.current_step_index + 1)
        await self.wrap_test(cmd.Step(direction=sd_enum.kDecrease, wrap=True, lowestOff=False), handle_current_values=False)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the Wrap field
        #  - Setup a Step command with: -- Direction=Increase -- Wrap=True -- LowestOff=True
        #  - Verify Attributes PercentSetting, FanMode, and SpeedSetting
        self.step(self.current_step_index + 1)
        await self.wrap_test(cmd.Step(direction=sd_enum.kIncrease, wrap=True, lowestOff=True), handle_current_values=False)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the Wrap field
        #  - Setup a Step command with: -- Direction=Increase -- Wrap=True -- LowestOff=False
        #  - Verify Attributes PercentSetting, FanMode, and SpeedSetting
        self.step(self.current_step_index + 1)
        await self.wrap_test(cmd.Step(direction=sd_enum.kIncrease, wrap=True, lowestOff=False), handle_current_values=False)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the Wrap field
        #  - Setup a Step command with: -- Direction=Decrease -- Wrap=True -- LowestOff=True
        #  - Verify Attributes PercentSetting, PercentCurrent, and SpeedCurrent
        self.step(self.current_step_index + 1)
        await self.wrap_test(cmd.Step(direction=sd_enum.kDecrease, wrap=True, lowestOff=True), handle_current_values=True)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the Wrap field
        #  - Setup a Step command with: -- Direction=Decrease -- Wrap=True -- LowestOff=False
        #  - Verify Attributes PercentSetting, PercentCurrent, and SpeedCurrent
        self.step(self.current_step_index + 1)
        await self.wrap_test(cmd.Step(direction=sd_enum.kDecrease, wrap=True, lowestOff=False), handle_current_values=True)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the Wrap field
        #  - Setup a Step command with: -- Direction=Increase -- Wrap=True -- LowestOff=True
        #  - Verify Attributes PercentSetting, PercentCurrent, and SpeedCurrent
        self.step(self.current_step_index + 1)
        await self.wrap_test(cmd.Step(direction=sd_enum.kIncrease, wrap=True, lowestOff=True), handle_current_values=True)

        # *** NEXT STEP ***
        # TH tests Step command behavior of the LowestOff field
        #  - Setup a Step command with: -- Direction=Increase -- Wrap=True -- LowestOff=False
        #  - Verify Attributes PercentSetting, PercentCurrent, and SpeedCurrent
        self.step(self.current_step_index + 1)
        await self.wrap_test(cmd.Step(direction=sd_enum.kIncrease, wrap=True, lowestOff=False), handle_current_values=True)


if __name__ == "__main__":
    default_matter_test_main()
