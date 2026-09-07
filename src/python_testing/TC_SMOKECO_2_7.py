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
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/smokeco_2_7_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --app-pipe /tmp/smokeco_2_7_fifo
#       --bool-arg simulate_mounting:true
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_SMOKECO_2_7(MatterBaseTest):
    def setup_test(self):
        super().setup_test()
        self.is_ci = self.matter_test_config.global_test_params.get('simulate_mounting', False)

    async def read_smokeco_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.SmokeCoAlarm
        endpoint = self.get_endpoint()
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_SMOKECO_2_7(self) -> str:
        return "[TC-SMOKECO-2.7] Unmount Attribute with DUT as Server"

    def steps_TC_SMOKECO_2_7(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH.", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the BatteryAlert attribute."),
            TestStep(3, "TH reads from the DUT the HardwareFaultAlert attribute."),
            TestStep(4, "TH subscribes to Unmounted attribute with  min interval 0s and max interval 30s."),
            TestStep(5, "TH reads from the DUT the ExpressedState attribute."),
            TestStep(6, "TH prompts operator to unmount the device."),
            TestStep(7, "TH waits for a report of Unmounted attribute from DUT with a timeout of 60 seconds."),
            TestStep(8, "TH reads ExpressedState attribute from DUT."),
            TestStep(9, "TH prompts operator to mount the device."),
            TestStep(10, "TH waits for a report of Unmounted attribute from DUT with a timeout of 60 seconds."),
            TestStep(11, "TH reads ExpressedState attribute from DUT."),
        ]

    def pics_TC_SMOKECO_2_7(self) -> list[str]:
        return [
            "SMOKECO.S.A000d",
        ]

    @async_test_body
    async def test_TC_SMOKECO_2_7(self):
        cluster = Clusters.SmokeCoAlarm
        attributes = cluster.Attributes
        attribute_list = await self.read_smokeco_attribute_expect_success(attribute=attributes.AttributeList)
        endpoint = self.get_endpoint()

        has_battery_alert = attributes.BatteryAlert.attribute_id in attribute_list
        has_hardware_fault_alert = attributes.HardwareFaultAlert.attribute_id in attribute_list
        has_unmounted = attributes.Unmounted.attribute_id in attribute_list
        has_expressed_state = attributes.ExpressedState.attribute_id in attribute_list
        inoperative_when_unmounted_supported = self.check_pics("SMOKECO.M.InoperativeWhenUnmounted") or self.is_ci

        # Step 1, "Commission DUT to TH."
        self.step(1)  # Commissioning already done

        # Step 2, "TH reads from the DUT the BatteryAlert attribute."

        self.step(2)
        if has_battery_alert:
            battery_alert_dut = await self.read_smokeco_attribute_expect_success(attribute=attributes.BatteryAlert)
            asserts.assert_not_equal(battery_alert_dut, 2, "Battery Alert should not be critical (2)")

        # Step 3, "TH reads from the DUT the HardwareFaultAlert attribute."
        self.step(3)
        if has_hardware_fault_alert:
            hardware_fault_alert_dut = await self.read_smokeco_attribute_expect_success(attribute=attributes.HardwareFaultAlert)
            asserts.assert_equal(hardware_fault_alert_dut, 0, "No hardware fault expected")

        # Step 4, "TH subscribes to Unmounted attribute with  min interval 0s and max interval 30s."
        self.step(4)
        if has_unmounted:
            sub_handler = AttributeSubscriptionHandler(expected_cluster=cluster, expected_attribute=attributes.Unmounted)
            await sub_handler.start(self.default_controller, self.dut_node_id, endpoint, max_interval_sec=30)

            unmounted_dut = await self.read_smokeco_attribute_expect_success(attribute=attributes.Unmounted)
            asserts.assert_equal(unmounted_dut, 0, "Expect not unmounted")

        # Step 5, "TH reads from the DUT the ExpressedState attribute."
        self.step(5)
        if has_expressed_state:
            expressed_state_dut = await self.read_smokeco_attribute_expect_success(attribute=attributes.ExpressedState)
            asserts.assert_not_equal(expressed_state_dut, 9, "ExpressedState should not be Inoperative")

        # Step 6, "TH prompts operator to unmount the device."
        self.step(6)

        if has_unmounted:
            if self.is_ci:
                # CI call to trigger unmounted.
                self.write_to_app_pipe({"Name": "SetUnmounted", "EndpointId": endpoint, "Unmounted": 1})
            else:
                self.wait_for_user_input(
                    prompt_msg="Unmount DUT and press enter")

        # Step 7, "TH waits for a report of Unmounted attribute from DUT with a timeout of 60 seconds."
        self.step(7)
        if has_unmounted:
            sub_handler.wait_for_attribute_report(timeout_sec=60)
            asserts.assert_equal(sub_handler.attribute_reports[cluster.Attributes.Unmounted]
                                 [0].value, 1, msg="Received unexpected value for Unmounted")
            sub_handler.reset()

        # Step 8, "TH reads ExpressedState attribute from DUT."
        self.step(8)
        if has_expressed_state and has_unmounted and inoperative_when_unmounted_supported:
            expressed_state_dut = await self.read_smokeco_attribute_expect_success(attribute=attributes.ExpressedState)
            asserts.assert_equal(expressed_state_dut, 9, "ExpressedState should be Inoperative")

        # Step 9, "TH prompts operator to mount the device."
        self.step(9)
        if has_unmounted:
            if self.is_ci:
                # CI call to trigger mounted.
                self.write_to_app_pipe({"Name": "SetUnmounted", "EndpointId": endpoint, "Unmounted": 0})
            else:
                self.wait_for_user_input(
                    prompt_msg="Mount DUT and press enter")

        # Step 10, "TH waits for a report of Unmounted attribute from DUT with a timeout of 60 seconds."
        self.step(10)
        if has_unmounted:
            sub_handler.wait_for_attribute_report(timeout_sec=60)
            asserts.assert_equal(sub_handler.attribute_reports[cluster.Attributes.Unmounted]
                                 [0].value, 0, msg="Received unexpected value for Unmounted")

        # Step 11, "TH reads ExpressedState attribute from DUT."
        self.step(11)
        if has_expressed_state and has_unmounted and inoperative_when_unmounted_supported:
            expressed_state_dut = await self.read_smokeco_attribute_expect_success(attribute=attributes.ExpressedState)
            asserts.assert_not_equal(expressed_state_dut, 9, "ExpressedState should not be Inoperative")


if __name__ == "__main__":
    default_matter_test_main()
