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

from chip import ChipDeviceCtrl
import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_OCC_2_3(MatterBaseTest):
    async def read_occ_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.OccupancySensing
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_OCC_2_3(self) -> str:
        return "[TC-OCC-2.3] HoldTime Backward Compatibility Test with server as DUT"

    def steps_TC_OCC_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "DUT supports HoldTime attribute. If DUT doesn’t support it, then stop and exit this test case."),
            TestStep(3, "Based on the feature flag value table, read OccupancySensorType attribute from DUT"),
            TestStep(4, "If TH reads 0 - PIR, TH reads PIROccupiedToUnoccupiedDelay attribute and its value should be same as HoldTime"),
            TestStep(5, "If TH reads 1 - Ultrasonic, TH reads UltrasonicOccupiedToUnoccupiedDelay attribute and its value should be same as HoldTime"),
            TestStep(6, "If TH reads 2 - PHY, TH reads PhysicalContactOccupiedToUnoccupiedDelay attribute and its value should be same as HoldTime")
        ]
        return steps

    def pics_TC_OCC_2_3(self) -> list[str]:
        pics = [
            "OCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_OCC_2_3(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.OccupancySensing.Attributes
        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
        
        self.step(2)
        if attributes.HoldTime.attribute_id in attribute_list:
            occupancy_hold_time_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.HoldTime)

        else:
            logging.info("No HoldTime attribute supports. Terminate this test case")
            
        self.step(3)
        if attributes.OccupancySensorType.attribute_id in attribute_list:
            occupancy_sensor_type_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorType)

            asserts.assert_less_equal(occupancy_sensor_type_dut, 3, "OccupancySensorType attribute is out of range")
            asserts.assert_greater_equal(occupancy_sensor_type_dut, 0, "OccupancySensorType attribute is out of range")
        else:
            logging.info("OccupancySensorType attribute doesn't exist. Test step skipped")

        self.step(4)
        if occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir:
            occupancy_pir_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIROccupiedToUnoccupiedDelay)

            asserts.assert_equal(occupancy_pir_otou_delay_dut, occupancy_hold_time_dut, "HoldTime attribute value is not equal to PIROccupiedToUnoccupiedDelay")
                
        elif occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kUltrasonic:
            occupancy_us_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicOccupiedToUnoccupiedDelay)
            
            asserts.assert_equal(occupancy_us_otou_delay_dut, occupancy_hold_time_dut, "HoldTime attribute value is not equal to UltrasonicOccupiedToUnoccupiedDelay")

        elif occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPIRAndUltrasonic:
            occupancy_pirus_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIROccupiedToUnoccupiedDelay)

            asserts.assert_equal(occupancy_pirus_otou_delay_dut, occupancy_hold_time_dut, "HoldTime attribute value is not equal to PIROccupiedToUnoccupiedDelay")                

        elif occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPhysicalContact:
            occupancy_phy_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactOccupiedToUnoccupiedDelay)
        
            asserts.assert_equal(occupancy_phy_otou_delay_dut, occupancy_hold_time_dut, "HoldTime attribute value is not equal to PhysicalContactOccupiedToUnoccupiedDelay") 
        else:
            logging.info("OccupancySensorType attribute value is out of range")
