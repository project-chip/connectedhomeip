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
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_VCC_2_1(MatterBaseTest):
    async def read_ts_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ValveConfigurationAndControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    @async_test_body
    async def test_TC_VCC_2_1(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.print_step(1, "Commissioning, already done")
        attributes = Clusters.ValveConfigurationAndControl.Attributes

        self.print_step(2, "Read attribute list to determine supported attributes")
        attribute_list = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        print(attribute_list)

        self.print_step(3, "Read OpenDuration attribute, if supported")
        if attributes.OpenDuration.attribute_id in attribute_list:
            open_duration_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.OpenDuration)

            if open_duration_dut is not NullValue:
                asserts.assert_less_equal(open_duration_dut, 0xFFFFFFFE, "OpenDuration attribute is out of range")
                asserts.assert_greater_equal(open_duration_dut, 1, "OpenDuration attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.print_step(4, "Read AutoCloseTime attribute, if supported")
        if attributes.AutoCloseTime.attribute_id in attribute_list:
            auto_close_time_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.AutoCloseTime)

            if auto_close_time_dut is not NullValue:
                asserts.assert_less_equal(auto_close_time_dut, 0xFFFFFFFFFFFFFFFE, "OpenDuration attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.print_step(5, "Read RemainingDuration attribute, if supported")
        if attributes.RemainingDuration.attribute_id in attribute_list:
            remaining_duration_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.RemainingDuration)

            if remaining_duration_dut is not NullValue:
                asserts.assert_less_equal(remaining_duration_dut, 0xFFFFFFFE, "RemainingDuration attribute is out of range")
                asserts.assert_greater_equal(remaining_duration_dut, 1, "RemainingDuration attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.print_step(6, "Read CurrentState attribute, if supported")
        if attributes.CurrentState.attribute_id in attribute_list:
            current_state_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)

            if current_state_dut is not NullValue:
                asserts.assert_less(current_state_dut, Clusters.Objects.ValveConfigurationAndControl.Enums.ValveStateEnum.kUnknownEnumValue,
                                    "CurrentState is not in valid range")
        else:
            logging.info("Test step skipped")

        self.print_step(7, "Read TargetState attribute, if supported")
        if attributes.TargetState.attribute_id in attribute_list:
            target_state_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)

            if target_state_dut is not NullValue:
                asserts.assert_less(target_state_dut, Clusters.Objects.ValveConfigurationAndControl.Enums.ValveStateEnum.kUnknownEnumValue,
                                    "TargetState is not in valid range")
        else:
            logging.info("Test step skipped")

        self.print_step(8, "Read StartUpState attribute, if supported")
        if attributes.StartUpState.attribute_id in attribute_list:
            start_up_state_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.StartUpState)

            asserts.assert_less(start_up_state_dut, Clusters.Objects.ValveConfigurationAndControl.Enums.ValveStateEnum.kUnknownEnumValue,
                                "StartUpState is not in valid range")
        else:
            logging.info("Test step skipped")

        self.print_step(9, "Read CurrentLevel attribute, if supported")
        if attributes.CurrentLevel.attribute_id in attribute_list:
            current_level_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)

            if current_level_dut is not NullValue:
                asserts.assert_less_equal(current_level_dut, 100, "CurrentLevel attribute is out of range")
                asserts.assert_greater_equal(current_level_dut, 0, "CurrentLevel attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.print_step(10, "Read TargetLevel attribute, if supported")
        if attributes.TargetLevel.attribute_id in attribute_list:
            target_level_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetLevel)

            if target_level_dut is not NullValue:
                asserts.assert_less_equal(target_level_dut, 100, "TargetLevel attribute is out of range")
                asserts.assert_greater_equal(target_level_dut, 0, "TargetLevel attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.print_step(11, "Read OpenLevel attribute, if supported")
        if attributes.OpenLevel.attribute_id in attribute_list:
            open_level_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.OpenLevel)

            if open_level_dut is not NullValue:
                asserts.assert_less_equal(open_level_dut, 100, "OpenLevel attribute is out of range")
                asserts.assert_greater_equal(open_level_dut, 1, "OpenLevel attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.print_step(12, "Read ValveFault attribute, if supported")
        if attributes.ValveFault.attribute_id in attribute_list:
            valve_fault_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.ValveFault)

            asserts.assert_less_equal(valve_fault_dut, 0b00000111, "ValveFault is not in valid range")
        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
