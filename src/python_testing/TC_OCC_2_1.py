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
#  There are CI issues to be followed up for the test cases below that implements manually controlling sensor device for
#  the occupancy state ON/OFF change.
#  [TC-OCC-3.1] test procedure step 4
#  [TC-OCC-3.2] test precedure step 3a, 3c

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --endpoint 1
# === END CI TEST ARGUMENTS ===

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
            TestStep(2, "Read Occupancy attribute."),
            TestStep(3, "Read OccupancySensorType attribute."),
            TestStep(4, "Read OccupancySensorTypeBitmap attribute."),
            TestStep(5, "Read HoldTimeLimits attribute, if supported"),
            TestStep(6, "Read HoldTime attribute, if supported"),
            TestStep(7, "Read PIROccupiedToUnoccupiedDelay attribute, if supported"),
            TestStep(8, "Read PIRUnoccupiedToOccupiedDelay attribute, if supported"),
            TestStep(9, "Read PIRUnoccupiedToOccupiedThreshold attribute, if supported"),
            TestStep(10, "Read UltrasonicOccupiedToUnoccupiedDelay attribute, if supported"),
            TestStep(11, "Read UltrasonicUnoccupiedToOccupiedDelay attribute, if supported"),
            TestStep(12, "Read UltrasonicUnoccupiedToOccupiedThreshold attribute, if supported"),
            TestStep(13, "Read PhysicalContactOccupiedToUnoccupiedDelay attribute, if supported"),
            TestStep(14, "Read PhysicalContactUnoccupiedToOccupiedDelay attribute, if supported"),
            TestStep(15, "Read PhysicalContactUnoccupiedToOccupiedThreshold attribute, if supported")
        ]
        return steps

    def pics_TC_OCC_2_1(self) -> list[str]:
        pics = [
            "OCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_OCC_2_1(self):
        endpoint = self.matter_test_config.endpoint
        cluster = Clusters.Objects.OccupancySensing
        attributes = cluster.Attributes

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)

        feature_map = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        has_feature_pir = (feature_map & cluster.Bitmaps.Feature.kPassiveInfrared) != 0
        has_feature_ultrasonic = (feature_map & cluster.Bitmaps.Feature.kUltrasonic) != 0
        has_feature_contact = (feature_map & cluster.Bitmaps.Feature.kPhysicalContact) != 0

        logging.info(
            f"Feature map: 0x{feature_map:x}. PIR: {has_feature_pir}, US:{has_feature_ultrasonic}, PHY:{has_feature_contact}")

        attribute_list = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        asserts.assert_in(attributes.Occupancy.attribute_id, attribute_list, "Occupancy attribute is mandatory")
        occupancy_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.Occupancy)
        asserts.assert_less_equal(occupancy_dut, 0b00000001, "Occupancy attribute is not in valid range")

        self.step(3)
        asserts.assert_in(attributes.OccupancySensorType.attribute_id, attribute_list,
                          "OccupancySensorType attribute is a mandatory attribute.")

        occupancy_sensor_type_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorType)
        asserts.assert_less(occupancy_sensor_type_dut, Clusters.Objects.OccupancySensing.Enums.OccupancySensorTypeEnum.kUnknownEnumValue,
                            "OccupancySensorType is not in valid range")
        asserts.assert_in(occupancy_sensor_type_dut, {Clusters.Objects.OccupancySensing.Enums.OccupancySensorTypeEnum.kPir,
                                                      Clusters.Objects.OccupancySensing.Enums.OccupancySensorTypeEnum.kUltrasonic,
                                                      Clusters.Objects.OccupancySensing.Enums.OccupancySensorTypeEnum.kPIRAndUltrasonic,
                                                      Clusters.Objects.OccupancySensing.Enums.OccupancySensorTypeEnum.kPhysicalContact}, "OccupancySensorType is not in valid range")
        self.step(4)
        asserts.assert_in(attributes.OccupancySensorTypeBitmap.attribute_id, attribute_list,
                          "OccupancySensorTypeBitmap attribute is a mandatory attribute.")

        occupancy_sensor_type_bitmap_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.OccupancySensorTypeBitmap)
        asserts.assert_less_equal(occupancy_sensor_type_bitmap_dut, 0b00000111,
                                  "OccupancySensorTypeBitmap attribute is not in valid range")

        self.step(5)
        if attributes.HoldTimeLimits.attribute_id in attribute_list:
            asserts.assert_in(attributes.HoldTime.attribute_id, attribute_list, "HoldTime attribute conformance failed.")
            hold_time_limits_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.HoldTimeLimits)
            asserts.assert_less_equal(hold_time_limits_dut.holdTimeMin, hold_time_limits_dut.holdTimeMax,
                                      "HoldTimeMin is not in valid range")
            asserts.assert_greater_equal(hold_time_limits_dut.holdTimeMin, 1, "HoldTimeMin is less than 1.")
            asserts.assert_greater_equal(hold_time_limits_dut.holdTimeMax, 10, "HoldTimeMax is less than 10.")
            asserts.assert_less_equal(hold_time_limits_dut.holdTimeMin, 0xFFFE, "HoldTimeMin is not in valid range.")
            asserts.assert_less_equal(hold_time_limits_dut.holdTimeMax, 0xFFFE, "HoldTimeMax is not in valid range.")
            asserts.assert_greater_equal(hold_time_limits_dut.holdTimeMax, hold_time_limits_dut.holdTimeMin,
                                         "HoldTimeMax is less than HoldTimeMin.")
            asserts.assert_less_equal(hold_time_limits_dut.holdTimeDefault,
                                      hold_time_limits_dut.holdTimeMax, "HoldTimeDefault is greater than HoldTimeMax.")
            asserts.assert_greater_equal(hold_time_limits_dut.holdTimeDefault,
                                         hold_time_limits_dut.holdTimeMin, "HoldTimeDefault is less than HoldTimeMin.")
        else:
            logging.info("HoldTimeLimits not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(6)
        if attributes.HoldTime.attribute_id in attribute_list:
            asserts.assert_in(attributes.HoldTimeLimits.attribute_id, attribute_list,
                              "HoldTimeLimits attribute conformance failed.")
            hold_time_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.HoldTime)
            hold_time_limits_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.HoldTimeLimits)

            asserts.assert_less_equal(hold_time_dut, hold_time_limits_dut.holdTimeMax,
                                      "HoldTime attribute is greater than HoldTimeMax.")
            asserts.assert_greater_equal(hold_time_dut, hold_time_limits_dut.holdTimeMin,
                                         "HoldTime attribute is less than HoldTimeMin.")
        else:
            logging.info("HoldTime not supported. The rest of legacy attribute test can be skipped")
            self.skip_all_remaining_steps(7)
            return

        self.step(7)
        if attributes.PIROccupiedToUnoccupiedDelay.attribute_id in attribute_list:
            has_feature_pir = (occupancy_sensor_type_bitmap_dut &
                               Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPir) != 0
            has_feature_ultrasonic = (occupancy_sensor_type_bitmap_dut &
                                      Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kUltrasonic) != 0
            has_feature_contact = (occupancy_sensor_type_bitmap_dut &
                                   Clusters.OccupancySensing.Bitmaps.OccupancySensorTypeBitmap.kPhysicalContact) != 0
            if has_feature_pir or (not has_feature_pir and not has_feature_ultrasonic and not has_feature_contact):
                pir_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIROccupiedToUnoccupiedDelay)
                asserts.assert_less_equal(pir_otou_delay_dut, 0xFFFE, "PIROccupiedToUnoccupiedDelay is not in valid range")
                asserts.assert_greater_equal(pir_otou_delay_dut, 0, "PIROccupiedToUnoccupiedDelay is not in valid range")
            else:
                logging.info("PIROccupiedToUnoccupiedDelay conformance failed")
                asserts.fail(
                    f"PIROccupiedToUnoccupiedDelay conformance is incorrect: {has_feature_pir}, {has_feature_ultrasonic}, {has_feature_contact}")
        else:
            logging.info("PIROccupiedToUnoccupiedDelay not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(8)
        if attributes.PIRUnoccupiedToOccupiedDelay.attribute_id in attribute_list:
            has_delay = attributes.PIRUnoccupiedToOccupiedDelay.attribute_id in attribute_list
            has_threshold = attributes.PIRUnoccupiedToOccupiedThreshold.attribute_id in attribute_list
            asserts.assert_equal(has_delay, has_threshold, "PIRUnoccupiedToOccupiedDelay conformance failure")
            pir_utoo_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIRUnoccupiedToOccupiedDelay)
            asserts.assert_less_equal(pir_utoo_delay_dut, 0xFFFE, "PIRUnoccupiedToOccupiedDelay is not in valid range")
            asserts.assert_greater_equal(pir_utoo_delay_dut, 0, "PIRUnoccupiedToOccupiedDelay is not in valid range")
        else:
            logging.info("PIRUnoccupiedToOccupiedDelay not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(9)
        if attributes.PIRUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
            has_delay = attributes.PIRUnoccupiedToOccupiedDelay.attribute_id in attribute_list
            has_threshold = attributes.PIRUnoccupiedToOccupiedThreshold.attribute_id in attribute_list
            asserts.assert_equal(has_delay, has_threshold, "PIRUnoccupiedToOccupiedThreshold conformance failure")
            pir_utoo_threshold_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PIRUnoccupiedToOccupiedThreshold)
            asserts.assert_less_equal(pir_utoo_threshold_dut, 0xFE, "PIRUnoccupiedToOccupiedThreshold is not in valid range")
            asserts.assert_greater_equal(pir_utoo_threshold_dut, 0, "PIRUnoccupiedToOccupiedThreshold is not in valid range")
        else:
            logging.info("PIRUnoccupiedToOccupiedThreshold not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(10)
        if attributes.UltrasonicOccupiedToUnoccupiedDelay.attribute_id in attribute_list:
            has_feature_ultrasonic = (occupancy_sensor_type_bitmap_dut &
                                      Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kUltrasonic) != 0
            has_ultrasonic_delay = attributes.UltrasonicOccupiedToUnoccupiedDelay.attribute_id in attribute_list
            asserts.assert_equal(has_feature_ultrasonic, has_ultrasonic_delay, "Bad conformance on Ultrasonic bitmap")

            ultrasonic_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicOccupiedToUnoccupiedDelay)
            asserts.assert_less_equal(ultrasonic_otou_delay_dut, 0xFFFE,
                                      "UltrasonicOccupiedToUnoccupiedDelay is not in valid range")
            asserts.assert_greater_equal(ultrasonic_otou_delay_dut, 0, "UltrasonicOccupiedToUnoccupiedDelay is not in valid range")

        else:
            logging.info("UltrasonicOccupiedToUnoccupiedDelay not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(11)
        if attributes.UltrasonicUnoccupiedToOccupiedDelay.attribute_id in attribute_list:
            has_delay = attributes.UltrasonicUnoccupiedToOccupiedDelay.attribute_id in attribute_list
            has_threshold = attributes.UltrasonicUnoccupiedToOccupiedThreshold.attribute_id in attribute_list
            asserts.assert_equal(has_delay, has_threshold, "UltrasonicUnoccupiedToOccupiedDelay conformance failure")

            ultrasonic_utoo_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicUnoccupiedToOccupiedDelay)
            asserts.assert_less_equal(ultrasonic_utoo_delay_dut, 0xFFFE,
                                      "UltrasonicUnoccupiedToOccupiedDelay is not in valid range")
            asserts.assert_greater_equal(ultrasonic_utoo_delay_dut, 0, "UltrasonicUnoccupiedToOccupiedDelay is not in valid range")
        else:
            logging.info("UltrasonicUnoccupiedToOccupiedDelay not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(12)
        if attributes.UltrasonicUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
            has_delay = attributes.UltrasonicUnoccupiedToOccupiedDelay.attribute_id in attribute_list
            has_threshold = attributes.UltrasonicUnoccupiedToOccupiedThreshold.attribute_id in attribute_list
            asserts.assert_equal(has_delay, has_threshold, "UltrasonicUnoccupiedToOccupiedThreshold conformance failure")

            ultrasonic_utoo_threshold_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.UltrasonicUnoccupiedToOccupiedThreshold)
            asserts.assert_less_equal(ultrasonic_utoo_threshold_dut, 0xFE,
                                      "UltrasonicUnoccupiedToOccupiedThreshold is not in valid range")
            asserts.assert_greater_equal(ultrasonic_utoo_threshold_dut, 0,
                                         "UltrasonicUnoccupiedToOccupiedThreshold is not in valid range")

        else:
            logging.info("UltrasonicUnoccupiedToOccupiedThreshold not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(13)
        if attributes.PhysicalContactOccupiedToUnoccupiedDelay.attribute_id in attribute_list:
            has_phycon_bitmap = (occupancy_sensor_type_bitmap_dut &
                                 Clusters.OccupancySensing.Enums.OccupancySensorTypeEnum.kPhysicalContact) != 0
            has_phycon_delay = attributes.PhysicalContactOccupiedToUnoccupiedDelay.attribute_id in attribute_list
            asserts.assert_equal(has_phycon_bitmap, has_phycon_delay, "Bad conformance on PhysicalContact bitmap")
            phycontact_otou_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactOccupiedToUnoccupiedDelay)
            asserts.assert_less_equal(phycontact_otou_delay_dut, 0xFFFE,
                                      "PhysicalContactOccupiedToUnoccupiedDelay is not in valid range")
            asserts.assert_greater_equal(phycontact_otou_delay_dut, 0,
                                         "PhysicalContactOccupiedToUnoccupiedDelay is not in valid range")

        else:
            logging.info("PhysicalContactOccupiedToUnoccupiedDelay not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(14)
        if attributes.PhysicalContactUnoccupiedToOccupiedDelay.attribute_id in attribute_list:
            has_delay = attributes.PhysicalContactUnoccupiedToOccupiedDelay.attribute_id in attribute_list
            has_threshold = attributes.PhysicalContactUnoccupiedToOccupiedThreshold.attribute_id in attribute_list
            asserts.assert_equal(has_delay, has_threshold, "PhysicalContactUnoccupiedToOccupiedDelay conformance failure")

            phycontact_utoo_delay_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactUnoccupiedToOccupiedDelay)
            asserts.assert_less_equal(phycontact_utoo_delay_dut, 0xFFFE,
                                      "PhysicalContactUnoccupiedToOccupiedDelay is not in valid range")
            asserts.assert_greater_equal(phycontact_utoo_delay_dut, 0,
                                         "PhysicalContactUnoccupiedToOccupiedDelay is not in valid range")

        else:
            logging.info("PhysicalContactUnoccupiedToOccupiedDelay not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(15)
        if attributes.PhysicalContactUnoccupiedToOccupiedThreshold.attribute_id in attribute_list:
            has_delay = attributes.PhysicalContactUnoccupiedToOccupiedDelay.attribute_id in attribute_list
            has_threshold = attributes.PhysicalContactUnoccupiedToOccupiedThreshold.attribute_id in attribute_list
            asserts.assert_equal(has_delay, has_threshold, "PhysicalContactUnoccupiedToOccupiedThreshold conformance failure")

            phycontact_utoo_threshold_dut = await self.read_occ_attribute_expect_success(endpoint=endpoint, attribute=attributes.PhysicalContactUnoccupiedToOccupiedThreshold)
            asserts.assert_less_equal(phycontact_utoo_threshold_dut, 0xFE,
                                      "PhysicalContactUnoccupiedToOccupiedThreshold is not in valid range")
            asserts.assert_greater_equal(phycontact_utoo_threshold_dut, 0,
                                         "PhysicalContactUnoccupiedToOccupiedThreshold is not in valid range")

        else:
            logging.info("PhysicalContactUnoccupiedToOccupiedThreshold not supported. Test step skipped")
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
