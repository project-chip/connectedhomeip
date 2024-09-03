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
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto --endpoint 1 --bool-arg simulate_occupancy:true
# === END CI TEST ARGUMENTS ===
#  There are CI issues to be followed up for the test cases below that implements manually controlling sensor device for
#  the occupancy state ON/OFF change.
#  [TC-OCC-3.1] test procedure step 3, 4
#  [TC-OCC-3.2] test precedure step 3a, 3c

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_OCC_2_3(MatterBaseTest):
    async def read_occ_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.OccupancySensing
        endpoint_id = self.matter_test_config.endpoint
        return await self.read_single_attribute_check_success(endpoint=endpoint_id, cluster=cluster, attribute=attribute)

    def desc_TC_OCC_2_3(self) -> str:
        return "[TC-OCC-2.3] HoldTime Backward Compatibility Test with server as DUT"

    def steps_TC_OCC_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads the FeatureMap attribute on the endpoint for use in later steps."),
            TestStep(3, "TH checks DUT HoldTime attribute support in the AttributeList attribute. If DUT doesn't support HoldTime attribute, skip the rest of this test case."),
            TestStep(4, "TH writes DUT HoldTime attribute with HoldTimeMin and afterwards reads the attribute."),
            TestStep(5, "TH writes DUT HoldTime attribute with HoldTimeMax and afterwards reads the attribute."),
            TestStep("6a", "If DUT FeatureMap has PIR or (!PIR & !US & !PHY), and PIROccupiedToUnoccupiedDelay is supported, then TH writes HoldTimeMin to DUT's HoldTime attribute, otherwise skip 6a, 6b."),
            TestStep("6b", "TH writes DUT's PIROccupiedToUnoccupiedDelay attribute with HoldTimeMax, then TH reads DUT's PIROccupiedToUnoccupiedDelay and HoldTime attributes."),
            TestStep("7a", "If DUT FeatureMap has US, and UltrasonicOccupiedToUnoccupiedDelay is supported, then TH writes HoldTimeMin to DUT's HoldTime attribute, otherwise skip 7a, 7b."),
            TestStep("7b", "TH writes DUT UltrasonicOccupiedToUnoccupiedDelay attribute with HoldTimeMax, then TH reads DUT UltrasonicOccupiedToUnoccupiedDelay and HoldTime attributes."),
            TestStep("8a", "If DUT FeatureMap has PHY, and PhysicalContactOccupiedToUnoccupiedDelay is supported, then TH writes HoldTimeMin to HoldTime attribute, otherwise skip 8a, 8b."),
            TestStep("8b", "TH writes DUT PhysicalContactOccupiedToUnoccupiedDelay attribute with HoldTimeMax, then TH reads DUT PhysicalContactOccupiedToUnoccupiedDelay and HoldTime attributes."),
        ]
        return steps

    def pics_TC_OCC_2_3(self) -> list[str]:
        pics = [
            "OCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_OCC_2_3(self):

        cluster = Clusters.Objects.OccupancySensing
        attributes = cluster.Attributes

        self.step(1)  # Commissioning already done

        self.step(2)

        feature_map = await self.read_occ_attribute_expect_success(attribute=attributes.FeatureMap)
        has_feature_pir = (feature_map & cluster.Bitmaps.Feature.kPassiveInfrared) != 0
        has_feature_ultrasonic = (feature_map & cluster.Bitmaps.Feature.kUltrasonic) != 0
        has_feature_contact = (feature_map & cluster.Bitmaps.Feature.kPhysicalContact) != 0
        has_no_legacy_features = ((not has_feature_pir) and (not has_feature_ultrasonic) and (not has_feature_contact))

        logging.info(
            f"Feature map: 0x{feature_map:x}. PIR: {has_feature_pir}, US:{has_feature_ultrasonic}, PHY:{has_feature_contact}")

        self.step(3)
        attribute_list = await self.read_occ_attribute_expect_success(attribute=attributes.AttributeList)
        if attributes.HoldTime.attribute_id not in attribute_list:
            logging.info("No HoldTime attribute supports. Terminate this test case")
            self.skip_all_remaining_steps(4)
        holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)

        self.step(4)
        hold_time_limits_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTimeLimits)
        asserts.assert_greater_equal(hold_time_limits_dut.holdTimeMin, 1, "HoldTimeMin has to be greater or equal to 1.")
        await self.write_single_attribute(attributes.HoldTime(hold_time_limits_dut.holdTimeMin))
        holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
        asserts.assert_equal(holdtime_dut, hold_time_limits_dut.holdTimeMin, "HoldTimeMin to HoldTime writing failure")

        self.step(5)
        await self.write_single_attribute(attributes.HoldTime(hold_time_limits_dut.holdTimeMax))
        asserts.assert_greater_equal(hold_time_limits_dut.holdTimeMax, 10, "HoldTimeMax has to be greater or equal to 10.")
        holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
        asserts.assert_equal(holdtime_dut, hold_time_limits_dut.holdTimeMax, "HoldTimeMax to HoldTime writing failure")

        has_pir_timing_attrib = attributes.PIROccupiedToUnoccupiedDelay.attribute_id in attribute_list
        has_no_legacy_features = ((not has_feature_pir) and (not has_feature_ultrasonic) and (not has_feature_contact))

        if (has_feature_pir or has_no_legacy_features) and has_pir_timing_attrib:
            self.step("6a")
            await self.write_single_attribute(attributes.HoldTime(hold_time_limits_dut.holdTimeMin))
            holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
            occupancy_pir_otou_delay_dut = await self.read_occ_attribute_expect_success(attribute=attributes.PIROccupiedToUnoccupiedDelay)
            asserts.assert_equal(occupancy_pir_otou_delay_dut, holdtime_dut,
                                 "PIROccupiedToUnoccupiedDelay has a different value from HoldTime.")

            self.step("6b")
            # perform reverse
            await self.write_single_attribute(attributes.PIROccupiedToUnoccupiedDelay(hold_time_limits_dut.holdTimeMax))
            occupancy_pir_otou_delay_dut = await self.read_occ_attribute_expect_success(
                attribute=attributes.PIROccupiedToUnoccupiedDelay)
            holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
            asserts.assert_equal(occupancy_pir_otou_delay_dut, holdtime_dut,
                                 "PIROccupiedToUnoccupiedDelay has a different value from HoldTime in reverse testing.")
            # self.skip_all_remaining_steps("7a")
        else:
            self.skip_step("6a")
            self.skip_step("6b")

        has_ultrasonic_timing_attrib = attributes.UltrasonicOccupiedToUnoccupiedDelay.attribute_id in attribute_list
        if has_feature_ultrasonic and has_ultrasonic_timing_attrib:
            self.step("7a")
            await self.write_single_attribute(attributes.HoldTime(hold_time_limits_dut.holdTimeMin))
            holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
            occupancy_us_otou_delay_dut = await self.read_occ_attribute_expect_success(attribute=attributes.UltrasonicOccupiedToUnoccupiedDelay)
            asserts.assert_equal(occupancy_us_otou_delay_dut, holdtime_dut,
                                 "UltrasonicOccupiedToUnoccupiedDelay has a different value from HoldTime.")

            self.step("7b")
            # perform reverse
            await self.write_single_attribute(
                attributes.UltrasonicOccupiedToUnoccupiedDelay(hold_time_limits_dut.holdTimeMax))
            occupancy_us_otou_delay_dut = await self.read_occ_attribute_expect_success(
                attribute=attributes.UltrasonicOccupiedToUnoccupiedDelay)
            holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
            asserts.assert_equal(occupancy_us_otou_delay_dut, holdtime_dut,
                                 "UltrasonicOccupiedToUnoccupiedDelay has a different value from HoldTime in reverse testing.")
        else:
            self.skip_step("7a")
            self.skip_step("7b")

        has_contact_timing_attrib = attributes.PhysicalContactOccupiedToUnoccupiedDelay.attribute_id in attribute_list
        if has_feature_contact and has_contact_timing_attrib:
            self.step("8a")
            await self.write_single_attribute(attributes.HoldTime(hold_time_limits_dut.holdTimeMin))
            holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
            occupancy_phy_otou_delay_dut = await self.read_occ_attribute_expect_success(attribute=attributes.PhysicalContactOccupiedToUnoccupiedDelay)
            asserts.assert_equal(occupancy_phy_otou_delay_dut, holdtime_dut,
                                 "PhysicalContactOccupiedToUnoccupiedDelay has a different value from HoldTime.")

            self.step("8b")
            # perform reverse
            await self.write_single_attribute(
                attributes.PhysicalContactOccupiedToUnoccupiedDelay(hold_time_limits_dut.holdTimeMin))
            occupancy_phy_otou_delay_dut = await self.read_occ_attribute_expect_success(
                attribute=attributes.PhysicalContactOccupiedToUnoccupiedDelay)
            holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
            asserts.assert_equal(occupancy_phy_otou_delay_dut, holdtime_dut,
                                 "PhysicalContactOccupiedToUnoccupiedDelay has a different value from HoldTime in reverse testing.")
        else:
            self.skip_step("8a")
            self.skip_step("8b")


if __name__ == "__main__":
    default_matter_test_main()
