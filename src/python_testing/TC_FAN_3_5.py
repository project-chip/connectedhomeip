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
import math
from dataclasses import dataclass, field
from typing import Any, List

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_infrastructure.chip.testing.matter_testing import (AttributeValueExpected, ClusterAttributeChangeAccumulator,
                                                                       ComparisonEnum, MatterBaseTest, async_test_body,
                                                                       default_matter_test_main)
from mobly import asserts

logger = logging.getLogger(__name__)


@dataclass
class Step:
    """
    Represents a step command to be sent to the FanControl cluster, along with the 
    expected attribute value updates after the command execution.

    Attributes:
        direction (Clusters.FanControl.Enums.StepDirectionEnum): The direction of 
            the step command, either to increase or decrease the fan speed. Defaults 
            to `StepDirectionEnum.kIncrease`.
        wrap (bool): Indicates whether the fan speed should wrap around when the 
            maximum or minimum speed is reached. Defaults to `False`.
        lowest_off (bool): Indicates that the fan being off is included as a step value.
            Defaults to `True`.
        expected_attributes (List[AttributeValueExpected]): A list of expected 
            attribute value updates after the step command is executed. Defaults to 
            an empty list.
    """
    direction: Clusters.FanControl.Enums.StepDirectionEnum = Clusters.FanControl.Enums.StepDirectionEnum.kIncrease
    wrap: bool = False
    lowest_off: bool = True
    expected_attributes: List[AttributeValueExpected] = field(default_factory=list)


@dataclass
class AttributeUpdate:
    """
    Represents an attribute update operation, encapsulating the attribute to be updated, 
    the value to write, and the expected attribute value updates after the write operation.

    Attributes:
        attribute (Clusters.ClusterObjects.ClusterAttributeDescriptor): 
            The attribute to be updated.
        value (Any): 
            The value to write to the specified attribute.
        expected_attributes (List[AttributeValueExpected]): 
            A list of expected attribute value updates after the write operation.
            Defaults to an empty list.
    """
    attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor
    value: Any
    expected_attributes: List[AttributeValueExpected] = field(default_factory=list)


