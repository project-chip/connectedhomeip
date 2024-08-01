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

from dataclasses import dataclass

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


@dataclass
class FeatureMapping:
    """Maps feature flags to a specific bitmap and sensor type"""
    feature_map: int
    sensor_type: Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum


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

        self.step(1)
        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(2)
        # OccupancySensorType will be determined by FeatureMap matching table at 2.7.6.2.
        asserts.assert_in(attributes.OccupancySensorType.attribute_id, attribute_list,
                          "OccupancySensorType attribute is a mandatory attribute.")
        occupancy_sensor_type_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorType)

        # For validation purposes, 2.7.6.2 table describes what feature flags map to what type of sensors
        TypeEnum = Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum

        Y = True
        N = False
        # Map is PIR, US, PHY => expected sensor type
        # odd Y/N mapping to make the table align nicely
        mappings = {
            (N, N, N): TypeEnum.kPir,
            (Y, N, N): TypeEnum.kPir,
            (N, Y, N): TypeEnum.kUltrasonic,
            (Y, Y, N): TypeEnum.kUltrasonic,
            (N, N, Y): TypeEnum.kPhysicalContact,
            (Y, N, Y): TypeEnum.kPir,
            (N, Y, Y): TypeEnum.kUltrasonic,
            (Y, Y, Y): TypeEnum.kPIRAndUltrasonic,
        }
        expected = mappings.get(
            (
                (feature_map & Clusters.OccupancySensing.Bitmaps.Feature.kPassiveInfrared) != 0,
                (feature_map & Clusters.OccupancySensing.Bitmaps.Feature.kUltrasonic) != 0,
                (feature_map & Clusters.OccupancySensing.Bitmaps.Feature.kPhysicalContact) != 0
            ))

        asserts.assert_equal(
            occupancy_sensor_type_dut,
            expected,
            f"Sensort Type should be f{expected}"
        )

        self.step(3)
        # OccupancySensorTypeBitmap will be determined by FeatureMap matching table at 2.7.6.2.
        asserts.assert_in(attributes.OccupancySensorTypeBitmap.attribute_id, attribute_list,
                          "OccupancySensorTypeBitmap attribute is a mandatory attribute.")

        occupancy_sensor_type_bitmap_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorTypeBitmap)

        # Feature map must match the sensor type bitmap
        must_match_bits = [
            (Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPir,
             Clusters.OccupancySensing.Bitmaps.Feature.kPassiveInfrared, "PIR"),
            (Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kUltrasonic,
                Clusters.OccupancySensing.Bitmaps.Feature.kUltrasonic, "Ultrasonic"),
            (Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPhysicalContact,
                Clusters.OccupancySensing.Bitmaps.Feature.kPhysicalContact, "Physical contact"),
        ]

        for sensor_bit, feature_bit, name in must_match_bits:
            asserts.assert_equal(
                (occupancy_sensor_type_bitmap_dut & sensor_bit) != 0,
                (feature_map & feature_bit) != 0,
                f"Feature bit and sensor bitmap must be equal for {name}"
            )


if __name__ == "__main__":
    default_matter_test_main()
