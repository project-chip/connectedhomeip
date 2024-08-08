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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
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

        if occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir:
            self.step(4)
            occupancy_pir_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIROccupiedToUnoccupiedDelay)

            asserts.assert_equal(occupancy_pir_otou_delay_dut, occupancy_hold_time_dut,
                                 "HoldTime attribute value is not equal to PIROccupiedToUnoccupiedDelay")
            self.skip_step(5)
            self.skip_step(6)

        elif occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kUltrasonic:
            self.step(4)
            occupancy_pir_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIROccupiedToUnoccupiedDelay)

            asserts.assert_equal(occupancy_pir_otou_delay_dut, occupancy_hold_time_dut,
                                 "HoldTime attribute value is not equal to PIROccupiedToUnoccupiedDelay")
            self.step(5)
            occupancy_us_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicOccupiedToUnoccupiedDelay)

            asserts.assert_equal(occupancy_us_otou_delay_dut, occupancy_hold_time_dut,
                                 "HoldTime attribute value is not equal to UltrasonicOccupiedToUnoccupiedDelay")
            self.skip_step(6)

        elif occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPIRAndUltrasonic:
            occupancy_pirus_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIROccupiedToUnoccupiedDelay)

            asserts.assert_equal(occupancy_pirus_otou_delay_dut, occupancy_hold_time_dut,
                                 "HoldTime attribute value is not equal to PIROccupiedToUnoccupiedDelay")

        elif occupancy_sensor_type_dut == Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPhysicalContact:
            self.skip_step(4)
            self.skip_step(5)
            self.step(6)
            occupancy_phy_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactOccupiedToUnoccupiedDelay)

            asserts.assert_equal(occupancy_phy_otou_delay_dut, occupancy_hold_time_dut,
                                 "HoldTime attribute value is not equal to PhysicalContactOccupiedToUnoccupiedDelay")
        else:
            logging.info("OccupancySensorType attribute value is out of range")


if __name__ == "__main__":
    default_matter_test_main()
