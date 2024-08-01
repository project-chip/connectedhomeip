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
        is_pir_feature_supported = (feature_map & Clusters.OccupancySensing.Bitmaps.Feature.kPassiveInfrared) != 0
        is_us_feature_supported = (feature_map & Clusters.OccupancySensing.Bitmaps.Feature.kUltrasonic) != 0
        is_phy_feature_supported = (feature_map & Clusters.OccupancySensing.Bitmaps.Feature.kPhysicalContact) != 0

        self.step(1)
        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(2)
        # OccupancySensorType will be determined by FeatureMap matching table at 2.7.6.2.
        asserts.assert_in(attributes.OccupancySensorType.attribute_id, attribute_list,
                          "OccupancySensorType attribute is a mandatory attribute.")
        occupancy_sensor_type_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorType)

        # if OccupancySensorType is PIR, check bitmaps
        assert .assert_equal(occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir,
                             not is_pir_feature_supported & & not is_us_feature_supported & & not is_phy_feature_supported, "PIR sensor type is wrong")
        assert .assert_equal(occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir,
                             is_pir_feature_supported & & not is_us_feature_supported & & not is_phy_feature_supported, "PIR sensor type is wrong")
        assert .assert_equal(occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir,
                             is_pir_feature_supported & & not is_us_feature_supported & & is_phy_feature_supported, "PIR sensor type is wrong")

        # if OccupancySensorType is Ultrasonic, check bitmaps
        assert .assert_equal(occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kUltrasonic,
                             not is_pir_feature_supported & & is_us_feature_supported & & not is_phy_feature_supported, "Ultrasonic sensor type is wrong")
        assert .assert_equal(occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kUltrasonic,
                             not is_pir_feature_supported & & is_us_feature_supported & & is_phy_feature_supported, "Ultrasonic sensor type is wrong")

        # if OccupancySensorType is PIRAndUltrasonic, check bitmaps
        assert .assert_equal(occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPIRAndUltrasonic,
                             is_pir_feature_supported & & is_us_feature_supported & & not is_phy_feature_supported, "PIRAndUltrasonic sensor type is wrong")
        assert .assert_equal(occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPIRAndUltrasonic,
                             is_pir_feature_supported & & is_us_feature_supported & & is_phy_feature_supported, "PIRAndUltrasonic sensor type is wrong")

        # if OccupancySensorType is PhysicalContact, check bitmaps
        assert .assert_equal(occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPhysicalContact,
                             not is_pir_feature_supported & & not is_us_feature_supported & & is_phy_feature_supported, "PHY Contact sensor type is wrong")

        self.step(3)
        # OccupancySensorTypeBitmap will be determined by FeatureMap matching table at 2.7.6.2.
        asserts.assert_in(attributes.OccupancySensorTypeBitmap.attribute_id, attribute_list,
                          "OccupancySensorTypeBitmap attribute is a mandatory attribute.")

        occupancy_sensor_type_bitmap_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorTypeBitmap)

        # if OccupancySensorTypeBitmap is PIR, check bitmaps
        assert .assert_equal(occupancy_sensor_bitmap_dut == Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPir,
                             not is_pir_feature_supported & & not is_us_feature_supported & & not is_phy_feature_supported, "PIR sensor type bitmap is wrong")
        assert .assert_equal(occupancy_sensor_bitmap_dut == Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPir,
                             is_pir_feature_supported & & not is_us_feature_supported & & not is_phy_feature_supported, "PIR sensor type bitmap is wrong")

        # if OccupancySensorTypeBitmap is Ultrasonic, check bitmaps
        assert .assert_equal(occupancy_sensor_bitmap_dut == Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kUltrasonic,
                             not is_pir_feature_supported & & is_us_feature_supported & & not is_phy_feature_supported, "Ultrasonic sensor type bitmap is wrong")

        # if OccupancySensorTypeBitmap is PhysicalContact, check bitmaps
        assert .assert_equal(occupancy_sensor_bitmap_dut == Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPhysicalContact,
                             not is_pir_feature_supported & & not is_us_feature_supported & & is_phy_feature_supported, "Physical contact sensor type bitmap is wrong")

        # if OccupancySensorTypeBitmap is PIR+Ultrasonic, check bitmaps
        assert .assert_equal(occupancy_sensor_type_bitmap_dut == 0b00000011,
                             not is_pir_feature_supported & & is_us_feature_supported & & not is_phy_feature_supported, "PIR+UL sensor type bitmap is wrong")

        # if OccupancySensorTypeBitmap is PIR+PhysicalContact, check bitmaps
        assert .assert_equal(occupancy_sensor_type_bitmap_dut == 0b00000101,
                             is_pir_feature_supported & & not is_us_feature_supported & & is_phy_feature_supported, "PIR+Phy contact sensor type bitmap is wrong")

        # if OccupancySensorTypeBitmap is US+PhysicalContact, check bitmaps
        assert .assert_equal(occupancy_sensor_type_bitmap_dut == 0b00000110,
                             not is_pir_feature_supported & & is_us_feature_supported & & is_phy_feature_supported, "US+Phy contact sensor type bitmap is wrong")

        # if OccupancySensorTypeBitmap is PIR+US+PhysicalContact, check bitmaps
        assert .assert_equal(occupancy_sensor_type_bitmap_dut == 0b00000111,
                             is_pir_feature_supported & & is_us_feature_supported & & is_phy_feature_supported, "PIR+US+Phy contact sensor type bitmap is wrong")


if __name__ == "__main__":
    default_matter_test_main()
