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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --app-pipe /tmp/smokeco_2_4_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --app-pipe /tmp/smokeco_2_4_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

from mobly import asserts
from support_modules.smokeco_support import SmokeCoBaseTest

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_SMOKECO_2_4(SmokeCoBaseTest):

    @async_test_body
    async def setup_test(self):
        super().setup_test()
        self.gd_cluster = Clusters.GeneralDiagnostics

    def desc_TC_SMOKECO_2_4(self) -> str:
        return "[TC-SMOKECO-2.4] Secondary Functionality - Mandatory with DUT as Server"

    def steps_TC_SMOKECO_2_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH"),
            TestStep(2, "TH subscribes to BatteryAlert attribute from DUT",
                     "Verify that BatteryAlert attribute has a value of 0 (Normal)"),
            TestStep(3, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(4, "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep(5, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Warning Battery Alert Test Event"),
            TestStep(6, "TH waits for a report of BatteryAlert attribute from DUT with a timeout of 300 seconds",
                     "Verify that BatteryAlert attribute has a value of 1 (Warning)"),
            TestStep(7, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 3 (BatteryAlert)"),
            TestStep(8, "TH reads LowBattery event from DUT",
                     "Verify that LowBattery event has a new event record and alarmSeverityLevel field indicates the BatteryAlert attribute current value of 1"),
            TestStep(9, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Critical Battery Alert Test Event"),
            TestStep(10, "TH waits for a report of BatteryAlert attribute from DUT with a timeout of 300 seconds",
                     "Verify that BatteryAlert attribute has a value of 2 (Critical)"),
            TestStep(11, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 3 (BatteryAlert)"),
            TestStep(12, "TH reads LowBattery event from DUT",
                     "Verify that LowBattery event has a new event record and alarmSeverityLevel field indicates the BatteryAlert attribute current value of 2"),
            TestStep(13, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Battery Alert Test Event Clear"),
            TestStep(14, "TH waits for a report of BatteryAlert attribute from DUT with a timeout of 300 seconds",
                     "Verify that BatteryAlert attribute has a value of 0 (Normal)"),
            TestStep(15, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(16, "TH reads AllClear event from DUT", "Verify that AllClear event has a new event record"),
            TestStep(17, "TH subscribes to HardwareFaultAlert attribute from DUT",
                     "Verify that HardwareFaultAlert attribute has a value of 0 (False)"),
            TestStep(18, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Hardware Fault Alert Test Event"),
            TestStep(19, "TH waits for a report of HardwareFaultAlert attribute from DUT with a timeout of 300 seconds",
                     "Verify that HardwareFaultAlert attribute has a value of 1 (True)"),
            TestStep(20, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 5 (HardwareFault)"),
            TestStep(21, "TH reads HardwareFault event from DUT", "Verify that HardwareFault event has a new event record"),
            TestStep(22, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Hardware Fault Alert Test Event Clear"),
            TestStep(23, "TH waits for a report of HardwareFaultAlert attribute from DUT with a timeout of 300 seconds",
                     "Verify that HardwareFaultAlert attribute has a value of 0 (False)"),
            TestStep(24, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(25, "TH reads AllClear event from DUT", "Verify that AllClear event has a new event record"),
            TestStep(26, "TH subscribes to EndOfServiceAlert attribute from DUT",
                     "Verify that EndOfServiceAlert attribute has a value of 0 (Normal)"),
            TestStep(27, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for End of Service Alert Test Event"),
            TestStep(28, "TH waits for a report of EndOfServiceAlert attribute from DUT with a timeout of 300 seconds",
                     "Verify that EndOfServiceAlert attribute has a value of 1 (Expired)"),
            TestStep(29, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 6 (EndOfService)"),
            TestStep(30, "TH reads EndOfService event from DUT", "Verify that EndOfService event has a new event record"),
            TestStep(31, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for End of Service Alert Test Event Clear"),
            TestStep(32, "TH waits for a report of EndOfServiceAlert attribute from DUT with a timeout of 300 seconds",
                     "Verify that EndOfServiceAlert attribute has a value of 0 (Normal)"),
            TestStep(33, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(34, "TH reads AllClear event from DUT", "Verify that AllClear event has a new event record"),
            TestStep(35, "TH subscribes to TestInProgress attribute from DUT",
                     "Verify that TestInProgress attribute has a value of 0 (False)"),
            TestStep(36, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(37, "Start manually DUT self-test"),
            TestStep(38, "TH waits for a report of TestInProgress attribute from DUT with a timeout of 180 seconds",
                     "Verify that TestInProgress attribute has a value of 1 (True)"),
            TestStep(39, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 4 (Testing)"),
            TestStep(40, "TH waits for a report of TestInProgress attribute from DUT with a timeout of 180 seconds",
                     "Verify that TestInProgress attribute has a value of 0 (False)"),
            TestStep(41, "TH reads SelfTestComplete event from DUT", "Verify that SelfTestComplete event has a new event record"),
            TestStep(42, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(43, "TH reads AllClear event from DUT", "Verify that AllClear event has a new event record"),
            TestStep(44, "TH sends SelfTestRequest command to DUT"),
            TestStep(45, "TH waits for a report of TestInProgress attribute from DUT with a timeout of 180 seconds",
                     "Verify that TestInProgress attribute has a value of 1 (True)"),
            TestStep(46, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 4 (Testing)"),
            TestStep(47, "TH waits for a report of TestInProgress attribute from DUT with a timeout of 180 seconds",
                     "Verify that TestInProgress attribute has a value of 0 (False)"),
            TestStep(48, "TH reads SelfTestComplete event from DUT", "Verify that SelfTestComplete event has a new event record"),
            TestStep(49, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(50, "TH reads AllClear event from DUT", "Verify that AllClear event has a new event record"),
        ]

    def pics_TC_SMOKECO_2_4(self) -> list[str]:
        return [
            "SMOKECO.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.SmokeCoAlarm))
    async def test_TC_SMOKECO_2_4(self):

        # Commission device already done
        self.step(1)

        self.step(2)
        # Create Attribute Subscription
        battery_alert_handler = AttributeSubscriptionHandler(
            expected_cluster=self.smokeco_cluster, expected_attribute=self.smokeco_cluster.Attributes.BatteryAlert)
        await battery_alert_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)
        battery_alert = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.BatteryAlert)
        asserts.assert_equal(battery_alert, self.smokeco_cluster.Enums.AlarmStateEnum.kNormal)

        self.step(3)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(4)
        test_event_triggers_enabled = await self.read_general_diagnostics_test_event_triggers_enabled()
        asserts.assert_true(test_event_triggers_enabled, "Event triggers are not enabled on General Diagnostics Cluster")

        # Start block BatteryWarning
        self.step(5)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_battery_warning)

        self.step(6)
        battery_alert_report = battery_alert_handler.wait_for_attribute_report(timeout_sec=300)
        log.info(f"Battery alert report {battery_alert_report} with value {battery_alert_report.value}")
        asserts.assert_equal(battery_alert_report.value, self.smokeco_cluster.Enums.AlarmStateEnum.kWarning)

        self.step(7)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kBatteryAlert)

        self.step(8)
        low_battery_event_data = await self.read_smokeco_event(smokeco_event=self.smokeco_cluster.Events.LowBattery)
        asserts.assert_equal(low_battery_event_data.alarmSeverityLevel, self.smokeco_cluster.Enums.AlarmStateEnum.kWarning)
        # End block BatterWarning

        # Start block BatteryCritical
        self.step(9)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_battery_critical)

        self.step(10)
        battery_alert_report = battery_alert_handler.wait_for_attribute_report(timeout_sec=300)
        log.info(f"Smoke state report {battery_alert_report} with value {battery_alert_report.value}")
        asserts.assert_equal(battery_alert_report.value, self.smokeco_cluster.Enums.AlarmStateEnum.kCritical)

        self.step(11)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kBatteryAlert)

        self.step(12)
        low_battery_event_data = await self.read_smokeco_event(self.smokeco_cluster.Events.LowBattery)
        asserts.assert_equal(low_battery_event_data.alarmSeverityLevel, self.smokeco_cluster.Enums.AlarmStateEnum.kCritical)

        # End block BatteryCritical

        # Start block BatteryClear
        self.step(13)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_battery_clear)

        self.step(14)
        battery_alert_report = battery_alert_handler.wait_for_attribute_report(timeout_sec=300)
        log.info(f"Battery Alert Report {battery_alert_report} with value {battery_alert_report.value}")
        asserts.assert_equal(battery_alert_report.value, self.smokeco_cluster.Enums.AlarmStateEnum.kNormal)

        self.step(15)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(16)
        # Only reads and verify has a new event
        await self.read_smokeco_event(self.smokeco_cluster.Events.AllClear)
        # Cancel the attribute report
        battery_alert_handler.cancel()
        # End block BatteryClear

        # Start HardwareFault
        # Subscribe to HardwareFault
        self.step(17)
        hardware_fault_handler = AttributeSubscriptionHandler(
            expected_cluster=self.smokeco_cluster, expected_attribute=self.smokeco_cluster.Attributes.HardwareFaultAlert)
        await hardware_fault_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)
        hardware_fault = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.HardwareFaultAlert)
        asserts.assert_equal(hardware_fault, False)

        self.step(18)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_hardware_alert)

        self.step(19)
        hardware_fault_report = hardware_fault_handler.wait_for_attribute_report(timeout_sec=300)
        log.info(f"Hardware Fault report {hardware_fault_report} with value {hardware_fault_report.value}")
        asserts.assert_equal(hardware_fault_report.value, True)

        self.step(20)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kHardwareFault)

        self.step(21)
        await self.read_smokeco_event(self.smokeco_cluster.Events.HardwareFault)
        # End HardwareFault

        # Clear HardwareFault alert
        self.step(22)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_hardware_clear)

        self.step(23)
        hardware_fault_report = hardware_fault_handler.wait_for_attribute_report(timeout_sec=300)
        log.info(f"Smoke state report {hardware_fault_report} with value {hardware_fault_report.value}")
        asserts.assert_equal(hardware_fault_report.value, 0)

        self.step(24)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(25)
        await self.read_smokeco_event(self.smokeco_cluster.Events.AllClear)
        # Clear the subscription
        hardware_fault_handler.cancel()
        # End clear HardwareFault

        # Start EndOfService check
        self.step(26)
        end_of_service_alert_handler = AttributeSubscriptionHandler(
            expected_cluster=self.smokeco_cluster, expected_attribute=self.smokeco_cluster.Attributes.EndOfServiceAlert)
        await end_of_service_alert_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)
        end_of_service_attr = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.EndOfServiceAlert)
        asserts.assert_equal(end_of_service_attr, self.smokeco_cluster.Enums.EndOfServiceEnum.kNormal)

        self.step(27)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_service_alert)

        self.step(28)
        end_of_service_report = end_of_service_alert_handler.wait_for_attribute_report(timeout_sec=300)
        log.info(f"End of service alert report {end_of_service_report} with value {end_of_service_report.value}")
        asserts.assert_equal(end_of_service_report.value, self.smokeco_cluster.Enums.EndOfServiceEnum.kExpired)

        self.step(29)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kEndOfService)

        self.step(30)
        await self.read_smokeco_event(self.smokeco_cluster.Events.EndOfService)
        # End of EndOfService check

        # Start of Clear EndOfService
        self.step(31)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_service_clear)

        self.step(32)
        end_of_service_report = end_of_service_alert_handler.wait_for_attribute_report(timeout_sec=300)
        log.info(f"End of service report {hardware_fault_report} with value {end_of_service_report.value}")
        asserts.assert_equal(end_of_service_report.value, self.smokeco_cluster.Enums.EndOfServiceEnum.kNormal)

        self.step(33)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(34)
        await self.read_smokeco_event(self.smokeco_cluster.Events.AllClear)
        end_of_service_alert_handler.cancel()
        # End of Clear EndOfService

        self.step(35)
        test_in_progress_handler = AttributeSubscriptionHandler(
            expected_cluster=self.smokeco_cluster, expected_attribute=self.smokeco_cluster.Attributes.TestInProgress)
        await test_in_progress_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)
        test_in_progress = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.TestInProgress)
        asserts.assert_false(test_in_progress, "Test in progress and should not be in progress.")

        self.step(36)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(37)
        # Start manually DUT self-test
        self.start_device_self_test()

        self.step(38)
        test_in_progress_report = test_in_progress_handler.wait_for_attribute_report(timeout_sec=180)
        log.info(f"Test in progress report {test_in_progress_report} with value {test_in_progress_report.value}")
        asserts.assert_true(test_in_progress_report.value, "Test is not in progresss and should be in progress.")

        self.step(39)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kTesting)

        self.step(40)
        test_in_progress_report = test_in_progress_handler.wait_for_attribute_report(timeout_sec=180)
        log.info(f"Test in progress report {test_in_progress_report} with value {test_in_progress_report.value}")
        asserts.assert_equal(test_in_progress_report.value, False)

        self.step(41)
        await self.read_smokeco_event(self.smokeco_cluster.Events.SelfTestComplete)

        self.step(42)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(43)
        await self.read_smokeco_event(self.smokeco_cluster.Events.AllClear)
        # End of Manual Test

        # Start SelfTestRequest
        self.step(44)
        self_test_cmd = self.smokeco_cluster.Commands.SelfTestRequest()
        await self.send_single_cmd(self_test_cmd, dev_ctrl=self.default_controller, endpoint=self.get_endpoint(), timedRequestTimeoutMs=5000)

        self.step(45)
        test_in_progress_report = test_in_progress_handler.wait_for_attribute_report(timeout_sec=180)
        log.info(f"Test in progress report {test_in_progress_report} with value {test_in_progress_report.value}")
        asserts.assert_true(test_in_progress_report.value, "Test is not in progress and should be in progress.")

        self.step(46)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kTesting)

        self.step(47)
        test_in_progress_report = test_in_progress_handler.wait_for_attribute_report(timeout_sec=180)
        log.info(f"Test in progress report {test_in_progress_report} with value {test_in_progress_report.value}")
        asserts.assert_false(test_in_progress_report.value, "Test should not be in progress and is on progress.")

        self.step(48)
        await self.read_smokeco_event(self.smokeco_cluster.Events.SelfTestComplete)

        self.step(49)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(50)
        await self.read_smokeco_event(self.smokeco_cluster.Events.AllClear)
        test_in_progress_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
