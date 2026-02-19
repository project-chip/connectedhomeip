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
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/occ_3_1_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --app-pipe /tmp/occ_3_1_fifo
#       --bool-arg simulate_occupancy:true
#     factory-reset: true
#     quiet: true
#   run2:
#     app: ${ALL_DEVICES_APP}
#     app-args: --device occupancy-sensor --discriminator 1234 --KVS kvs1
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#
#  There are CI issues to be followed up for the test cases below that implements manually controlling sensor device for
#  the occupancy state ON/OFF change.
#  [TC-OCC-3.1] test procedure step 5, 9, 14
#  [TC-OCC-3.2] test procedure step 3a, 3c

import asyncio
import logging
from typing import Any, Optional

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import AttributeValue, MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_OCC_3_1(MatterBaseTest):
    def setup_test(self):
        super().setup_test()
        self.is_ci = self.matter_test_config.global_test_params.get('simulate_occupancy', False)

    async def read_occ_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.OccupancySensing
        endpoint = self.get_endpoint()
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def write_hold_time(self, hold_time: Optional[Any]) -> Status:
        dev_ctrl = self.default_controller
        node_id = self.dut_node_id
        endpoint = self.get_endpoint()

        cluster = Clusters.OccupancySensing
        write_result = await dev_ctrl.WriteAttribute(node_id, [(endpoint, cluster.Attributes.HoldTime(hold_time))])
        return write_result[0].Status

    def desc_TC_OCC_3_1(self) -> str:
        return "[TC-OCC-3.1] Primary functionality with server as DUT"

    def steps_TC_OCC_3_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH.", is_commissioning=True),
            TestStep(2, "TH reads AttributeList attribute."),
            TestStep(3, "If HoldTime attribute is supported, TH writes HoldTime attribute to 10 sec on DUT."),
            TestStep(4, "If HoldTime attribute is supported, TH reads HoldTime attribute."),
            TestStep(5, "Prompt operator to await until DUT occupancy is in unoccupied state and confirm before moving on."),
            TestStep(6, "TH reads Occupancy attribute."),
            TestStep(7, "Set up a wildcard subscription for attributes and events of the Occupancy Sensing Cluster."),
            TestStep(8, "Start accumulating all attribute and event reports on the subscription."),
            TestStep(9, "Prompt operator to trigger occupancy change to occupied."),
            TestStep(10, "TH reads Occupancy attribute."),
            TestStep(11, "Wait for up to 30 seconds for TH to have received an attribute data report."),
            TestStep(12, "If OCCEVENT feature is supported or OCC.S.E00(OccupancyChanged) is set, TH waits for event data report."),
            TestStep(13, "TH clears the accumulated subscription reports and restarts accumulating."),
            TestStep(14, "Prompt operator to ensure sensor no longer detects occupancy. Wait for HoldTime duration if supported."),
            TestStep(15, "TH reads Occupancy attribute."),
            TestStep(16, "Wait for up to 30 seconds for TH to have received an attribute data report."),
            TestStep(17, "If OCCEVENT feature is supported or OCC.S.E00(OccupancyChanged) is set, TH waits for event data report."),
        ]

    def pics_TC_OCC_3_1(self) -> list[str]:
        return [
            "OCC.S",
        ]

    @async_test_body
    async def test_TC_OCC_3_1(self):
        hold_time = 10 if not self.is_ci else 1  # 10 seconds for occupancy state hold time
        endpoint_id = self.get_endpoint()
        node_id = self.dut_node_id
        dev_ctrl = self.default_controller

        self.step(1)  # Commissioning already done

        self.step(2)
        cluster = Clusters.OccupancySensing
        attributes = cluster.Attributes
        attribute_list = await self.read_occ_attribute_expect_success(attribute=attributes.AttributeList)

        has_hold_time = attributes.HoldTime.attribute_id in attribute_list

        # Check for OCCEVENT feature (F09) or OCC.S.E00(OccupancyChanged) PICS
        feature_map = await self.read_occ_attribute_expect_success(attribute=attributes.FeatureMap)

        # OCCEVENT (F09) requires cluster Rev >= v7
        try:
            has_occevent_feature = (feature_map & cluster.Bitmaps.Feature.kOccupancyEventReporting) != 0
        except AttributeError:
            has_occevent_feature = False

        occupancy_event_supported = has_occevent_feature or self.check_pics("OCC.S.E00")

        log.info(f"Feature map: 0x{feature_map:x}, OCCEVENT feature: {has_occevent_feature}")
        log.info(f"HoldTime supported: {has_hold_time}, OccupancyChanged event supported: {occupancy_event_supported}")

        self.step(3)
        if has_hold_time:
            # Write HoldTime attribute to 10 sec
            await self.write_single_attribute(cluster.Attributes.HoldTime(hold_time))
        else:
            log.info("HoldTime attribute not supported. Skipping step 3.")
            self.mark_current_step_skipped()

        self.step(4)
        if has_hold_time:
            holdtime_dut = await self.read_occ_attribute_expect_success(attribute=attributes.HoldTime)
            asserts.assert_equal(holdtime_dut, hold_time, f"HoldTime read-back does not match the written value of {hold_time}")
        else:
            log.info("HoldTime attribute not supported. Skipping step 4.")
            self.mark_current_step_skipped()

        self.step(5)
        if self.is_ci:
            # CI call to trigger unoccupied.
            self.write_to_app_pipe({"Name": "SetOccupancy", "EndpointId": endpoint_id, "Occupancy": 0})
        else:
            self.wait_for_user_input(
                prompt_msg="Type any letter and press ENTER after the sensor occupancy is in unoccupied state (occupancy attribute = 0)")

        self.step(6)
        # Read and verify Occupancy = 0 (Unoccupied)
        occupancy_dut = await self.read_occ_attribute_expect_success(attribute=attributes.Occupancy)
        asserts.assert_equal(occupancy_dut, 0, "Occupancy attribute is not 0 (Unoccupied).")

        self.step(7)
        # Set up wildcard subscription for attributes and events
        # MinIntervalFloor = 0, MaxIntervalCeiling = 30, KeepSubscriptions = false (EventSubscriptionHandler has True hardcoded and can't be changed)
        attrib_listener = AttributeSubscriptionHandler(expected_cluster=cluster)
        await attrib_listener.start(dev_ctrl, node_id, endpoint=endpoint_id, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        if occupancy_event_supported:
            event_listener = EventSubscriptionHandler(expected_cluster=cluster)
            await event_listener.start(dev_ctrl, node_id, endpoint=endpoint_id, min_interval_sec=0, max_interval_sec=30)

        self.step(8)
        # Start accumulating all attribute and event reports on the subscription
        # (Already being done by the subscription handlers)
        log.info("Subscription established. Accumulating attribute and event reports.")

        self.step(9)
        # Prompt operator to trigger occupancy change to occupied
        if self.is_ci:
            self.write_to_app_pipe({"Name": "SetOccupancy", "EndpointId": endpoint_id, "Occupancy": 1})
        else:
            self.wait_for_user_input(prompt_msg="Type any letter and press ENTER after triggering occupancy to occupied.")

        self.step(10)
        # TH reads Occupancy attribute - verify received value is 1 (Occupied)
        occupancy_dut = await self.read_occ_attribute_expect_success(attribute=attributes.Occupancy)
        asserts.assert_equal(occupancy_dut, 1, "Occupancy attribute is not 1 (Occupied).")

        self.step(11)
        # Wait for up to 30 seconds for attribute data report with Occupancy = 1
        attrib_listener.await_all_final_values_reported(
            expected_final_values=[AttributeValue(endpoint_id=endpoint_id,
                                                  attribute=cluster.Attributes.Occupancy, value=1)],
            timeout_sec=30.0)
        log.info("Received attribute report for Occupancy = 1 (Occupied).")

        self.step(12)
        if occupancy_event_supported:
            # Wait for OccupancyChanged event with Occupancy = 1
            event = event_listener.wait_for_event_report(cluster.Events.OccupancyChanged, timeout_sec=30.0)
            asserts.assert_equal(event.occupancy, 1, "OccupancyChanged event did not report Occupancy = 1 (Occupied)")
            log.info("Received OccupancyChanged event with Occupancy = 1 (Occupied).")
        else:
            log.info("OccupancyChanged event not supported. Skipping step 12.")
            self.mark_current_step_skipped()

        self.step(13)
        # Clear accumulated reports and restart accumulating
        attrib_listener.reset()
        if occupancy_event_supported:
            event_listener.reset()
        log.info("Cleared accumulated reports. Restarting accumulation.")

        self.step(14)
        # Prompt operator to ensure sensor no longer detects occupancy. Wait for HoldTime duration if supported
        if self.is_ci:
            # CI call to trigger unoccupied.
            self.write_to_app_pipe({"Name": "SetOccupancy", "EndpointId": endpoint_id, "Occupancy": 0})
        else:
            self.wait_for_user_input(
                prompt_msg="Ensure the sensor no longer detects occupancy, then press ENTER")

        if has_hold_time:
            log.info(f"Waiting for HoldTime duration ({hold_time} seconds) plus buffer...")
            await asyncio.sleep(hold_time + 2.0)  # add extra 2 seconds buffer

        self.step(15)
        # TH reads Occupancy attribute - verify received value is 0 (Unoccupied)
        occupancy_dut = await self.read_occ_attribute_expect_success(attribute=attributes.Occupancy)
        asserts.assert_equal(occupancy_dut, 0, "Occupancy attribute is not 0 (Unoccupied) after HoldTime period.")

        self.step(16)
        # Wait for up to 30 seconds for attribute data report with Occupancy = 0
        attrib_listener.await_all_final_values_reported(
            expected_final_values=[AttributeValue(endpoint_id=endpoint_id,
                                                  attribute=cluster.Attributes.Occupancy, value=0)],
            timeout_sec=30.0)
        log.info("Received attribute report for Occupancy = 0 (Unoccupied).")

        self.step(17)
        if occupancy_event_supported:
            # Wait for OccupancyChanged event with Occupancy = 0
            event = event_listener.wait_for_event_report(cluster.Events.OccupancyChanged, timeout_sec=30.0)
            asserts.assert_equal(event.occupancy, 0, "OccupancyChanged event did not report Occupancy = 0 (Unoccupied)")
            log.info("Received OccupancyChanged event with Occupancy = 0 (Unoccupied).")
        else:
            log.info("OccupancyChanged event not supported. Skipping step 17.")
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
