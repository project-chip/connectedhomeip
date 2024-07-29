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


class TC_OCC_2_2(MatterBaseTest):
    async def read_occ_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.OccupancySensing
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_OCC_2_2(self) -> str:
        return "[TC-OCC-2.2] OccupancySensorTypeBitmap and OccupancySensorType interdependency with server as DUT"

    def steps_TC_OCC_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read attribute list to determine supported attributes"),
            TestStep(3, "Read OccupancySensorType attribute."),
            TestStep(4, "Read OccupancySensorTypeBitmap attribute.")
        ]
        return steps

    def pics_TC_OCC_2_2(self) -> list[str]:
        pics = [
            "OCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_OCC_2_2(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.OccupancySensing.Attributes

        self.step(2)
        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
        
        self.step(3)
        if attributes.OccupancySensorType.attribute_id in attribute_list:
            occupancy_sensor_type_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorType)

            if occupancy_sensor_type_dut != NullValue:
                asserts.assert_less_equal(occupancy_sensor_type_dut, 3, "OccupancySensorType is not in valid range")
                asserts.assert_greater_equal(occupancy_sensor_type_dut, 0, "OccupancySensorType is not in valid range")
        else:
            logging.info("Test step skipped")

        self.step(4)
        if attributes.OccupancySensorTypeBitmap.attribute_id in attribute_list:
            occupancy_sensor_type_bitmap_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorTypeBitmap)

            if occupancy_sensor_type_bitmap_dut != NullValue:
                if occupancy_sensor_type_bitmap_dut == 1:
                    asserts.assert_less_equal(occupancy_sensor_type_bitmap_dut, 0b00000111, "OccupancySensorTypeBitmap attribute is not in valid range")
        else:
            logging.info("Test step skipped")
