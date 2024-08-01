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
            TestStep(2, "Read OccupancySensorType attribute selection based on FeatureMap Bitmap."),
            TestStep(3, "Read OccupancySensorTypeBitmap attribute selection based on FeatureMap Bitmap.")
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

        attributes = Clusters.OccupancySensing.Attributes
        feature_map = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        is_pir_feature_supported = feature_map & Clusters.OccupancySensing.Bitmaps.Feature.kPassiveInfrared
        is_us_feature_supported = feature_map & Clusters.OccupancySensing.Bitmaps.Feature.kUltrasonic
        is_phy_feature_supported = feature_map & Clusters.OccupancySensing.Bitmaps.Feature.kPhysicalContact

        self.step(1)
        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(2)
        # OccupancySensorType will be determined by FeatureMap matching table at 2.7.6.2.
        if attributes.OccupancySensorType.attribute_id in attribute_list:
            occupancy_sensor_type_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorType)

            # No Bitmap support from PIR, US, Physical Contact, then OccupancySensorType should be PIR
            if (is_pir_feature_supported != 1) & (is_us_feature_supported != 1) & (is_phy_feature_supported != 1):
                asserts.assert_equal(occupancy_sensor_type_dut,
                                     Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir, "OccupancySensorType is not PIR")
            # Bitmap supports PIR, then OccupancySensorType should be PIR
            elif (is_pir_feature_supported == 1) & (is_us_feature_supported != 1) & (is_phy_feature_supported != 1):
                asserts.assert_equal(occupancy_sensor_type_dut,
                                     Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir, "OccupancySensorType is not PIR")
            # Bitmap supports US, then OccupancySensorType should be US
            elif (is_pir_feature_supported != 1) & (is_us_feature_supported == 1) & (is_phy_feature_supported != 1):
                asserts.assert_equal(
                    occupancy_sensor_type_dut, Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kUltrasonic, "OccupancySensorType is not Ultrasonic")

            # Bitmap supports PIR and US, then OccupancySensorType should be PIRAndUltrasonic
            elif (is_pir_feature_supported == 1) & (is_us_feature_supported == 1) & (is_phy_feature_supported != 1):
                asserts.assert_equal(occupancy_sensor_type_dut, Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPIRAndUltrasonic,
                                     "OccupancySensorType is not PIRAndUltrasonic")
            # Bitmap supports Physical Contact, then OccupancySensorType should be Physical Contact
            elif (is_pir_feature_supported != 1) & (is_us_feature_supported != 1) & (is_phy_feature_supported == 1):
                asserts.assert_equal(occupancy_sensor_type_dut, Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPhysicalContact,
                                     "OccupancySensorType is not PhysicalContact")
            # Bitmap supports Physical Contact and PIR, then OccupancySensorType should be PIR
            elif (is_pir_feature_supported == 1) & (is_us_feature_supported != 1) & (is_phy_feature_supported == 1):
                asserts.assert_equal(occupancy_sensor_type_dut,
                                     Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir, "OccupancySensorType is not PIR")
            # Bitmap supports US and Physical Contact, then OccupancySensorType should be US
            elif (is_pir_feature_supported != 1) & (is_us_feature_supported == 1) & (is_phy_feature_supported == 1):
                asserts.assert_equal(
                    occupancy_sensor_type_dut, Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kUltrasonic, "OccupancySensorType is not Ultrasonic")
            # Bitmap supports PIR, US and Physical Contact, then OccupancySensorType should be PIRAndUltrasonic
            elif (is_pir_feature_supported == 1) & (is_us_feature_supported == 1) & (is_phy_feature_supported == 1):
                asserts.assert_equal(occupancy_sensor_type_dut, Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPIRAndUltrasonic,
                                     "OccupancySensorType is not PIRAndUltrasonic")

        else:
            logging.info("OccupancySensorType mandatory attribute is not supported. Test step skipped")
            asserts.fail("Missing mandatory attribute OccupancySensorType")

        self.step(3)
        # OccupancySensorTypeBitmap will be determined by FeatureMap matching table at 2.7.6.2.
        if attributes.OccupancySensorTypeBitmap.attribute_id in attribute_list:
            occupancy_sensor_type_bitmap_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorTypeBitmap)

            # No Bitmap support from PIR, US, Physical Contact, then OccupancySensorTypeBitmap should be PIR
            if (is_pir_feature_supported != 1) & (is_us_feature_supported != 1) & (is_phy_feature_supported != 1):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut,
                                     Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPir, "OccupancySensorType is not PIR")
            # Bitmap supports PIR, then OccupancySensorTypeBitmap should be PIR
            elif (is_pir_feature_supported == 1) & (is_us_feature_supported != 1) & (is_phy_feature_supported != 1):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut,
                                     Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPir, "OccupancySensorType is not PIR")
            # Bitmap supports US, then OccupancySensorTypeBitmap should be US
            elif (is_pir_feature_supported != 1) & (is_us_feature_supported == 1) & (is_phy_feature_supported != 1):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut,
                                     Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kUltrasonic, "OccupancySensorType is not Ultrasonic")
            # Bitmap supports PIR and US, then OccupancySensorTypeBitmap should be PIRAndUltrasonic
            elif (is_pir_feature_supported == 1) & (is_us_feature_supported == 1) & (is_phy_feature_supported != 1):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut, 0b00000011, "OccupancySensorType is not PIR+Ultrasonic")
            # Bitmap supports Physical Contact, then OccupancySensorTypeBitmap should be Physical Contact
            elif (is_pir_feature_supported != 1) & (is_us_feature_supported != 1) & (is_phy_feature_supported == 1):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut,
                                     Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPhysicalContact, "OccupancySensorType is not PhysicalContact")
            # Bitmap supports Physical Contact and PIR, then OccupancySensorTypeBitmap should be PIR + PhysicalContact
            elif (is_pir_feature_supported == 1) & (is_us_feature_supported != 1) & (is_phy_feature_supported == 1):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut, 0b00000101,
                                     "OccupancySensorType is not PIR + PhysicalContact")
            # Bitmap supports US and Physical Contact, then OccupancySensorTypeBitmap should be US + Physical contact
            elif (is_pir_feature_supported != 1) & (is_us_feature_supported == 1) & (is_phy_feature_supported == 1):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut, 0b00000110,
                                     "OccupancySensorType is not PhysicalContact + Ultrasonic")
            # Bitmap supports PIR, US and Physical Contact, then OccupancySensorTypeBitmap should be PIRAndUltrasonicAndPhy
            elif (is_pir_feature_supported == 1) & (is_us_feature_supported == 1) & (is_phy_feature_supported == 1):
                asserts.assert_equal(occupancy_sensor_type_bitmap_dut, 0b00000111,
                                     "OccupancySensorType is not PIR+Ultrasonic+Ultrasonic")

        else:
            logging.info("OccupancySensorTypeBitmap mandatory attribute is not supported. Test step skipped")
            asserts.fail("Missing mandatory attribute OccupancySensorTypeBitmap")


if __name__ == "__main__":
    default_matter_test_main()
