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
        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(2)
        if attributes.Occupancy.attribute_id in attribute_list:
            occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)
            asserts.assert_less_equal(occupancy_dut, 0b00000001, "Occupancy attribute is not in valid range")
        else:
            logging.info("Occupancy attribute is a mandatory attribute. Test step fails.")
            asserts.fail("Missing mandatory attribute Occupancy")

        self.step(3)
        if attributes.OccupancySensorType.attribute_id in attribute_list:
            occupancy_sensor_type_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorType)
            asserts.assert_less(occupancy_sensor_type_dut, Clusters.Objects.OccupancySensing.Enums.OccupancySensorTypeEnum.kUnknownEnumValue,
                                "OccupancySensorType is not in valid range")
        else:
            logging.info("OccupancySensorType attribute is a mandatory attribute. Test step fails")
            asserts.fail("Missing mandatory attribute OccupancySensorType")

        self.step(4)
        if attributes.OccupancySensorTypeBitmap.attribute_id in attribute_list:
            occupancy_sensor_type_bitmap_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorTypeBitmap)
            asserts.assert_less_equal(occupancy_sensor_type_bitmap_dut, 0b00000111, "OccupancySensorTypeBitmap attribute is not in valid range")
        else:
            logging.info("OccupancySensorTypeBitmap attribute is a mandatory attribute. Test step fails")
            asserts.fail("Missing mandatory attribute OccupancySensorTypeBitmap")

        self.step(5)
        if attributes.HoldTimeLimits.attribute_id in attribute_list:
            if attributes.HoldTime.attribute_id in attribute_list: # check HoldTime conformance
                hold_time_limits_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.HoldTimeLimits)

                asserts.assert_less_equal(hold_time_limits_dut.HoldTimeMin, hold_time_limits_dut.HoldTimeMax, "HoldTimeMin is not in valid range")
                asserts.assert_greater_equal(hold_time_limits_dut.HoldTimeMin, 0, "HoldTimeMin is not in valid range")
                asserts.assert_less_equal(hold_time_limits_dut.HoldTimeMax, 0xFFFE, "HoldTimeMin is not in valid range")
                asserts.assert_greater_equal(hold_time_limits_dut.HoldTimeMax, hold_time_limits_dut.HoldTimeMin, "HoldTimeMin is not in valid range")
                asserts.assert_less_equal(hold_time_limits_dut.HoldTimeDefault, hold_time_limits_dut.HoldTimeMax, "HoldTimeMin is not in valid range")
                asserts.assert_greater_equal(hold_time_limits_dut.HoldTimeDefault, hold_time_limits_dut.HoldTimeMin, "HoldTimeMin is not in valid range")
            else:
                logging.info("HoldTime conformance failed.  Test step skipped")

        else:
            logging.info("HoldTimeLimits not supported. Test step skipped")
        
        self.step(6)
        if attributes.HoldTime.attribute_id in attribute_list:
            hold_time_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.HoldTime)

            asserts.assert_less_equal(hold_time_dut, 0xFFFE, "HoldTime attribute is out of range")
            asserts.assert_greater_equal(hold_time_dut, 0, "HoldTime attribute is out of range")
        else:
            logging.info("HoldTime not supported. The rest of legacy attribute test can be skipped")

        self.step(7)
        if attributes.PIROccupiedToUnoccupiedDelay.attribute_id in attribute_list:
            if ((occupancy_sensor_type_bitmap_dut == 0b00000001) | ((occupancy_sensor_type_bitmap_dut != 0b00000001)&(occupancy_sensor_type_bitmap_dut != 0b00000010)&(occupancy_sensor_type_bitmap_dut != 0b00000100))):
                pir_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIROccupiedToUnoccupiedDelay)

                asserts.assert_less_equal(pir_otou_delay_dut, 0xFFFE, "PIROccupiedToUnoccupiedDelay is not in valid range")
                asserts.assert_greater_equal(pir_otou_delay_dut, 0, "PIROccupiedToUnoccupiedDelay is not in valid range")
            else:
                logging.info("PIROccupiedToUnoccupiedDelay conformance failed. Test step skipped")
        else:
            logging.info("PIROccupiedToUnoccupiedDelay not supported. Test step skipped")

        self.step(8)
        if attributes.PIRUnoccupiedToOccupiedDelay.attribute_id in attribute_list: 
            if attributes.PIRUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
            
                pir_utoo_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIRUnoccupiedToOccupiedDelay)
                asserts.assert_less_equal(pir_utoo_delay_dut, 0xFFFE, "PIRUnoccupiedToOccupiedDelay is not in valid range")
                asserts.assert_greater_equal(pir_utoo_delay_dut, 0, "PIRUnoccupiedToOccupiedDelay is not in valid range")
            else:
                logging.info("PIRUnoccupiedToOccupiedDelay conformance failed. Test step skipped")
        else:
            logging.info("PIRUnoccupiedToOccupiedDelay not supported. Test step skipped")

        self.step(9)
        if attributes.PIRUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
            if attributes.PIRUnoccupiedToOccupiedDelay.attribute_id in attribute_list:                
                pir_utoo_threshold_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIRUnoccupiedToOccupiedThreshold)
    
                asserts.assert_less_equal(pir_utoo_threshold_dut, 0xFE, "PIRUnoccupiedToOccupiedThreshold is not in valid range")
                asserts.assert_greater_equal(pir_utoo_threshold_dut, 0, "PIRUnoccupiedToOccupiedThreshold is not in valid range")
            else:
                logging.info("PIRUnoccupiedToOccupiedThreshold conformance failed. Test step skipped")
        else:
            logging.info("PIRUnoccupiedToOccupiedThreshold not supported. Test step skipped")

        self.step(10)
        if attributes.UltrasonicOccupiedToUnoccupiedDelay.attribute_id in attribute_list:
            if occupancy_sensor_type_bitmap_dut == 0b00000010:
                ultrasonic_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicOccupiedToUnoccupiedDelay)
    
                asserts.assert_less_equal(ultrasonic_otou_delay_dut, 0xFFFE, "UltrasonicOccupiedToUnoccupiedDelay is not in valid range")
                asserts.assert_greater_equal(ultrasonic_otou_delay_dut, 0, "UltrasonicOccupiedToUnoccupiedDelay is not in valid range")
            else:
                logging.info("UltrasonicOccupiedToUnoccupiedDelay conformance failed. Test step skipped")
        else:
            logging.info("UltrasonicOccupiedToUnoccupiedDelay not supported. Test step skipped")

        self.step(11)
        if attributes.UltrasonicUnoccupiedToOccupiedDelay.attribute_id in attribute_list: 
            if attributes.UltrasonicUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
                ultrasonic_utoo_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicUnoccupiedToOccupiedDelay)
                
                asserts.assert_less_equal(ultrasonic_utoo_delay_dut, 0xFFFE, "UltrasonicUnoccupiedToOccupiedDelay is not in valid range")
                asserts.assert_greater_equal(ultrasonic_utoo_delay_dut, 0, "UltrasonicUnoccupiedToOccupiedDelay is not in valid range")
            else:
                logging.info("UltrasonicUnoccupiedToOccupiedDelay conformance failed. Test step skipped")
        else:
            logging.info("UltrasonicUnoccupiedToOccupiedDelay not supported. Test step skipped")

        self.step(12)
        if attributes.UltrasonicUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
            if attributes.UltrasonicUnoccupiedToOccupiedDelay.attribute_id in attribute_list:
                ultrasonic_utoo_threshold_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicUnoccupiedToOccupiedThreshold)

                asserts.assert_less_equal(ultrasonic_utoo_threshold_dut, 0xFE, "UltrasonicUnoccupiedToOccupiedThreshold is not in valid range")
                asserts.assert_greater_equal(ultrasonic_utoo_threshold_dut, 0, "UltrasonicUnoccupiedToOccupiedThreshold is not in valid range")
            else:
                logging.info("UltrasonicUnoccupiedToOccupiedThreshold conformance failed. Test step skipped")
        else:
            logging.info("UltrasonicUnoccupiedToOccupiedThreshold not supported. Test step skipped")

        self.step(13)
        if attributes.PhysicalContactOccupiedToUnoccupiedDelay.attribute_id in attribute_list:
            if occupancy_sensor_type_bitmap_dut == 0b00000100:
                phycontact_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactOccupiedToUnoccupiedDelay)
                
                asserts.assert_less_equal(phycontact_otou_delay_dut, 0xFFFE, "PhysicalContactOccupiedToUnoccupiedDelay is not in valid range")
                asserts.assert_greater_equal(phycontact_otou_delay_dut, 0, "PhysicalContactOccupiedToUnoccupiedDelay is not in valid range")
            else:
                logging.info("PhysicalContactOccupiedToUnoccupiedDelay conformance failed. Test step skipped")
        else:
            logging.info("PhysicalContactOccupiedToUnoccupiedDelay not supported. Test step skipped")

        self.step(14)
        if attributes.PhysicalContactUnoccupiedToOccupiedDelay.attribute_id in attribute_list:
            if attributes.PhysicalContactUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
                phycontact_utoo_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactUnoccupiedToOccupiedDelay)
                asserts.assert_less_equal(phycontact_utoo_delay_dut, 0xFFFE, "PhysicalContactUnoccupiedToOccupiedDelay is not in valid range")
                asserts.assert_greater_equal(phycontact_utoo_delay_dut, 0, "PhysicalContactUnoccupiedToOccupiedDelay is not in valid range")
            else:
                logging.info("PhysicalContactUnoccupiedToOccupiedDelay conformance failed. Test step skipped")
        else:
            logging.info("PhysicalContactUnoccupiedToOccupiedDelay not supported. Test step skipped")

        self.step(15)
        if attributes.PhysicalContactUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
            if attributes.PhysicalContactUnoccupiedToOccupiedDelay.attribute_id in attribute_list:
                phycontact_utoo_threshold_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactUnoccupiedToOccupiedThreshold)
    
                asserts.assert_less_equal(phycontact_utoo_threshold_dut, 0xFE, "PhysicalContactUnoccupiedToOccupiedThreshold is not in valid range")
                asserts.assert_greater_equal(phycontact_utoo_threshold_dut, 0, "PhysicalContactUnoccupiedToOccupiedThreshold is not in valid range")
            else:
                logging.info("PhysicalContactUnoccupiedToOccupiedThreshold conformance failed. Test step skipped")
        else:
            logging.info("PhysicalContactUnoccupiedToOccupiedThreshold not supported. Test step skipped")

if __name__ == "__main__":
    default_matter_test_main()
