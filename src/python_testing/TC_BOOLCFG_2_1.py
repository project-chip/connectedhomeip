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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import functools
import logging
from operator import ior

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_BOOLCFG_2_1(MatterBaseTest):
    async def read_boolcfg_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOLCFG_2_1(self) -> str:
        return "[TC-BOOLCFG-2.1] Attributes with DUT as Server"

    def steps_TC_BOOLCFG_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read attribute list to determine supported attributes"),
            TestStep(3, "Read SupportedSensitivityLevels attribute, if supported"),
            TestStep(4, "Read CurrentSensitivityLevel attribute, if supported"),
            TestStep(5, "Read DefaultSensitivityLevel attribute, if supported"),
            TestStep(6, "Read AlarmsActive attribute, if supported"),
            TestStep(7, "Read AlarmsSuppressed attribute, if supported"),
            TestStep(8, "Read AlarmsEnabled attribute, if supported"),
            TestStep(9, "Read AlarmsSupported attribute, if supported"),
            TestStep(10, "Read SensorFault attribute, if supported"),
        ]
        return steps

    def pics_TC_BOOLCFG_2_1(self) -> list[str]:
        pics = [
            "BOOLCFG.S",
        ]
        return pics

    @async_test_body
    async def test_TC_BOOLCFG_2_1(self):

        endpoint = self.user_params.get("endpoint", 1)
        all_alarm_mode_bitmap_bits = functools.reduce(
            ior, [b.value for b in Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap])
        all_sensor_fault_bitmap_bits = functools.reduce(
            ior, [b.value for b in Clusters.BooleanStateConfiguration.Bitmaps.SensorFaultBitmap])

        self.step(1)
        attributes = Clusters.BooleanStateConfiguration.Attributes

        self.step(2)
        attribute_list = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        number_of_supported_levels = 0

        self.step(3)
        if attributes.SupportedSensitivityLevels.attribute_id in attribute_list:
            number_of_supported_levels = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.SupportedSensitivityLevels)
            asserts.assert_less_equal(number_of_supported_levels, 10, "SupportedSensitivityLevels attribute is out of range")
            asserts.assert_greater_equal(number_of_supported_levels, 2, "SupportedSensitivityLevels attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.step(4)
        if attributes.CurrentSensitivityLevel.attribute_id in attribute_list:
            current_sensitivity_level_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentSensitivityLevel)
            asserts.assert_less_equal(current_sensitivity_level_dut, number_of_supported_levels,
                                      "CurrentSensitivityLevel is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(5)
        if attributes.DefaultSensitivityLevel.attribute_id in attribute_list:
            default_sensitivity_level_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.DefaultSensitivityLevel)
            asserts.assert_less_equal(default_sensitivity_level_dut, number_of_supported_levels,
                                      "DefaultSensitivityLevel is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(6)
        if attributes.AlarmsActive.attribute_id in attribute_list:
            alarms_active_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(alarms_active_dut & ~all_alarm_mode_bitmap_bits, 0, "AlarmsActive is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(7)
        if attributes.AlarmsSuppressed.attribute_id in attribute_list:
            alarms_suppressed_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsSuppressed)
            asserts.assert_equal(alarms_suppressed_dut & ~all_alarm_mode_bitmap_bits, 0, "AlarmsSuppressed is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(8)
        if attributes.AlarmsEnabled.attribute_id in attribute_list:
            alarms_enabled_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsEnabled)
            asserts.assert_equal(alarms_enabled_dut & ~all_alarm_mode_bitmap_bits, 0, "AlarmsEnabled is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(9)
        if attributes.AlarmsSupported.attribute_id in attribute_list:
            alarms_supported_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsSupported)
            asserts.assert_equal(alarms_supported_dut & ~all_alarm_mode_bitmap_bits, 0, "AlarmsSupported is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(10)
        if attributes.SensorFault.attribute_id in attribute_list:
            sensor_fault_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.SensorFault)
            asserts.assert_equal(sensor_fault_dut & ~all_sensor_fault_bitmap_bits, 0, "SensorFault is not in valid range")
        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
