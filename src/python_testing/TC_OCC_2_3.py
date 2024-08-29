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
from chip import ChipDeviceCtrl
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_OCC_2_3(MatterBaseTest):
    async def read_occ_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.OccupancySensing
        endpoint_id = self.matter_test_config.endpoint
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_OCC_2_3(self) -> str:
        return "[TC-OCC-2.3] HoldTime Backward Compatibility Test with server as DUT"

    def steps_TC_OCC_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads the FeatureMap attribute on the endpoint and verifies if DUT supports any of the PIR (PassiveInfrared), US (Ultrasonic), PHY (PhysicalContact) or OTHER features."),
            TestStep(3, "TH checks DUT HoldTime attribute support. If DUT doesn't support HoldTime, skip the rest of this test case."),
            TestStep(4, "TH writes DUT HoldTime attribute with HoldTimeMin and verify."),
            TestStep(5, "TH writes DUT HoldTime attribute with HoldTimeMax and verify."),
            TestStep(6a, "If DUT supports either of PIR or OTHER featuremap, and PIROccupiedToUnoccupiedDelay, then TH reads DUT PIROccupiedToUnoccupiedDelay and HoldTime attributes.")
            TestStep(6b, "TH writes DUT PIROccupiedToUnoccupiedDelay attribute with HoldTimeMin, then TH reads DUT PIROccupiedToUnoccupiedDelay and HoldTime attributes.")
            TestStep(7a, "If DUT supports US featuremap and UltrasonicOccupiedToUnoccupiedDelay, then TH reads DUT UltrasonicOccupiedToUnoccupiedDelay and HoldTime attributes.")
            TestStep(7b, "TH writes DUT UltrasonicOccupiedToUnoccupiedDelay attribute with HoldTimeMin, then TH reads DUT UltrasonicOccupiedToUnoccupiedDelay and HoldTime attributes.")
            TestStep(8a, "If DUT supports PHY featuremap and PhysicalContactOccupiedToUnoccupiedDelay, then TH reads DUT PhysicalContactOccupiedToUnoccupiedDelay and HoldTime attributes.")
            TestStep(8b, "TH writes DUT PhysicalContactOccupiedToUnoccupiedDelay attribute with HoldTimeMin, then TH reads DUT PhysicalContactOccupiedToUnoccupiedDelay and HoldTime attributes.")
        ]
        return steps

    def pics_TC_OCC_2_3(self) -> list[str]:
        pics = [
            "OCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_OCC_2_3(self):

        endpoint_id = self.matter_test_config.endpoint
        node_id = self.dut_node_id
        dev_ctrl = self.default_controller

        post_prompt_settle_delay_seconds = 10.0
        cluster = Clusters.Objects.OccupancySensing
        attributes = cluster.Attributes

        self.step(1)  # Commissioning already done

        self.step(2)
        attribute_list = await self.read_occ_attribute_expect_success(attribute=attributes.AttributeList)

        feature_map = await self.read_occ_attribute_expect_success(attribute=attributes.FeatureMap)
        has_feature_pir = (feature_map & cluster.Bitmaps.Feature.kPassiveInfrared) != 0
        has_feature_ultrasonic = (feature_map & cluster.Bitmaps.Feature.kUltrasonic) != 0
        has_feature_contact = (feature_map & cluster.Bitmaps.Feature.kPhysicalContact) != 0
        has_feature_other = (feature_map & cluster.Bitmaps.Feature.kOther) != 0

        logging.info(
            f"Feature map: 0x{feature_map:x}. PIR: {has_feature_pir}, US:{has_feature_ultrasonic}, PHY:{has_feature_contact}")

        if not has_feature_pir or not has_feature_ultrasonic or not has_feature_contact or not has_feature_other:
            logging.info("PIR, US, PHY, OTHER featuremap not supported. Stop this test case.")
            self.skip_all_remaining_steps("3")

        self.step(3)
        if not attributes.HoldTime.attribute_id in attribute_list:
            logging.info("No HoldTime attribute supports. Terminate this test case")
            self.skip_all_remaining_steps("4")
            occupancy_hold_time_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)

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

        self.step(6a)
        has_pir_timing_attrib = attributes.PIROccupiedToUnoccupiedDelay.attribute_id in attribute_list
        has_ultrasonic_timing_attrib = attributes.UltrasonicOccupiedToUnoccupiedDelay.attribute_id in attribute_list
        has_contact_timing_attrib = attributes.PhysicalContactOccupiedToUnoccupiedDelay.attribute_id in attribute_list

        if (has_feature_pir or has_feature_other) and has_pir_timing_attrib:
            occupancy_pir_otou_delay_dut = await self.read_occ_attribute_expect_success(attribute=attributes.PIROccupiedToUnoccupiedDelay)
            asserts.assert_equal(occupancy_pir_otou_delay_dut, holdtime_dut, 
				 "PIROccupiedToUnoccupiedDelay has a different value from HoldTime.")

	    self.step(6b)
	    # perform reverse
	    await self.write_single_attribute(attributes.PIROccupiedToUnoccupiedDelay(hold_time_limits_dut.holdTimeMin))
            occupancy_pir_otou_delay_dut = await self.read_occ_attribute_expect_success(attribute=attributes.PIROccupiedToUnoccupiedDelay)
	    holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
	    asserts.assert_equal(occupancy_pir_otou_delay_dut, holdtime_dut, 
				 "PIROccupiedToUnoccupiedDelay has a different value from HoldTime in reverse testing.")
	    self.skip_all_remaining_steps("7a")

      	self.step(7a)
        if has_feature_ultrasonic and has_ultrasonic_timing_attrib:
            occupancy_us_otou_delay_dut = await self.read_occ_attribute_expect_success(attribute=attributes.UltrasonicOccupiedToUnoccupiedDelay)
            asserts.assert_equal(occupancy_us_otou_delay_dut, holdtime_dut, 
				 "UltrasonicOccupiedToUnoccupiedDelay has a different value from HoldTime.")

	    self.step(7b)
	    # perform reverse
	    await self.write_single_attribute(attributes.UltrasonicOccupiedToUnoccupiedDelay(hold_time_limits_dut.holdTimeMin))
	    occupancy_us_otou_delay_dut = await self.read_occ_attribute_expect_success(attribute=attributes.UltrasonicOccupiedToUnoccupiedDelay)
	    holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
	    asserts.assert_equal(occupancy_us_otou_delay_dut, holdtime_dut, 
				 "UltrasonicOccupiedToUnoccupiedDelay has a different value from HoldTime in reverse testing.")
	    self.skip_all_remaining_steps("8a")

      	self.step(8a)
        if has_feature_contact and has_contact_timing_attrib:
            occupancy_phy_otou_delay_dut = await self.read_occ_attribute_expect_success(attribute=attributes.PhysicalContactOccupiedToUnoccupiedDelay)
            asserts.assert_equal(occupancy_phy_otou_delay_dut, holdtime_dut, 
				 "PhysicalContactOccupiedToUnoccupiedDelay has a different value from HoldTime.")

            self.step(8b)
            # perform reverse
            await self.write_single_attribute(attributes.PhysicalContactOccupiedToUnoccupiedDelay(hold_time_limits_dut.holdTimeMin))
            occupancy_phy_otou_delay_dut = await self.read_occ_attribute_expect_success(attribute=attributes.PhysicalContactOccupiedToUnoccupiedDelay)
            holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
            asserts.assert_equal(occupancy_phy_otou_delay_dut, holdtime_dut,
				 "PhysicalContactOccupiedToUnoccupiedDelay has a different value from HoldTime in reverse testing.")


if __name__ == "__main__":
    default_matter_test_main()
