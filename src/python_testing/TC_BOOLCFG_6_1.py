#
#    Copyright (c) 2026 Project CHIP Authors
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
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --app-pipe /tmp/boolcfg_6_1_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --app-pipe /tmp/boolcfg_6_1_fifo
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_BOOLCFG_6_1(MatterBaseTest):

    def desc_TC_BOOLCFG_6_1(self) -> str:
        return "[TC-BOOLCFG-6.1] Sensor fault functionality with DUT as Server"

    def steps_TC_BOOLCFG_6_1(self) -> list[TestStep]:
        return [
            TestStep("1", "Commission DUT to TH", is_commissioning=True),
            TestStep("2a", "TH reads FeatureMap attribute.", "DUT replies with FeatureMap attribute."),
            TestStep("2b", "TH reads AttributeList attribute.", "DUT replies with AttributeList attribute."),
            TestStep("3", "If FAULTEV feature is not supported, "
                     "skip remaining steps and end test case."),
            TestStep("4", "Set up a wildcard subscription for attributes and events of the Boolean State Configuration Cluster, "
                     "with MinIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false.",
                     "Subscription successfully established."),
            TestStep("5", "Start accumulating all attribute and event reports on the subscription."),
            TestStep("6", "Prompt operator to cause a sensor fault to be reported on the endpoint under test."),
            TestStep("7", "If SensorFault attribute is supported, TH reads SensorFault attribute.",
                     "DUT responds success and the received value is not equal to 0."),
            TestStep("8", "Wait for up to 30 seconds for TH to have received an event data report.",
                     "An event report has been received from DUT within 30 seconds for the SensorFault event "
                     "and the received report contains a value for SensorFault not equal to 0."),
            TestStep("9", "If SensorFault attribute is supported, TH waits to receive an attribute data report for up to 30 seconds.",
                     "An attribute report has been received from DUT within 30 seconds for the SensorFault attribute "
                     "and the received report contains a value not equal to 0."),
            TestStep("10", "Prompt operator to clear the sensor fault reported on the endpoint under test."),
            TestStep("11", "If SensorFault attribute is supported, TH reads SensorFault attribute.",
                     "DUT responds success and the received value is equal to 0."),
            TestStep("12", "Wait for up to 30 seconds for TH to have received an event data report.",
                     "An event report has been received from DUT within 30 seconds for the SensorFault event "
                     "and the received report contains a value for SensorFault equal to 0."),
            TestStep("13", "If SensorFault attribute is supported, TH waits to receive an attribute data report for up to 30 seconds.",
                     "An attribute report has been received from DUT within 30 seconds for the SensorFault attribute "
                     "and the received report contains a value equal to 0."),
        ]

    def pics_TC_BOOLCFG_6_1(self) -> list[str]:
        return [
            "BOOLCFG.S",
        ]

    async def _trigger_sensor_fault(self, endpoint: int, fault_value: int) -> None:
        logger.info("Setting SensorFault to 0x%04x on endpoint %d", fault_value, endpoint)
        if self.is_pics_sdk_ci_only:
            self.write_to_app_pipe({"Name": "SetBooleanStateSensorFault", "EndpointId": endpoint, "SensorFault": fault_value})
        else:
            if fault_value != 0:
                result = self.wait_for_user_input(
                    prompt_msg="Cause a sensor fault to be reported on the endpoint under test "
                               "as instructed by the DUT's manufacturer. Were you able to cause a sensor fault?",
                    prompt_msg_placeholder="Enter 'y' or 'n'",
                    default_value="n")
                asserts.assert_equal(result.lower(), "y", "Operator was not able to cause a sensor fault")
            else:
                result = self.wait_for_user_input(
                    prompt_msg="Clear the sensor fault reported on the endpoint under test "
                               "as instructed by the DUT's manufacturer. Were you able to clear the sensor fault?",
                    prompt_msg_placeholder="Enter 'y' or 'n'",
                    default_value="n")
                asserts.assert_equal(result.lower(), "y", "Operator was not able to clear the sensor fault")

    @run_if_endpoint_matches(has_cluster(Clusters.BooleanStateConfiguration))
    async def test_TC_BOOLCFG_6_1(self) -> None:
        cluster = Clusters.BooleanStateConfiguration
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()
        node_id = self.dut_node_id
        dev_ctrl = self.default_controller

        # Step 1: Commissioning
        self.step("1")

        # Step 2a: Read FeatureMap
        self.step("2a")
        feature_map = await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl, node_id=node_id, endpoint=endpoint,
            cluster=cluster, attribute=attributes.FeatureMap)
        logger.info("FeatureMap: 0x%08x", feature_map)

        is_fault_events_supported = feature_map & cluster.Bitmaps.Feature.kFaultEvents

        # Step 2b: Read AttributeList
        self.step("2b")
        attribute_list = await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl, node_id=node_id, endpoint=endpoint,
            cluster=cluster, attribute=attributes.AttributeList)
        logger.info("AttributeList: %s", attribute_list)

        # Step 3: Guard - skip if FAULTEV not supported
        self.step("3")
        if not is_fault_events_supported:
            logger.info("FAULTEV feature not supported, skipping remaining steps")
            self.mark_all_remaining_steps_skipped("4")
            return

        # Step 4: Set up subscription
        self.step("4")
        attr_cb = None
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.SensorFault):
            attr_cb = AttributeSubscriptionHandler(expected_cluster=cluster, expected_attribute=attributes.SensorFault)
            await attr_cb.start(
                dev_ctrl=dev_ctrl, node_id=node_id, endpoint=endpoint,
                min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        event_cb = EventSubscriptionHandler(expected_cluster=cluster)
        await event_cb.start(
            dev_ctrl=dev_ctrl, node_id=node_id, endpoint=endpoint,
            min_interval_sec=0, max_interval_sec=30)

        # Step 5: Start accumulating reports - flush any priming reports received during subscription setup
        self.step("5")
        if attr_cb is not None:
            attr_cb.reset()
        event_cb.reset()

        # Step 6: Trigger sensor fault
        self.step("6")
        await self._trigger_sensor_fault(endpoint, fault_value=1)

        # Step 7: Read SensorFault attribute - should be non-zero
        self.step("7")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.SensorFault):
            sensor_fault = await self.read_single_attribute_check_success(
                dev_ctrl=dev_ctrl, node_id=node_id, endpoint=endpoint,
                cluster=cluster, attribute=attributes.SensorFault)
            logger.info("SensorFault attribute: 0x%04x", sensor_fault)
            asserts.assert_not_equal(sensor_fault, 0, "SensorFault should not be 0 after triggering fault")

        # Step 8: Wait for SensorFault event report with non-zero value
        self.step("8")
        event_data = event_cb.wait_for_event_type_report(cluster.Events.SensorFault, timeout_sec=30)
        logger.info("Received SensorFault event: sensorFault=0x%04x", event_data.sensorFault)
        asserts.assert_not_equal(event_data.sensorFault, 0,
                                 "SensorFault event should contain a non-zero sensorFault value")

        # Step 9: Wait for SensorFault attribute report with non-zero value
        self.step("9")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.SensorFault):
            item = attr_cb.wait_for_attribute_report(timeout_sec=30)
            logger.info("Received SensorFault attribute report: 0x%04x", item.value)
            asserts.assert_not_equal(item.value, 0, "SensorFault attribute report should be non-zero")

        # Reset accumulated reports before clearing the fault
        if attr_cb is not None:
            attr_cb.reset()
        event_cb.reset()

        # Step 10: Clear sensor fault
        self.step("10")
        await self._trigger_sensor_fault(endpoint, fault_value=0)

        # Step 11: Read SensorFault attribute - should be 0
        self.step("11")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.SensorFault):
            sensor_fault = await self.read_single_attribute_check_success(
                dev_ctrl=dev_ctrl, node_id=node_id, endpoint=endpoint,
                cluster=cluster, attribute=attributes.SensorFault)
            logger.info("SensorFault attribute: 0x%04x", sensor_fault)
            asserts.assert_equal(sensor_fault, 0, "SensorFault should be 0 after clearing fault")

        # Step 12: Wait for SensorFault event report with zero value
        self.step("12")
        event_data = event_cb.wait_for_event_type_report(cluster.Events.SensorFault, timeout_sec=30)
        logger.info("Received SensorFault event: sensorFault=0x%04x", event_data.sensorFault)
        asserts.assert_equal(event_data.sensorFault, 0,
                             "SensorFault event should contain a zero sensorFault value after clearing")

        # Step 13: Wait for SensorFault attribute report with zero value
        self.step("13")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.SensorFault):
            item = attr_cb.wait_for_attribute_report(timeout_sec=30)
            logger.info("Received SensorFault attribute report: 0x%04x", item.value)
            asserts.assert_equal(item.value, 0, "SensorFault attribute report should be zero")


if __name__ == "__main__":
    default_matter_test_main()
