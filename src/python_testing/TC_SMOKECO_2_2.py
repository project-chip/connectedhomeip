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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --app-pipe /tmp/smokeco_2_2_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.WARNING:0x005c000000000090
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CRITICAL:0x005c00000000009c
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CLEAR:0x005c0000000000a0
#       --endpoint 1
#       --app-pipe /tmp/smokeco_2_2_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

from mobly import asserts
from TC_SMOKECOTestBase import SmokeCoBaseTest

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_SMOKECO_2_2(SmokeCoBaseTest):


    @async_test_body
    async def setup_test(self):
        super().setup_test()
        self.gd_cluster = Clusters.GeneralDiagnostics
        # # Verify if General Diagnostics is available on endpoint 0 (Required for this test)
        # try:
        #     await self.read_single_attribute_check_success(cluster=self.gd_cluster,dev_ctrl=self.default_controller,node_id=self.dut_node_id,endpoint=0,attribute=self.gd_cluster.Attributes.UpTime)
        # except Exception: #Interactionmodel
        #     asserts.fail("Unable to read the General Diacgnostic Cluster on endpoint 0")
        # asserts.assert_true('PIXIT.SMOKECO.TEST_EVENT_TRIGGER.WARNING' in self.matter_test_config.global_test_params,
        #                     "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.WARNING must be included on the command line in "
        #                     "the --hex-arg flag as PIXIT.SMOKECO.TEST_EVENT_TRIGGER.WARNING:<endpoint>")
        # asserts.assert_true('PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CRITICAL' in self.matter_test_config.global_test_params,
        #                     "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CRITICAL must be included on the command line in "
        #                     "the --hex-arg flag as PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CRITICAL:<endpoint>")
        # asserts.assert_true('PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CLEAR' in self.matter_test_config.global_test_params,
        #                     "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CLEAR must be included on the command line in "
        #                     "the --hex-arg flag as PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CLEAR:<endpoint>")
        self.pixit_test_event_warning_smoke_alarm = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.WARNING", 0x005c000000000090)
        self.pixit_test_event_critical_smoke_alarm = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CRITICAL", 0x005c00000000009c)
        self.pixit_test_event_clear_smoke_alarm = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CLEAR", 0x005c0000000000a0)


    def desc_TC_SMOKECO_2_2(self) -> str:
        return "[TC-SMOKECO-2.2] Primary Functionality - Smoke Alarm with DUT as Server"

    def steps_TC_SMOKECO_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH"),
            TestStep(2, "TH subscribes to SmokeState attribute from DUT",
                     "Verify that SmokeState attribute has a value of 0 (Normal)"),
            TestStep(3, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(4, "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep(5, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Warning Smoke Alarm Test Event"),
            TestStep(6, "TH waits for a report of SmokeState attribute from DUT with a timeout of 300 seconds",
                     "Verify that SmokeState attribute has a value of 1 (Warning)"),
            TestStep(7, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 1 (SmokeAlarm)"),
            TestStep(8, "TH reads SmokeAlarm event from DUT",
                     "Verify that SmokeAlarm event has a new event record and AlarmSeverityLevel field indicates the SmokeState attribute current value of 1"),
            TestStep(9, "Start manually DUT self-test"),
            TestStep(10, "After a few seconds, TH reads TestInProgress attribute from DUT",
                     "Verify that TestInProgress attribute has a value of 0 (False)"),
            TestStep(11, "TH sends SelfTestRequest command to DUT"),
            TestStep(12, "After a few seconds, TH receives the cluster-specific Status Code from DUT",
                     "Verify that Status Code shows BUSY"),
            TestStep(13, "TH reads TestInProgress attribute from DUT",
                     "Verify that TestInProgress attribute has a value of 0 (False)"),
            TestStep(14, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Critical Smoke Alarm Test Event"),
            TestStep(15, "TH waits for a report of SmokeState attribute from DUT with a timeout of 300 seconds",
                     "Verify that SmokeState attribute has a value of 2 (Critical)"),
            TestStep(16, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 1 (SmokeAlarm)"),
            TestStep(17, "TH reads SmokeAlarm event from DUT",
                     "Verify that SmokeAlarm event has a new event record and AlarmSeverityLevel field indicates the SmokeState attribute current value of 2"),
            TestStep(18, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Smoke Alarm Test Event Clear"),
            TestStep(19, "TH waits for a report of SmokeState attribute from DUT with a timeout of 300 seconds",
                     "Verify that SmokeState attribute has a value of 0 (Normal)"),
            TestStep(20, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(21, "TH reads AllClear event from DUT", "Verify that AllClear event has a new event record"),
        ]

    def pics_TC_SMOKECO_2_2(self) -> list[str]:
        return [
            "SMOKECO.S",
        ]



    @run_if_endpoint_matches(has_cluster(Clusters.SmokeCoAlarm))
    async def test_TC_SMOKECO_2_2(self):

        # Step 1, "Commission DUT to TH."
        self.step(1)  # Commissioning already done

        self.step(2)
        # Create Attribute Subscription
        smoke_state_handler = AttributeSubscriptionHandler(expected_cluster=self.smokeco_cluster, expected_attribute=self.smokeco_cluster.Attributes.SmokeState)
        await smoke_state_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)

        # Read attribute
        smoke_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.SmokeState)
        asserts.assert_equal(smoke_state,self.smokeco_cluster.Enums.AlarmStateEnum.kNormal)

        self.step(3)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state,self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(4)
        # Reads General Diagnostic Cluster
        test_event_trigger_enabled = await self.read_single_attribute_check_success(
            cluster=self.gd_cluster,
            attribute=self.gd_cluster.Attributes.TestEventTriggersEnabled,
            dev_ctrl=self.default_controller,
            endpoint=0)
        asserts.assert_equal(test_event_trigger_enabled,True,"TestEventTriggersEnabled is not True")

        self.step(5)
        # By defalt on endpoint 0
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_warning_smoke_alarm)

        self.step(6)
        smoke_state_report = smoke_state_handler.wait_for_attribute_report(timeout_sec=300)
        log.info(f"Smoke state report {smoke_state_report} with value {smoke_state_report.value}")
        asserts.assert_equal(smoke_state_report.value,self.smokeco_cluster.Enums.AlarmStateEnum.kWarning)

        self.step(7)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state,self.smokeco_cluster.Enums.ExpressedStateEnum.kSmokeAlarm)

        self.step(8)
        smoke_alarm_event_data = await self.read_smokeco_event(self.smokeco_cluster.Events.SmokeAlarm)
        log.info(f"SmokeAlarm Event {smoke_alarm_event_data}")
        asserts.assert_equal(smoke_alarm_event_data.alarmSeverityLevel,self.smokeco_cluster.Enums.AlarmStateEnum.kWarning)

        self.step(9)
        # Manually Start the Seft Test
        if self.is_pics_sdk_ci_only:
            # LongPress will trigger the SelfTest in the SmokeCo cluster
            command_dict = {"Name": "LongPress", "EndpointId": self.get_endpoint(),"NewPosition": 0}
            self.write_to_app_pipe(command_dict=command_dict)
        else:
            self.wait_for_user_input(prompt_msg="Start manually DUT self-test",prompt_msg_placeholder="Enter 'y' when done")

        self.step(10)
        test_in_progress = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.TestInProgress)
        asserts.assert_equal(test_in_progress,False)

        # Gather these steps
        self.step(11)
        self.step(12)
        try:
            self_test_cmd = self.smokeco_cluster.Commands.SelfTestRequest()
            await self.send_single_cmd(self_test_cmd,dev_ctrl=self.default_controller,endpoint=self.get_endpoint(),timedRequestTimeoutMs=5000)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Busy, "Unexpected error returned")

        self.step(13)
        test_in_progress = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.TestInProgress)
        asserts.assert_equal(test_in_progress,False)

        self.step(14)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_critical_smoke_alarm)

        self.step(15)
        smoke_state_report = smoke_state_handler.wait_for_attribute_report(timeout_sec=300)
        asserts.assert_equal(smoke_state_report.value,self.smokeco_cluster.Enums.AlarmStateEnum.kCritical)

        self.step(16)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state,self.smokeco_cluster.Enums.ExpressedStateEnum.kSmokeAlarm)

        self.step(17)
        smoke_alarm_event_data = await self.read_smokeco_event(self.smokeco_cluster.Events.SmokeAlarm)
        # Critical is 2
        asserts.assert_equal(smoke_alarm_event_data.alarmSeverityLevel,self.smokeco_cluster.Enums.AlarmStateEnum.kCritical)

        # Smoke Alarm Test Event Clear
        self.step(18)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_clear_smoke_alarm)

        self.step(19)
        smoke_state_report_clear = smoke_state_handler.wait_for_attribute_report(timeout_sec=300)
        asserts.assert_equal(smoke_state_report_clear.value,self.smokeco_cluster.Enums.AlarmStateEnum.kNormal)
        smoke_state_handler.cancel()

        self.step(20)
        expressed_state_clear = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state_clear,self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(21)
        # This will fail if AllClearEvent is not retrieved
        await self.read_smokeco_event(self.smokeco_cluster.Events.AllClear)


if __name__ == "__main__":
    default_matter_test_main()
