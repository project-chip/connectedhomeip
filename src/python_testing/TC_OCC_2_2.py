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


class TC_OCC_2_2(MatterBaseTest):
    async def read_occ_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.OccupancySensing
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_OCC_2_2(self) -> str:
        return "[TC-OCC-2.2] OccupancySensorTypeBitmap and OccupancySensorType interdependency with server as DUT"

    def steps_TC_OCC_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read OccupancySensorType attribute with repect to featureMap."),
            TestStep(3, "Read OccupancySensorTypeBitmap attribute with repect to featureMap.")
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

        feature_map_PIR = Clusters.OccupancySensing.Bitmaps.Feature.kPassiveInfrared
        feature_map_US = Clusters.OccupancySensing.Bitmaps.Feature.kUltrasonic
        feature_map_PHY = Clusters.OccupancySensing.Bitmaps.Feature.kPhysicalContact

        self.step(1)
        attributes = Clusters.OccupancySensing.Attributes
        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(2)
        if attributes.OccupancySensorType.attribute_id in attribute_list:
            occupancy_sensor_type_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorType)
            feature_map = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.attributes.FeatureMap)

            if (feature_map != feature_map_PIR) & (feature_map != feature_map_US) & (feature_map != feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_dut,
                                     Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir, "OccupancySensorType is not PIR")
            elif (feature_map == feature_map_PIR) & (feature_map != feature_map_US) & (feature_map != feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_dut,
                                     Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir, "OccupancySensorType is not PIR")
            elif (feature_map != feature_map_PIR) & (feature_map == feature_map_US) & (feature_map != feature_map_PHY):
                asserts.assert_equal(
                    occupancy_sensor_type_dut, Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kUltrasonci, "OccupancySensorType is not Ultrasonic")
            elif (feature_map == feature_map_PIR) & (feature_map == feature_map_US) & (feature_map != feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_dut, Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPIRAndUltrasonic,
                                     "OccupancySensorType is not PIRAndUltrasonic")
            elif (feature_map != feature_map_PIR) & (feature_map != feature_map_US) & (feature_map == feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_dut, Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPhysicalContact,
                                     "OccupancySensorType is not PhysicalContact")
            elif (feature_map == feature_map_PIR) & (feature_map != feature_map_US) & (feature_map == feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_dut,
                                     Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir, "OccupancySensorType is not PIR")
            elif (feature_map != feature_map_PIR) & (feature_map == feature_map_US) & (feature_map == feature_map_PHY):
                asserts.assert_equal(
                    occupancy_sensor_type_dut, Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kUltrasonic, "OccupancySensorType is not Ultrasonic")
            elif (feature_map == feature_map_PIR) & (feature_map == feature_map_US) & (feature_map == feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_dut, Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPIRAndUltrasonic,
                                     "OccupancySensorType is not PIRAndUltrasonic")

        else:
            logging.info("OccupancySensorType mandatory attribute is not supported. Test step skipped")
            asserts.fail("Missing mandatory attribute OccupancySensorType")

        self.step(3)
        if attributes.OccupancySensorTypeBitmap.attribute_id in attribute_list:
            occupancy_sensor_type_bitmap_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorTypeBitmap)
            feature_map = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.attributes.FeatureMap)

            if (feature_map != feature_map_PIR) & (feature_map != feature_map_US) & (feature_map != feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut,
                                     Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPir, "OccupancySensorType is not PIR")
            elif (feature_map == feature_map_PIR) & (feature_map != feature_map_US) & (feature_map != feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut,
                                     Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPir, "OccupancySensorType is not PIR")
            elif (feature_map != feature_map_PIR) & (feature_map == feature_map_US) & (feature_map != feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut,
                                     Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kUltrasonic, "OccupancySensorType is not Ultrasonic")
            elif (feature_map == feature_map_PIR) & (feature_map == feature_map_US) & (feature_map != feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut, 0b00000011, "OccupancySensorType is not PIR+Ultrasonic")
            elif (feature_map != feature_map_PIR) & (feature_map != feature_map_US) & (feature_map == feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut,
                                     Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPhysicalContact, "OccupancySensorType is not PhysicalContact")
            elif (feature_map == feature_map_PIR) & (feature_map != feature_map_US) & (feature_map == feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut, 0b00000101,
                                     "OccupancySensorType is not PIR + PhysicalContact")
            elif (feature_map != feature_map_PIR) & (feature_map == feature_map_US) & (feature_map == feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut, 0b00000110,
                                     "OccupancySensorType is not PhysicalContact + Ultrasonic")
            elif (feature_map == feature_map_PIR) & (feature_map == feature_map_US) & (feature_map == feature_map_PHY):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut, 0b00000111,
                                     "OccupancySensorType is not PIR+Ultrasonic+Ultrasonic")

        else:
            logging.info("OccupancySensorTypeBitmap mandatory attribute is not supported. Test step skipped")
            asserts.fail("Missing mandatory attribute OccupancySensorTypeBitmap")


if __name__ == "__main__":
    default_matter_test_main()
