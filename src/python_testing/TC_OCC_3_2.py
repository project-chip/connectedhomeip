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
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --bool-arg simulate_occupancy:true
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

#  There are CI integration for the test cases below that implements manually controlling sensor device for
#  the occupancy state ON/OFF change.
#  [TC-OCC-3.1] test procedure step 3, 4
#  [TC-OCC-3.2] test precedure step 3a, 3c

import logging
import time

import chip.clusters as Clusters
from chip.testing.matter_testing import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, async_test_body,
                                         await_sequence_of_reports, default_matter_test_main)
from mobly import asserts


class TC_OCC_3_2(MatterBaseTest):
    def setup_test(self):
        super().setup_test()
        self.is_ci = self.matter_test_config.global_test_params.get('simulate_occupancy', False)

    async def read_occ_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.OccupancySensing
        endpoint_id = self.get_endpoint()
        return await self.read_single_attribute_check_success(endpoint=endpoint_id, cluster=cluster, attribute=attribute)

    def desc_TC_OCC_3_2(self) -> str:
        return "[TC-OCC-3.2] Subscription Report Verification with server as DUT"

    def steps_TC_OCC_3_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH if not already done", is_commissioning=True),
            TestStep(2, "TH establishes a wildcard subscription to all attributes on Occupancy Sensing Cluster on the endpoint under test. Subscription min interval = 0 and max interval = 30 seconds."),
            TestStep("3a", "Prepare DUT to be unoccupied state."),
            TestStep("3b", "TH reads DUT Occupancy attribute."),
            TestStep("3c", "Trigger DUT to change the occupancy state."),
            TestStep("3d", "TH awaits a ReportDataMessage containing an attribute report for DUT Occupancy attribute."),
            TestStep("4a", "Check if DUT supports HoldTime attribute, If not supported, then stop and skip the rest of test cases."),
            TestStep("4b", "TH writes HoldTimeMin to HoldTime attribute."),
            TestStep("4c", "TH clears its report history and writes HoldTimeMax to HoldTime attribute."),
            TestStep("4d", "TH awaits a ReportDataMessage containing an attribute report for DUT HoldTime attribute and all legacy attributes supported."),
        ]
        return steps

    def pics_TC_OCC_3_2(self) -> list[str]:
        pics = [
            "OCC.S",
        ]
        return pics

    # Sends and out-of-band command to the all-clusters-app
    def write_to_app_pipe(self, command):
        # CI app pipe id creation
        self.app_pipe = "/tmp/chip_all_clusters_fifo_"
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                asserts.fail("The --app-pid flag must be set when using named pipe")
            self.app_pipe = self.app_pipe + str(app_pid)

        with open(self.app_pipe, "w") as app_pipe:
            app_pipe.write(command + "\n")
        # Delay for pipe command to be processed (otherwise tests are flaky)
        time.sleep(0.001)

    @async_test_body
    async def test_TC_OCC_3_2(self):
        endpoint_id = self.get_endpoint()
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

        # add Namepiped to assimilate the manual sensor untrigger here
        self.step("3a")
        # CI call to trigger off
        if self.is_ci:
            self.write_to_app_pipe('{"Name":"SetOccupancy", "EndpointId": 1, "Occupancy": 0}')
        else:
            self.wait_for_user_input(prompt_msg="Type any letter and press ENTER after DUT goes back to unoccupied state.")

        self.step("3b")
        initial_dut = await self.read_occ_attribute_expect_success(attribute=attributes.Occupancy)
        asserts.assert_equal(initial_dut, 0, "Occupancy attribute is still detected state")

        # add Namepiped to assimilate the manual sensor trigger here
        self.step("3c")
        attrib_listener.reset()

        # CI call to trigger on
        if self.is_ci:
            self.write_to_app_pipe('{"Name":"SetOccupancy", "EndpointId": 1, "Occupancy": 1}')
        else:
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
        hold_time_limits_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTimeLimits)
        hold_time_min = hold_time_limits_dut.holdTimeMin
        hold_time_max = hold_time_limits_dut.holdTimeMax
        await self.write_single_attribute(attributes.HoldTime(hold_time_min))
        hold_time_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
        asserts.assert_equal(hold_time_dut, hold_time_min, "HoldTime did not match written HoldTimeMin")

        # HoldTime may already have been HoldTimeMin, or not. Make sure we look only at subsequent reports.
        attrib_listener.reset()

        self.step("4c")
        await self.write_single_attribute(attributes.HoldTime(hold_time_max))

        self.step("4d")
        await_sequence_of_reports(report_queue=attrib_listener.attribute_queue, endpoint_id=endpoint_id,
                                  attribute=cluster.Attributes.HoldTime, sequence=[hold_time_max], timeout_sec=post_prompt_settle_delay_seconds)


if __name__ == "__main__":
    default_matter_test_main()
