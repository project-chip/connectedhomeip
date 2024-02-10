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

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_VALCC_2_1(MatterBaseTest):
    async def read_valcc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ValveConfigurationAndControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_VALCC_2_1(self) -> str:
        return "[TC-VALCC-2.1] Attributes with DUT as Server"

    def steps_TC_VALCC_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read attribute list to determine supported attributes"),
            TestStep(3, "Read OpenDuration attribute, if supported"),
            TestStep(4, "Read DefaultOpenDuration attribute, if supported"),
            TestStep(5, "Read AutoCloseTime attribute, if supported"),
            TestStep(6, "Read RemainingDuration attribute, if supported"),
            TestStep(7, "Read CurrentState attribute, if supported"),
            TestStep(8, "Read TargetState attribute, if supported"),
            TestStep(9, "Read CurrentLevel attribute, if supported"),
            TestStep(10, "Read TargetLevel attribute, if supported"),
            TestStep(11, "Read DefaultOpenLevel attribute, if supported"),
            TestStep(12, "Read ValveFault attribute, if supported"),
            TestStep(13, "Read LevelStep attribute, if supported")
        ]
        return steps

    def pics_TC_VALCC_2_1(self) -> list[str]:
        pics = [
            "VALCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_VALCC_2_1(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.ValveConfigurationAndControl.Attributes

        self.step(2)
        attribute_list = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(3)
        if attributes.OpenDuration.attribute_id in attribute_list:
            open_duration_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.OpenDuration)

            if open_duration_dut is not NullValue:
                asserts.assert_less_equal(open_duration_dut, 0xFFFFFFFE, "OpenDuration attribute is out of range")
                asserts.assert_greater_equal(open_duration_dut, 1, "OpenDuration attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.step(4)
        if attributes.DefaultOpenDuration.attribute_id in attribute_list:
            default_open_duration_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.DefaultOpenDuration)

            if default_open_duration_dut is not NullValue:
                asserts.assert_less_equal(default_open_duration_dut, 0xFFFFFFFE, "DefaultOpenDuration attribute is out of range")
                asserts.assert_greater_equal(default_open_duration_dut, 1, "DefaultOpenDuration attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.step(5)
        if attributes.AutoCloseTime.attribute_id in attribute_list:
            auto_close_time_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.AutoCloseTime)

            if auto_close_time_dut is not NullValue:
                asserts.assert_less_equal(auto_close_time_dut, 0xFFFFFFFFFFFFFFFE, "OpenDuration attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.step(6)
        if attributes.RemainingDuration.attribute_id in attribute_list:
            remaining_duration_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.RemainingDuration)

            if remaining_duration_dut is not NullValue:
                asserts.assert_less_equal(remaining_duration_dut, 0xFFFFFFFE, "RemainingDuration attribute is out of range")
                asserts.assert_greater_equal(remaining_duration_dut, 1, "RemainingDuration attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.step(7)
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            if current_state_dut is not NullValue:
                asserts.assert_less(current_state_dut, Clusters.Objects.ValveConfigurationAndControl.Enums.ValveStateEnum.kUnknownEnumValue,
                                    "CurrentState is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(8)
        if attributes.TargetState.attribute_id in attribute_list:
            target_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)

            if target_state_dut is not NullValue:
                asserts.assert_less(target_state_dut, Clusters.Objects.ValveConfigurationAndControl.Enums.ValveStateEnum.kUnknownEnumValue,
                                    "TargetState is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(9)
        if attributes.CurrentLevel.attribute_id in attribute_list:
            current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)

            if current_level_dut is not NullValue:
                asserts.assert_less_equal(current_level_dut, 100, "CurrentLevel attribute is out of range")
                asserts.assert_greater_equal(current_level_dut, 0, "CurrentLevel attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.step(10)
        if attributes.TargetLevel.attribute_id in attribute_list:
            target_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetLevel)

            if target_level_dut is not NullValue:
                asserts.assert_less_equal(target_level_dut, 100, "TargetLevel attribute is out of range")
                asserts.assert_greater_equal(target_level_dut, 0, "TargetLevel attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.step(11)
        if attributes.DefaultOpenLevel.attribute_id in attribute_list:
            default_open_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.DefaultOpenLevel)

            if default_open_level_dut is not NullValue:
                asserts.assert_less_equal(default_open_level_dut, 100, "DefaultOpenLevel attribute is out of range")
                asserts.assert_greater_equal(default_open_level_dut, 1, "DefaultOpenLevel attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.step(12)
        if attributes.ValveFault.attribute_id in attribute_list:
            valve_fault_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.ValveFault)

            asserts.assert_less_equal(valve_fault_dut, 0b00000111, "ValveFault is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(13)
        if attributes.LevelStep.attribute_id in attribute_list:
            level_step_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.LevelStep)

            asserts.assert_less_equal(level_step_dut, 50, "LevelStep attribute is out of range")
            asserts.assert_greater_equal(level_step_dut, 1, "LevelStep attribute is out of range")
        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
