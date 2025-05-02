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


@dataclass
class Update:
    action: ActionEnum
    attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor = None
    attribute_value: Any = None
    step: Clusters.FanControl.Commands.Step = None


class TC_FAN_3_5(MatterBaseTest):
    def desc_TC_FAN_3_5(self) -> str:
        return "[TC-FAN-3.5] Optional step functionality with DUT as Server"

    def steps_TC_FAN_3_5(self):
        return [TestStep("1", "[FC] Commissioning already done.", is_commissioning=True),
                TestStep("2", "[FC] TH checks the DUT for support of the Step and MultiSpeed features.",
                         "If the DUT does not support both features, the test is skipped."),
                TestStep("3", "[FC] TH reads from the DUT the SpeedMax attribute.", "Store value for future reference."),
                TestStep("4", "[FC] TH subscribes to the FanControl cluster.", "Enables the TH to receive attribute updates."),

                TestStep("5", "[FC] TH writes to the DUT the PercentSetting attribute with value 50.",
                         "- Verify that the PercentStting and PercentCurrent attributes are both set to 50."),
                TestStep("5a", "[FC] TH sends command Step with: Direction = Increase, Wrap = False, LowestOff = True.",
                         "Verify that the PercentStting and PercentCurrent attributes are both greater than 50."),

                TestStep("6", "[FC] TH writes to the DUT the PercentSetting attribute with value 50.",
                         "- Verify that the PercentStting and PercentCurrent attributes are both set to 50."),
                TestStep("6a", "[FC] TH sends command Step with: Direction = Decrease, Wrap = False, LowestOff = True.",
                         "Verify that the PercentStting and PercentCurrent attributes are both less than 50."),

                TestStep("7", "[FC] TH writes to the DUT the SpeedSetting attribute with value 0.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0."),
                TestStep("7a", "[FC] TH sends command Step with: Direction = Decrease, Wrap = False, LowestOff = True.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0. - Verify that the PercentStting and PercentCurrent attributes are both set to 0. - Verify that the FanMode attribute is set to Off (0)."),

                TestStep("8", "[FC] TH writes to the DUT the SpeedSetting attribute with value 0.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0."),
                TestStep("8a", "[FC] TH sends command Step with: Direction = Decrease, Wrap = True, LowestOff = True.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax. - Verify that the PercentStting and PercentCurrent attributes are both set to the corresponding values as per the percent formula: percent=floor(speed/SpeedMax*100). - Verify that the FanMode attribute is set to High (3)."),

                TestStep("9", "[FC] TH writes to the DUT the SpeedSetting attribute with value 1.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1."),
                TestStep("9a", "[FC] TH sends command Step with: Direction = Decrease, Wrap = False, LowestOff = False.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1. - Verify that the PercentStting and PercentCurrent attributes are both set to the corresponding values as per the percent formula: percent=floor(speed/SpeedMax*100). - Verify that the FanMode attribute is set to Low (1)."),

                TestStep("10", "[FC] TH writes to the DUT the SpeedSetting attribute with value 1.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1."),
                TestStep("10a", "[FC] TH sends command Step with: Direction = Decrease, Wrap = False, LowestOff = True.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0. - Verify that the PercentStting and PercentCurrent attributes are both set to 0. - Verify that the FanMode attribute is set to Off (0)."),

                TestStep("11", "[FC] TH writes to the DUT the SpeedSetting attribute with value 1.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1."),
                TestStep("11a", "[FC] TH sends command Step with: Direction = Decrease, Wrap = True, LowestOff = False.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax. - Verify that the PercentStting and PercentCurrent attributes are both set to the corresponding values as per the percent formula: percent=floor(speed/SpeedMax*100). - Verify that the FanMode attribute is set to High (3)."),

                TestStep("12", "[FC] TH writes to the DUT the SpeedSetting attribute with value SpeedMax.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax."),
                TestStep("12a", "[FC] TH sends command Step with: Direction = Increase, Wrap = False, LowestOff = True.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax. - Verify that the PercentStting and PercentCurrent attributes are both set to the corresponding values as per the percent formula: percent=floor(speed/SpeedMax*100). - Verify that the FanMode attribute is set to High (3)."),

                TestStep("13", "[FC] TH writes to the DUT the SpeedSetting attribute with value SpeedMax.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax."),
                TestStep("13a", "[FC] TH sends command Step with: Direction = Increase, Wrap = True, LowestOff = True.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0. - Verify that the PercentStting and PercentCurrent attributes are both set to 0. - Verify that the FanMode attribute is set to Off (0)."),

                TestStep("14", "[FC] TH writes to the DUT the SpeedSetting attribute with value SpeedMax.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax."),
                TestStep("14a", "[FC] TH sends command Step with: Direction = Increase, Wrap = True, LowestOff = False.",
                         "- Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1. - Verify that the PercentStting and PercentCurrent attributes are both set to the corresponding values as per the percent formula: percent=floor(speed/SpeedMax*100). - Verify that the FanMode attribute is set to Low (1)."),
                ]

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
        fm_enum = cluster.Enums.FanModeEnum
        self.timeout_sec: float = 0.5

        # *** STEP 1 ***
        # Commissioning already done
        self.step("1")

        # *** STEP 2 ***
        # TH checks the DUT for support of the Step and MultiSpeed features
        #  - If the DUT does not support both features, the test is skipped
        self.step("2")
        feature_map = await self.read_setting(attr.FeatureMap)
        self.supports_step = bool(feature_map & cluster.Bitmaps.Feature.kStep)
        self.supports_multi_speed = bool(feature_map & cluster.Bitmaps.Feature.kMultiSpeed)
        if not self.supports_step or not self.supports_multi_speed:
            logger.info("Both the Step and MultiSpeed features must be supported by the DUT for this test, skipping test.")
            return

        # *** STEP 3 ***
        # TH reads from the DUT the SpeedMax attribute
        #  - Store value for future reference
        self.step("3")
        self.speed_max = await self.read_setting(attr.SpeedMax)

        # *** STEP 4 ***
        # TH subscribes to the FanControl cluster
        #  - Enables the TH to receive attribute updates
        self.step("4")
        self.attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        await self.attribute_subscription.start(self.default_controller, self.dut_node_id, self.endpoint)

        # *** STEP 5 ***
        # TH writes to the DUT the PercentSetting attribute with value 50
        #  - Verify that the PercentStting and PercentCurrent attributes are both set to 50
        self.step("5")
        percent_setting = 50
        update = Update(attribute=attr.PercentSetting, attribute_value=percent_setting, action=ActionEnum.Write)
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=write_expect)

        # *** STEP 5a ***
        # TH sends command Step with: Direction = Increase, Wrap = False, LowestOff = True
        #  - Verify that the PercentStting and PercentCurrent attributes are both greater than 50
        self.step("5a")
        step = cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=True)
        update = Update(step=step, action=ActionEnum.Command)
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.GreaterThan, percent_setting),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.GreaterThan, percent_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=command_expect)

        # *** STEP 6 ***
        # TH writes to the DUT the PercentSetting attribute with value 50
        #  - Verify that the PercentStting and PercentCurrent attributes are both set to 50
        self.step("6")
        percent_setting = 50
        update = Update(attribute=attr.PercentSetting, attribute_value=percent_setting, action=ActionEnum.Write)
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.Equal, percent_setting),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.Equal, percent_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=write_expect)

        # *** STEP 6a ***
        # TH sends command Step with: Direction = Decrease, Wrap = False, LowestOff = True
        #  - Verify that the PercentStting and PercentCurrent attributes are both less than 50
        self.step("6a")
        step = cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=True)
        update = Update(step=step, action=ActionEnum.Command)
        command_expect = [
            AttributeValueExpected(self.endpoint, attr.PercentSetting, ComparisonEnum.LessThan, percent_setting),
            AttributeValueExpected(self.endpoint, attr.PercentCurrent, ComparisonEnum.LessThan, percent_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=command_expect)

        # *** STEP 7 ***
        # TH writes to the DUT the SpeedSetting attribute with value 0
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        self.step("7")
        speed_setting = 0
        update = Update(attribute=attr.SpeedSetting, attribute_value=speed_setting, action=ActionEnum.Write)
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=write_expect)

        # *** STEP 7a ***
        # TH sends command Step with: Direction = Decrease, Wrap = False, LowestOff = True
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        #  - Verify that the PercentStting and PercentCurrent attributes are both set to 0
        #  - Verify that the FanMode attribute is set to Off (0)
        self.step("7a")
        step = cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=True)
        update = Update(step=step, action=ActionEnum.Command)
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
        await self.update_and_verify_attribute_values(update, expect_updates=False, expected_attributes=command_expect)

        # *** STEP 8 ***
        # TH writes to the DUT the SpeedSetting attribute with value 0
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        self.step("8")
        speed_setting = 0
        update = Update(attribute=attr.SpeedSetting, attribute_value=speed_setting, action=ActionEnum.Write)
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=False, expected_attributes=write_expect)

        # *** STEP 8a ***
        # TH sends command Step with: Direction = Decrease, Wrap = True, LowestOff = True
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        #  - Verify that the PercentStting and PercentCurrent attributes are both set to the
        #    corresponding values as per the percent formula: percent=floor(speed/SpeedMax*100)
        #  - Verify that the FanMode attribute is set to High (3)
        self.step("8a")
        step = cmd.Step(direction=sd_enum.kDecrease, wrap=True, lowestOff=True)
        update = Update(step=step, action=ActionEnum.Command)
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
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=command_expect)

        # *** STEP 9 ***
        # TH writes to the DUT the SpeedSetting attribute with value 1
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1
        self.step("9")
        speed_setting = 1
        update = Update(attribute=attr.SpeedSetting, attribute_value=speed_setting, action=ActionEnum.Write)
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=write_expect)

        # *** STEP 9a ***
        # TH sends command Step with: Direction = Decrease, Wrap = False, LowestOff = False
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1
        #  - Verify that the PercentStting and PercentCurrent attributes are both set to the
        #    corresponding values as per the percent formula: percent=floor(speed/SpeedMax*100)
        #  - Verify that the FanMode attribute is set to Low (1)
        self.step("9a")
        step = cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=False)
        update = Update(step=step, action=ActionEnum.Command)
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
        await self.update_and_verify_attribute_values(update, expect_updates=False, expected_attributes=command_expect)

        # *** STEP 10 ***
        # TH writes to the DUT the SpeedSetting attribute with value 1
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1
        self.step("10")
        speed_setting = 1
        update = Update(attribute=attr.SpeedSetting, attribute_value=speed_setting, action=ActionEnum.Write)
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=False, expected_attributes=write_expect)

        # *** STEP "10a" ***
        # TH sends command Step with: Direction = Decrease, Wrap = False, LowestOff = True
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        #  - Verify that the PercentStting and PercentCurrent attributes are both set to 0
        #  - Verify that the FanMode attribute is set to Off (0)
        self.step("10a")
        step = cmd.Step(direction=sd_enum.kDecrease, wrap=False, lowestOff=True)
        update = Update(step=step, action=ActionEnum.Command)
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
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=command_expect)

        # *** STEP 11 ***
        # TH writes to the DUT the SpeedSetting attribute with value 1
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1
        self.step("11")
        speed_setting = 1
        update = Update(attribute=attr.SpeedSetting, attribute_value=speed_setting, action=ActionEnum.Write)
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=write_expect)

        # *** STEP 11a ***
        # TH sends command Step with: Direction = Decrease, Wrap = True, LowestOff = False
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        #  - Verify that the PercentStting and PercentCurrent attributes are both set to the
        #    corresponding values as per the percent formula: percent=floor(speed/SpeedMax*100)
        #  - Verify that the FanMode attribute is set to High (3)
        self.step("11a")
        step = cmd.Step(direction=sd_enum.kDecrease, wrap=True, lowestOff=False)
        update = Update(step=step, action=ActionEnum.Command)
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
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=command_expect)

        # *** STEP 12 ***
        # TH writes to the DUT the SpeedSetting attribute with value SpeedMax
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        self.step("12")
        speed_setting = self.speed_max
        update = Update(attribute=attr.SpeedSetting, attribute_value=speed_setting, action=ActionEnum.Write)
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=False, expected_attributes=write_expect)

        # *** STEP 12a ***
        # TH sends command Step with: Direction = Increase, Wrap = False, LowestOff = True
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        #  - Verify that the PercentStting and PercentCurrent attributes are both set to the
        #    corresponding values as per the percent formula: percent=floor(speed/SpeedMax*100)
        #  - Verify that the FanMode attribute is set to High (3)
        self.step("12a")
        step = cmd.Step(direction=sd_enum.kIncrease, wrap=False, lowestOff=True)
        update = Update(step=step, action=ActionEnum.Command)
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
        await self.update_and_verify_attribute_values(update, expect_updates=False, expected_attributes=command_expect)

        # *** STEP 13 ***
        # TH writes to the DUT the SpeedSetting attribute with value SpeedMax
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        self.step("13")
        speed_setting = self.speed_max
        update = Update(attribute=attr.SpeedSetting, attribute_value=speed_setting, action=ActionEnum.Write)
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=False, expected_attributes=write_expect)

        # *** STEP "13a" ***
        # TH sends command Step with: Direction = Increase, Wrap = True, LowestOff = True
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 0
        #  - Verify that the PercentStting and PercentCurrent attributes are both set to 0
        #  - Verify that the FanMode attribute is set to Off (0)
        self.step("13a")
        step = cmd.Step(direction=sd_enum.kIncrease, wrap=True, lowestOff=True)
        update = Update(step=step, action=ActionEnum.Command)
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
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=command_expect)

        # *** STEP 14 ***
        # TH writes to the DUT the SpeedSetting attribute with value SpeedMax
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to SpeedMax
        self.step("14")
        speed_setting = self.speed_max
        update = Update(attribute=attr.SpeedSetting, attribute_value=speed_setting, action=ActionEnum.Write)
        write_expect = [
            AttributeValueExpected(self.endpoint, attr.SpeedSetting, ComparisonEnum.Equal, speed_setting),
            AttributeValueExpected(self.endpoint, attr.SpeedCurrent, ComparisonEnum.Equal, speed_setting),
        ]
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=write_expect)

        # *** STEP "14a" ***
        # TH sends command Step with: Direction = Increase, Wrap = True, LowestOff = False
        #  - Verify that the SpeedSetting and SpeedCurrent attributes are both set to 1
        #  - Verify that the PercentStting and PercentCurrent attributes are both set to the
        #    corresponding values as per the percent formula: percent=floor(speed/SpeedMax*100)
        #  - Verify that the FanMode attribute is set to Low (1)
        self.step("14a")
        step = cmd.Step(direction=sd_enum.kIncrease, wrap=True, lowestOff=False)
        update = Update(step=step, action=ActionEnum.Command)
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
        await self.update_and_verify_attribute_values(update, expect_updates=True, expected_attributes=command_expect)


if __name__ == "__main__":
    default_matter_test_main()
