#
#    Copyright (c) 2024 Project CHIP (Matter) Authors
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

#  TODO: There are CI issues to be followed up for the test cases below that implements manually controlling sensor device for
#  the occupancy state ON/OFF change.
#  [TC-OCC-3.1] test procedure step 4
#  [TC-OCC-3.2] test precedure step 3a, 3c

import logging

import chip.clusters as Clusters
from matter_testing_support import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, async_test_body,
                                    await_sequence_of_reports, default_matter_test_main)
from mobly import asserts


class TC_OCC_3_2(MatterBaseTest):
    async def read_occ_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.OccupancySensing
        endpoint_id = self.matter_test_config.endpoint
        return await self.read_single_attribute_check_success(endpoint=endpoint_id, cluster=cluster, attribute=attribute)

    def desc_TC_OCC_3_2(self) -> str:
        return "[TC-OCC-3.2] Subscription Report Verification with server as DUT"

    def steps_TC_OCC_3_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH if not already done", is_commissioning=True),
            TestStep(2, "TH establishes a wildcard subscription to all attributes on Occupancy Sensing Cluster on the endpoint under test. Subscription min interval = 0 and max interval = 30 seconds."),
            TestStep("3a", "Do not trigger DUT for occupancy state change."),
            TestStep("3b", "TH reads DUT Occupancy attribute and saves the initial value as initial"),
            TestStep("3c", "Trigger DUT to change the occupancy state."),
            TestStep("3d", "TH awaits a ReportDataMessage containing an attribute report for DUT Occupancy attribute."),
            TestStep("4a", "Check if DUT supports HoldTime attribute, If not supported, then stop and skip the rest of test cases."),
            TestStep("4b", "TH reads DUT HoldTime attribute and saves the initial value as initial"),
            TestStep("4c", "TH writes a different value to DUT HoldTime attribute."),
            TestStep("4d", "TH awaits a ReportDataMessage containing an attribute report for DUT HoldTime attribute."),
            TestStep("5a", "Check if DUT supports DUT feature flag PIR or (!PIR & !US & !PHY) and has the PIROccupiedToUnoccupiedDelay attribute, If not supported, then skip 5b, 5c, 5d."),
            TestStep("5b", "TH reads DUT PIROccupiedToUnoccupiedDelay attribute and saves the initial value as initial"),
            TestStep("5c", "TH writes a different value to DUT PIROccupiedToUnoccupiedDelay attribute."),
            TestStep("5d", "TH awaits a ReportDataMessage containing an attribute report for DUT PIROccupiedToUnoccupiedDelay attribute."),
            TestStep("6a", "Check if DUT supports DUT feature flag US and has the UltrasonicOccupiedToUnoccupiedDelay attribute. If not supported, then skip 6b, 6c, 6d."),
            TestStep("6b", "TH reads DUT UltrasonicOccupiedToUnoccupiedDelay attribute and saves the initial value as initial"),
            TestStep("6c", "TH writes a different value to DUT UltrasonicOccupiedToUnoccupiedDelay attribute."),
            TestStep("6d", "TH awaits a ReportDataMessage containing an attribute report for DUT UltrasonicOccupiedToUnoccupiedDelay attribute."),
            TestStep("7a", "Check if DUT supports DUT feature flag PHY and has the PhysicalContactOccupiedToUnoccupiedDelay attribute. If not supported, skip 7b, 7c, 7d."),
            TestStep("7b", "TH reads DUT PhysicalContactOccupiedToUnoccupiedDelay attribute and saves the initial value as initial"),
            TestStep("7c", "TH writes a different value to DUT PhysicalContactOccupiedToUnoccupiedDelay attribute."),
            TestStep("7d", "TH awaits a ReportDataMessage containing an attribute report for DUT PhysicalContactOccupiedToUnoccupiedDelay attribute.")
        ]
        return steps

    def pics_TC_OCC_3_2(self) -> list[str]:
        pics = [
            "OCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_OCC_3_2(self):
        endpoint_id = self.matter_test_config.endpoint
        node_id = self.dut_node_id
        dev_ctrl = self.default_controller

        post_prompt_settle_delay_seconds = 10.0
        cluster = Clusters.Objects.OccupancySensing
        attributes = cluster.Attributes

        self.step(1)  # Commissioning already done

        self.step(2)
        feature_map = await self.read_occ_attribute_expect_success(attribute=attributes.FeatureMap)
        has_feature_pir = (feature_map & cluster.Bitmaps.Feature.kPassiveInfrared) != 0
        has_feature_ultrasonic = (feature_map & cluster.Bitmaps.Feature.kUltrasonic) != 0
        has_feature_contact = (feature_map & cluster.Bitmaps.Feature.kPhysicalContact) != 0

        logging.info(
            f"Feature map: 0x{feature_map:x}. PIR: {has_feature_pir}, US:{has_feature_ultrasonic}, PHY:{has_feature_contact}")

        attribute_list = await self.read_occ_attribute_expect_success(attribute=attributes.AttributeList)
        has_pir_timing_attrib = attributes.PIROccupiedToUnoccupiedDelay.attribute_id in attribute_list
        has_ultrasonic_timing_attrib = attributes.UltrasonicOccupiedToUnoccupiedDelay.attribute_id in attribute_list
        has_contact_timing_attrib = attributes.PhysicalContactOccupiedToUnoccupiedDelay.attribute_id in attribute_list
        logging.info(f"Attribute list: {attribute_list}")
        logging.info(f"--> Has PIROccupiedToUnoccupiedDelay: {has_pir_timing_attrib}")
        logging.info(f"--> Has UltrasonicOccupiedToUnoccupiedDelay: {has_ultrasonic_timing_attrib}")
        logging.info(f"--> Has PhysicalContactOccupiedToUnoccupiedDelay: {has_contact_timing_attrib}")

        # min interval = 0, and max interval = 30 seconds
        attrib_listener = ClusterAttributeChangeAccumulator(Clusters.Objects.OccupancySensing)
        await attrib_listener.start(dev_ctrl, node_id, endpoint=endpoint_id, min_interval_sec=0, max_interval_sec=30)

        # TODO - Will add Namepiped to assimilate the manual sensor untrigger here
        self.step("3a")
        self.wait_for_user_input(prompt_msg="Type any letter and press ENTER after DUT goes back to unoccupied state.")

        self.step("3b")
        if attributes.Occupancy.attribute_id in attribute_list:
            initial_dut = await self.read_occ_attribute_expect_success(attribute=attributes.Occupancy)
            asserts.assert_equal(initial_dut, 0, "Occupancy attribute is still detected state")

        # TODO - Will add Namepiped to assimilate the manual sensor trigger here
        self.step("3c")
        self.wait_for_user_input(
            prompt_msg="Type any letter and press ENTER after the sensor occupancy is triggered and its occupancy state changed.")

        self.step("3d")
        await_sequence_of_reports(report_queue=attrib_listener.attribute_queue, endpoint_id=endpoint_id, attribute=cluster.Attributes.Occupancy, sequence=[
                                  1], timeout_sec=post_prompt_settle_delay_seconds)

        self.step("4a")
        if attributes.HoldTime.attribute_id not in attribute_list:
            logging.info("No HoldTime attribute supports. Terminate this test case")
            self.skip_all_remaining_steps("4b")

        self.step("4b")
        initial_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)

        self.step("4c")
        # write a different a HoldTime attribute value
        diff_val = 12
        await self.write_single_attribute(attributes.HoldTime(diff_val))

        self.step("4d")
        await_sequence_of_reports(report_queue=attrib_listener.attribute_queue, endpoint_id=endpoint_id, attribute=cluster.Attributes.HoldTime, sequence=[
                                  diff_val], timeout_sec=post_prompt_settle_delay_seconds)

        self.step("5a")

        has_no_legacy_features = ((not has_feature_pir) and (not has_feature_ultrasonic) and (not has_feature_contact))

        if has_pir_timing_attrib and (has_feature_pir or has_no_legacy_features):
            self.step("5b")
            initial_dut = await self.read_occ_attribute_expect_success(attribute=attributes.PIROccupiedToUnoccupiedDelay)

            self.step("5c")
            # write the new attribute value
            diff_val = 11
            await self.write_single_attribute(attributes.PIROccupiedToUnoccupiedDelay(diff_val))

            self.step("5d")
            await_sequence_of_reports(report_queue=attrib_listener.attribute_queue, endpoint_id=endpoint_id, attribute=cluster.Attributes.PIROccupiedToUnoccupiedDelay, sequence=[
                                      diff_val], timeout_sec=post_prompt_settle_delay_seconds)
        else:
            logging.info("No PIR timing attribute support. Skipping steps 5b, 5c, 5d")
            self.skip_step("5b")
            self.skip_step("5c")
            self.skip_step("5d")

        self.step("6a")
        if not has_feature_ultrasonic or not has_ultrasonic_timing_attrib:
            logging.info("No Ultrasonic timing attribute supports. Skipping steps 6b, 6c, 6d")
            self.skip_step("6b")
            self.skip_step("6c")
            self.skip_step("6d")
        else:
            self.step("6b")
            initial_dut = await self.read_occ_attribute_expect_success(attribute=attributes.UltrasonicOccupiedToUnoccupiedDelay)

            self.step("6c")
            # write the new attribute value
            diff_val = 14
            await self.write_single_attribute(attributes.UltrasonicOccupiedToUnoccupiedDelay(diff_val))

            self.step("6d")
            await_sequence_of_reports(report_queue=attrib_listener.attribute_queue, endpoint_id=endpoint_id, attribute=cluster.Attributes.UltrasonicOccupiedToUnoccupiedDelay, sequence=[
                                      diff_val], timeout_sec=post_prompt_settle_delay_seconds)

        self.step("7a")
        if not has_feature_contact or not has_contact_timing_attrib:
            logging.info("No Physical contact timing attribute supports. Skipping steps 7b, 7c, 7d")
            self.skip_step("7b")
            self.skip_step("7c")
            self.skip_step("7d")
        else:
            self.step("7b")
            initial_dut = await self.read_occ_attribute_expect_success(attribute=attributes.PhysicalContactOccupiedToUnoccupiedDelay)

            self.step("7c")
            # write the new attribute value
            diff_val = 9
            await self.write_single_attribute(attributes.PhysicalContactOccupiedToUnoccupiedDelay(diff_val))

            self.step("7d")
            await_sequence_of_reports(report_queue=attrib_listener.attribute_queue, endpoint_id=endpoint_id, attribute=cluster.Attributes.PhysicalContactOccupiedToUnoccupiedDelay, sequence=[
                                      diff_val], timeout_sec=post_prompt_settle_delay_seconds)


if __name__ == "__main__":
    default_matter_test_main()
