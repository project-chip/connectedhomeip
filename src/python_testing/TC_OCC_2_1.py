#
#    Copyright (c) 2024 Project CHIP Authors
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


class TC_OCC_2_1(MatterBaseTest):
    async def read_occ_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.OccupancySensing
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_OCC_2_1(self) -> str:
        return "[TC-OCC-2.1] Attributes with DUT as Server"

    def steps_TC_OCC_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read attribute list to determine supported attributes"),
            TestStep(3, "Read Occupancy attribute."),
            TestStep(4, "Read OccupancySensorType attribute."),
            TestStep(5, "Read OccupancySensorTypeBitmap attribute."),
            TestStep(6, "Read HoldTime attribute, if supported"),
            TestStep(7, "Read HoldTimeLimits attribute, if supported"),
            TestStep(8, "Read PIROccupiedToUnoccupiedDelay attribute, if supported"),
            TestStep(9, "Read PIRUnoccupiedToOccupiedDelay attribute, if supported"),
            TestStep(10, "Read PIRUnoccupiedToOccupiedThreshold attribute, if supported"),
            TestStep(11, "Read UltrasonicOccupiedToUnoccupiedDelay attribute, if supported"),
            TestStep(12, "Read UltrasonicUnoccupiedToOccupiedDelay attribute, if supported"),
            TestStep(13, "Read UltrasonicUnoccupiedToOccupiedThreshold attribute, if supported"),
            TestStep(14, "Read PhysicalContactOccupiedToUnoccupiedDelay attribute, if supported"),
            TestStep(15, "Read PhysicalContactUnoccupiedToOccupiedDelay attribute, if supported"),
            TestStep(16, "Read PhysicalContactUnoccupiedToOccupiedThreshold attribute, if supported")
        ]
        return steps

    def pics_TC_OCC_2_1(self) -> list[str]:
        pics = [
            "OCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_OCC_2_1(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.OccupancySensing.Attributes

        self.step(2)
        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(3)
        if attributes.Occupancy.attribute_id in attribute_list:
            occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)

            if occupancy_dut != NullValue:
                asserts.assert_less_equal(occupancy_dut, 0b00000001, "Occupancy attribute is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(4)
        if attributes.OccupancySensorType.attribute_id in attribute_list:
            occupancy_sensor_type_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorType)

            if occupancy_sensor_type_dut != NullValue:
                asserts.assert_less(occupancy_sensor_type_dut, Clusters.Objects.OccupancySensing.Enums.OccupancySensorTypeEnum.kUnknownEnumValue,
                                    "OccupancySensorType is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(5)
        if attributes.OccupancySensorTypeBitmap.attribute_id in attribute_list:
            occupancy_sensor_type_bitmap_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorTypeBitmap)

            if occupancy_sensor_type_bitmap_dut != NullValue:
                asserts.assert_less_equal(occupancy_sensor_type_bitmap_dut, 0b00000111, "OccupancySensorTypeBitmap attribute is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(6)
        if attributes.HoldTime.attribute_id in attribute_list:
            hold_time_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.HoldTime)

            if hold_time_dut != NullValue:
                asserts.assert_less_equal(hold_time_dut_dut, 0xFFFE, "HoldTime attribute is out of range")
                asserts.assert_greater_equal(hold_time_dut_dut, 0, "HoldTime attribute is out of range")
        else:
            logging.info("Test step skipped")

        self.step(7)
        if attributes.HoldTimeLimits.attribute_id in attribute_list:
            hold_time_limits_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.HoldTimeLimits)

            if hold_time_limits_dut != NullValue:
                asserts.assert_less_equal(hold_time_limits_dut.HoldTimeMin, hold_time_limits_dut.HoldTimeMax, "HoldTimeMin is not in valid range")
                asserts.assert_greater_equal(hold_time_limits_dut.HoldTimeMin, 0, "HoldTimeMin is not in valid range")
                asserts.assert_less_equal(hold_time_limits_dut.HoldTimeMax, 0xFFFE, "HoldTimeMin is not in valid range")
                asserts.assert_greater_equal(hold_time_limits_dut.HoldTimeMax, hold_time_limits_dut.HoldTimeMin, "HoldTimeMin is not in valid range")
                asserts.assert_less_equal(hold_time_limits_dut.HoldTimeDefault, hold_time_limits_dut.HoldTimeMax, "HoldTimeMin is not in valid range")
                asserts.assert_greater_equal(hold_time_limits_dut.HoldTimeDefault, hold_time_limits_dut.HoldTimeMin, "HoldTimeMin is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(8)
        if attributes.PIROccupiedToUnoccupiedDelay.attribute_id in attribute_list:
            pir_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIROccupiedToUnoccupiedDelay)

            if pir_otou_delay_dut != NullValue:
                asserts.assert_less_equal(pir_otou_delay_dut, 0xFFFE, "PIROccupiedToUnoccupiedDelay is not in valid range")
                asserts.assert_greater_equal(pir_otou_delay_dut, 0, "PIROccupiedToUnoccupiedDelay is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(9)
        if attributes.PIRUnoccupiedToOccupiedDelay.attribute_id in attribute_list:
            pir_utoo_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIRUnoccupiedToOccupiedDelay)

            if pir_utoo_delay_dut != NullValue:
                asserts.assert_less_equal(pir_utoo_delay_dut, 0xFFFE, "PIRUnoccupiedToOccupiedDelay is not in valid range")
                asserts.assert_greater_equal(pir_utoo_delay_dut, 0, "PIRUnoccupiedToOccupiedDelay is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(10)
        if attributes.PIRUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
            pir_utoo_threshold_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIRUnoccupiedToOccupiedThreshold)

            if pir_utoo_threshold_dut != NullValue:
                asserts.assert_less_equal(pir_utoo_threshold_dut, 0xFE, "PIRUnoccupiedToOccupiedThreshold is not in valid range")
                asserts.assert_greater_equal(pir_utoo_threshold_dut, 0, "PIRUnoccupiedToOccupiedThreshold is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(11)
        if attributes.UltrasonicOccupiedToUnoccupiedDelay.attribute_id in attribute_list:
            ultrasonic_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicOccupiedToUnoccupiedDelay)

            if ultrasonic_otou_delay_dut != NullValue:
                asserts.assert_less_equal(ultrasonic_otou_delay_dut, 0xFFFE, "UltrasonicOccupiedToUnoccupiedDelay is not in valid range")
                asserts.assert_greater_equal(ultrasonic_otou_delay_dut, 0, "UltrasonicOccupiedToUnoccupiedDelay is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(12)
        if attributes.UltrasonicUnoccupiedToOccupiedDelay.attribute_id in attribute_list:
            ultrasonic_utoo_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicUnoccupiedToOccupiedDelay)

            if ultrasonic_utoo_delay_dut != NullValue:
                asserts.assert_less_equal(ultrasonic_utoo_delay_dut, 0xFFFE, "UltrasonicUnoccupiedToOccupiedDelay is not in valid range")
                asserts.assert_greater_equal(ultrasonic_utoo_delay_dut, 0, "UltrasonicUnoccupiedToOccupiedDelay is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(13)
        if attributes.UltrasonicUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
            ultrasonic_utoo_threshold_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicUnoccupiedToOccupiedThreshold)

            if ultrasonic_utoo_threshold_dut != NullValue:
                asserts.assert_less_equal(ultrasonic_utoo_threshold_dut, 0xFE, "UltrasonicUnoccupiedToOccupiedThreshold is not in valid range")
                asserts.assert_greater_equal(ultrasonic_utoo_threshold_dut, 0, "UltrasonicUnoccupiedToOccupiedThreshold is not in valid range")
        else:
            logging.info("Test step skipped")

       self.step(14)
        if attributes.PhysicalContactOccupiedToUnoccupiedDelay.attribute_id in attribute_list:
            phycontact_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactOccupiedToUnoccupiedDelay)

            if phycontact_otou_delay_dut != NullValue:
                asserts.assert_less_equal(phycontact_otou_delay_dut, 0xFFFE, "PhysicalContactOccupiedToUnoccupiedDelay is not in valid range")
                asserts.assert_greater_equal(phycontact_otou_delay_dut, 0, "PhysicalContactOccupiedToUnoccupiedDelay is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(15)
        if attributes.PhysicalContactUnoccupiedToOccupiedDelay.attribute_id in attribute_list:
            phycontact_utoo_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactUnoccupiedToOccupiedDelay)

            if phycontact_utoo_delay_dut != NullValue:
                asserts.assert_less_equal(phycontact_utoo_delay_dut, 0xFFFE, "PhysicalContactUnoccupiedToOccupiedDelay is not in valid range")
                asserts.assert_greater_equal(phycontact_utoo_delay_dut, 0, "PhysicalContactUnoccupiedToOccupiedDelay is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(16)
        if attributes.PhysicalContactUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
            phycontact_utoo_threshold_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactUnoccupiedToOccupiedThreshold)

            if phycontact_utoo_threshold_dut != NullValue:
                asserts.assert_less_equal(phycontact_utoo_threshold_dut, 0xFE, "PhysicalContactUnoccupiedToOccupiedThreshold is not in valid range")
                asserts.assert_greater_equal(phycontact_utoo_threshold_dut, 0, "PhysicalContactUnoccupiedToOccupiedThreshold is not in valid range")
        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()