class TC_FAN_3_5(MatterBaseTest):
    def desc_TC_FAN_3_5(self) -> str:
        return "[TC-FAN-3.5] Optional step functionality with DUT as Server"

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

    async def send_step_command(self, direction: Clusters.Objects.FanControl.Enums.StepDirectionEnum,
                                wrap: bool = False, lowestOff: bool = False) -> None:
        """
        Sends a step command to the Fan Control cluster.

        This asynchronous method sends a single command to adjust the fan's speed or state
        based on the specified direction, wrap, and lowestOff parameters.

        Args:
            direction (Clusters.Objects.FanControl.Enums.StepDirectionEnum): 
                The direction in which to step the fan's speed or state.
            wrap (bool, optional): 
                Indicates whether the fan speed should wrap around when reaching the maximum or minimum limit. 
                Defaults to False.
            lowestOff (bool, optional): 
                Specifies whether the lowest fan speed should turn the fan off. 
                Defaults to False.

        Raises:
            InteractionModelError: 
                If an unexpected error occurs, it asserts that the error status is equal to `Status.Success`.

        Returns:
            None
        """
        try:
            logger.info(f"[FC] Sending Step command - direction: {direction.name}, wrap: {wrap}, lowestOff: {lowestOff}")
            await self.send_single_cmd(cmd=Clusters.Objects.FanControl.Commands.Step(direction=direction, wrap=wrap, lowestOff=lowestOff), endpoint=self.endpoint)
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

    async def verify_step_attribute_values(self, attribute_update: AttributeUpdate, step: Step, expect_write_updates: bool = True, expect_command_updates: bool = True) -> None:
        """
        Verifies that attribute values are updated correctly, if expected, after performing write and command operations.

        This method performs the following steps:
        1. Writes a value to a specified attribute and verifies that the expected attribute values are updated accordingly.
        2. Sends a step command and verifies that the expected attribute values are updated after the command execution.

        Args:
            attribute_update (AttributeUpdate): An object containing the attribute to be updated, the value to write, 
                                                 and the expected attribute values after the update.
            step (Step): An object containing the parameters for the step command, including direction, wrap, 
                         lowestOff, and the expected attribute values after the command execution.
            expect_write_updates (bool): - If False, the value of the last report of the specified attribute will be
                                           used following the write operation (not incoming reports)
                                         - If True, the value of the incoming report of the specified attribute will be
                                           used following the write operation (not pre-existing reports)
                                         Defaults to True.
            expect_command_updates (bool): - If False, the value of the last report of the specified attribute will be
                                              used following the command operation (not incoming reports)
                                           - If True, the value of the incoming report of the specified attribute will be
                                              used following the command operation (not pre-existing reports)
                                           Defaults to True.
        Returns:
            None

        Raises:
            Assertion failure if the expected attribute values are not reported within the specified timeout.
        """
        if expect_write_updates:
            self.attribute_subscription.reset()
        await self.write_setting(attribute_update.attribute, attribute_update.value)
        if expect_write_updates:
            self.attribute_subscription.await_all_final_values_reported_threshold(
                value_expectations=attribute_update.expected_attributes, timeout_sec=self.timeout_sec)
        else:
            for expected_attribute in attribute_update.expected_attributes:
                self.verify_expected_value(expected_attribute.attribute, expected_attribute.threshold_value)

        if expect_command_updates:
            self.attribute_subscription.reset()
        await self.send_step_command(direction=step.direction, wrap=step.wrap, lowestOff=step.lowest_off)
        if expect_command_updates:
            self.attribute_subscription.await_all_final_values_reported_threshold(
                value_expectations=step.expected_attributes, timeout_sec=self.timeout_sec)
        else:
            for expected_attribute in step.expected_attributes:
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

    def pics_TC_FAN_3_5(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_5(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        sd_enum = cluster.Enums.StepDirectionEnum
        fm_enum = cluster.Enums.FanModeEnum
        self.timeout_sec: float = 0.5

        # *** STEP 1 ***
        # Commissioning already done
        # self.step(1)
        print(f"[FC] # self.step(1)")

        # *** STEP 2 ***
        # TH checks the DUT for support of the Step and MultiSpeed features
        #  - If the DUT does not support both features, the test is skipped
        # self.step(2)
        print(f"[FC] # self.step(2)")
        feature_map = await self.read_setting(attr.FeatureMap)
        self.supports_step = bool(feature_map & cluster.Bitmaps.Feature.kStep)
        self.multi_speed = bool(feature_map & cluster.Bitmaps.Feature.kMultiSpeed)
        if not self.supports_step or not self.supports_step:
            logger.info("Both the Step and MultiSpeed features must be supported by the DUT for this test, skipping test.")
            return

        # *** STEP 3 ***
        # TH reads from the DUT the SpeedMax attribute
        #  - Store value for future reference
        # self.step(3)
        print(f"[FC] # self.step(3)")
        self.speed_max = await self.read_setting(attr.SpeedMax)

        # *** STEP 4 ***
        # TH subscribes to the FanControl cluster
        #  - Enables the TH to receive attribute updates
        # self.step(4)
        print(f"[FC] # self.step(4)")
        self.attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        await self.attribute_subscription.start(self.default_controller, self.dut_node_id, self.endpoint)

        # *** STEP 5 ***
        # TH writes to the DUT the PercentSetting attribute with 50, then sends a Step
        # command with Direction set to Increase, Wrap set to False, and LowestOff set to True
        #  - Verify that the PercentStting and PercentCurrent attributes
        #    are both set to 50 after the write operation
        #  - Verify that the PercentStting and PercentCurrent attributes
        #    are both greater than 50 after the Step command is executed
        # self.step(5)
        print(f"[FC] # self.step(5)")
        percent_setting = 50
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting),
        ]
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.GreaterThan, percent_setting),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.GreaterThan, percent_setting),
        ]
        attribute_update = AttributeUpdate(attr.PercentSetting, percent_setting, write_expect)
        step = Step(direction=sd_enum.kIncrease, expected_attributes=command_expect, wrap=False, lowest_off=True)
        await self.verify_step_attribute_values(attribute_update, step)

        # *** STEP 6 ***
        # TH writes to the DUT the PercentSetting attribute with 50, then sends a Step
        # command with Direction set to Decrease, Wrap set to False, and LowestOff set to True
        #  - Verify that the PercentStting and PercentCurrent attributes
        #    are both set to 50 after the write operation
        #  - Verify that the PercentStting and PercentCurrent attributes
        #    are both less than 50 after the Step command is executed
        # self.step(6)
        print(f"[FC] # self.step(6)")
        percent_setting = 50
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting),
        ]
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.LessThan, percent_setting),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.LessThan, percent_setting),
        ]
        attribute_update = AttributeUpdate(attr.PercentSetting, percent_setting, write_expect)
        step = Step(direction=sd_enum.kDecrease, expected_attributes=command_expect, wrap=False, lowest_off=True)
        await self.verify_step_attribute_values(attribute_update, step)

        # *** STEP 7 ***
        # TH writes to the DUT the SpeedSetting attribute with 0, then sends a Step
        # command with Direction set to Increase, Wrap set to False, and LowestOff set to True
        #  - After the write operation:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        #  - After the step command is executed:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1
        #    - Verify that the PercentStting and PercentCurrent attributes are both
        #      set to the corresponding values as per the percent formula:
        #      percent=floor(speed/SpeedMax*100)
        #    - Verify that the FanMode attribute is set to Low (1)
        # self.step(7)
        print(f"[FC] # self.step(7)")
        speed_setting = 0
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        speed_setting_expected = 1
        percent_setting_expected = self.compute_percent_setting(speed_setting_expected)
        fan_mode_expected = fm_enum.kLow
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.FanMode, ComparisonEnum.Equal, fan_mode_expected),
        ]
        attribute_update = AttributeUpdate(attr.SpeedSetting, speed_setting, write_expect)
        step = Step(direction=sd_enum.kIncrease, expected_attributes=command_expect, wrap=False, lowest_off=True)
        await self.verify_step_attribute_values(attribute_update, step)

        # *** STEP 8 ***
        # TH writes to the DUT the SpeedSetting attribute with 0, then sends a Step
        # command with Direction set to Decrease, Wrap set to False, and LowestOff set to True
        #  - After the write operation:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        #  - After the step command is executed:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        #    - Verify that the PercentStting and PercentCurrent attributes are both set to 0
        #    - Verify that the FanMode attribute is set to Off (0)
        # self.step(8)
        print(f"[FC] # self.step(8)")
        speed_setting = 0
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        speed_setting_expected = 0
        percent_setting_expected = 0
        fan_mode_expected = fm_enum.kOff
        command_expect = [
            # AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting_expected),
            # AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.FanMode, ComparisonEnum.Equal, fan_mode_expected),
        ]
        attribute_update = AttributeUpdate(attr.SpeedSetting, speed_setting, write_expect)
        step = Step(direction=sd_enum.kDecrease, expected_attributes=command_expect, wrap=False, lowest_off=True)
        await self.verify_step_attribute_values(attribute_update, step, expect_command_updates=False)

        # *** STEP 9 ***
        # TH writes to the DUT the SpeedSetting attribute with 0, then sends a Step
        # command with Direction set to Decrease, Wrap set to True, and LowestOff set to True
        #  - After the write operation:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        #  - After the step command is executed:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        #    - Verify that the PercentStting and PercentCurrent attributes are both
        #      set to the corresponding values as per the percent formula:
        #      percent=floor(speed/SpeedMax*100)
        #    - Verify that the FanMode attribute is set to High (3)
        # self.step(9)
        print(f"[FC] # self.step(9)")
        speed_setting = 0
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        speed_setting_expected = self.speed_max
        percent_setting_expected = self.compute_percent_setting(speed_setting_expected)
        fan_mode_expected = fm_enum.kHigh
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.FanMode, ComparisonEnum.Equal, fan_mode_expected),
        ]
        attribute_update = AttributeUpdate(attr.SpeedSetting, speed_setting, write_expect)
        step = Step(direction=sd_enum.kDecrease, expected_attributes=command_expect, wrap=True, lowest_off=True)
        await self.verify_step_attribute_values(attribute_update, step, expect_write_updates=False)

        # *** STEP 10 ***
        # TH writes to the DUT the SpeedSetting attribute with 1, then sends a Step
        # command with Direction set to Decrease, Wrap set to False, and LowestOff set to False
        #  - After the write operation:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1
        #  - After the step command is executed:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1
        #    - Verify that the PercentStting and PercentCurrent attributes are both
        #      set to the corresponding values as per the percent formula:
        #      percent=floor(speed/SpeedMax*100)
        #    - Verify that the FanMode attribute is set to Low (1)
        # self.step(10)
        print(f"[FC] # self.step(10)")
        speed_setting = 1
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        speed_setting_expected = 1
        percent_setting_expected = self.compute_percent_setting(speed_setting_expected)
        fan_mode_expected = fm_enum.kLow
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.FanMode, ComparisonEnum.Equal, fan_mode_expected),
        ]
        attribute_update = AttributeUpdate(attr.SpeedSetting, speed_setting, write_expect)
        step = Step(direction=sd_enum.kDecrease, expected_attributes=command_expect, wrap=False, lowest_off=False)
        await self.verify_step_attribute_values(attribute_update, step, expect_command_updates=False)

        # *** STEP 11 ***
        # TH writes to the DUT the SpeedSetting attribute with 1, then sends a Step
        # command with Direction set to Decrease, Wrap set to False, and LowestOff set to True
        #  - After the write operation:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        #  - After the step command is executed:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        #    - Verify that the PercentStting and PercentCurrent attributes are both set to 0
        #    - Verify that the FanMode attribute is set to Off (0)
        # self.step(11)
        print(f"[FC] # self.step(11)")
        speed_setting = 1
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        speed_setting_expected = 0
        percent_setting_expected = 0
        fan_mode_expected = fm_enum.kOff
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.FanMode, ComparisonEnum.Equal, fan_mode_expected),
        ]
        attribute_update = AttributeUpdate(attr.SpeedSetting, speed_setting, write_expect)
        step = Step(direction=sd_enum.kDecrease, expected_attributes=command_expect, wrap=False, lowest_off=True)
        await self.verify_step_attribute_values(attribute_update, step, expect_write_updates=False)

        # *** STEP 12 ***
        # TH writes to the DUT the SpeedSetting attribute with SpeedMax, then sends a Step
        # command with Direction set to Increase, Wrap set to False, and LowestOff set to False
        #  - After the write operation:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        #  - After the step command is executed:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        #    - Verify that the PercentStting and PercentCurrent attributes are both
        #      set to the corresponding values as per the percent formula:
        #      percent=floor(speed/SpeedMax*100)
        #    - Verify that the FanMode attribute is set to High (3)
        # self.step(12)
        print(f"[FC] # self.step(12)")
        speed_setting = self.speed_max
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        speed_setting_expected = self.speed_max
        percent_setting_expected = self.compute_percent_setting(speed_setting_expected)
        fan_mode_expected = fm_enum.kHigh
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.FanMode, ComparisonEnum.Equal, fan_mode_expected),
        ]
        attribute_update = AttributeUpdate(attr.SpeedSetting, speed_setting, write_expect)
        step = Step(direction=sd_enum.kIncrease, expected_attributes=command_expect, wrap=False, lowest_off=False)
        await self.verify_step_attribute_values(attribute_update, step, expect_command_updates=False)

        # *** STEP 13 ***
        # TH writes to the DUT the SpeedSetting attribute with SpeedMax, then sends a Step
        # command with Direction set to Increase, Wrap set to True, and LowestOff set to True
        #  - After the write operation:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        #  - After the step command is executed:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        #    - Verify that the PercentStting and PercentCurrent attributes are both set to 0
        #    - Verify that the FanMode attribute is set to Off (0)
        # self.step(13)
        print(f"[FC] # self.step(13)")
        speed_setting = self.speed_max
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        speed_setting_expected = 0
        percent_setting_expected = 0
        fan_mode_expected = fm_enum.kOff
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.FanMode, ComparisonEnum.Equal, fan_mode_expected),
        ]
        attribute_update = AttributeUpdate(attr.SpeedSetting, speed_setting, write_expect)
        step = Step(direction=sd_enum.kIncrease, expected_attributes=command_expect, wrap=True, lowest_off=True)
        await self.verify_step_attribute_values(attribute_update, step, expect_write_updates=False)

        # *** STEP 14 ***
        # TH writes to the DUT the SpeedSetting attribute with SpeedMax, then sends a Step
        # command with Direction set to Increase, Wrap set to True, and LowestOff set to False
        #  - After the write operation:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        #  - After the step command is executed:
        #    - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1
        #    - Verify that the PercentStting and PercentCurrent attributes are both
        #      set to the corresponding values as per the percent formula:
        #      percent=floor(speed/SpeedMax*100)
        #    - Verify that the FanMode attribute is set to Low (1)
        # self.step(14)
        print(f"[FC] # self.step(14)")
        speed_setting = self.speed_max
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        speed_setting_expected = 1
        percent_setting_expected = self.compute_percent_setting(speed_setting_expected)
        fan_mode_expected = fm_enum.kLow
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting_expected),
            AttributeValueExpected(self.endpoint, attr.FanMode, ComparisonEnum.Equal, fan_mode_expected),
        ]
        attribute_update = AttributeUpdate(attr.SpeedSetting, speed_setting, write_expect)
        step = Step(direction=sd_enum.kIncrease, expected_attributes=command_expect, wrap=True, lowest_off=False)
        await self.verify_step_attribute_values(attribute_update, step)


if __name__ == "__main__":
    default_matter_test_main()
