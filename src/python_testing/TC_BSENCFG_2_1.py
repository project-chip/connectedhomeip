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
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_BSENCFG_2_1(MatterBaseTest):
    async def read_ts_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    @async_test_body
    async def test_TC_BSENCFG_2_1(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.print_step(1, "Commissioning, already done")
        attributes = Clusters.BooleanStateConfiguration.Attributes

        self.print_step(2, "Read attribute list to determine supported attributes")
        attribute_list = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.print_step(3, "Read SensitivityLevel attribute, if supported")
        if attributes.SensitivityLevel.attribute_id in attribute_list:
            sensitivity_level_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.SensitivityLevel)
            asserts.assert_less(sensitivity_level_dut, Clusters.Objects.BooleanStateConfiguration.Enums.SensitivityEnum.kUnknownEnumValue,
                                "SensitivityLevel is not in valid range")
        else:
            logging.info("Test step skipped")

        self.print_step(4, "Read AlarmsActive attribute, if supported")
        if attributes.AlarmsActive.attribute_id in attribute_list:
            alarms_active_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_less_equal(alarms_active_dut, 0b00000011, "AlarmsActive is not in valid range")
        else:
            logging.info("Test step skipped")

        self.print_step(5, "Read AlarmsSuppressed attribute, if supported")
        if attributes.AlarmsSuppressed.attribute_id in attribute_list:
            alarms_suppressed_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsSuppressed)
            asserts.assert_less_equal(alarms_suppressed_dut, 0b00000011, "AlarmsSuppressed is not in valid range")
        else:
            logging.info("Test step skipped")

        self.print_step(5, "Read AlarmsEnabled attribute, if supported")
        if attributes.AlarmsEnabled.attribute_id in attribute_list:
            alarms_enabled_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsEnabled)
            asserts.assert_less_equal(alarms_enabled_dut, 0b00000011, "AlarmsEnabled is not in valid range")
        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
