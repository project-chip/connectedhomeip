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
#     app-args: --discriminator 1234 --KVS /tmp/kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --app-pipe /tmp/smokeco_2_5_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.INTERCONNECTSMOKEALARM:005c000000000092
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.INTERCONNECTSMOKEALARM.CLEAR:005c0000000000a2
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.INTERCONNECTCOALARM:005c000000000094
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.INTERCONNECTCOALARM.CLEAR:005c0000000000a4
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CONTAMINATIONSTATE.HIGH:005c000000000096
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CONTAMINATIONSTATE.LOW:005c000000000097
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CONTAMINATIONSTATE.CLEAR:005c0000000000a6
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKESENSITIVITY.HIGH:005c000000000098
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKESENSITIVITY.LOW:005c000000000099
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKESENSITIVITY.CLEAR:005c0000000000a8
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKEALARM.WARNING:005c000000000090
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKEALARM.CRITICAL:005c00000000009c
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKEALARM.CLEAR:005c0000000000a0
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.COALARM.WARNING:005c000000000091
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.COALARM.CRITICAL:005c00000000009d
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.COALARM.CLEAR:005c0000000000a1
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.MANUALDEVICEMUTE:005c00000000009b
#       --hex-arg PIXIT.SMOKECO.TEST_EVENT_TRIGGER.MANUALDEVICEMUTE.CLEAR:005c0000000000ab
#       --endpoint 1
#       --app-pipe /tmp/smokeco_2_5_fifo
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging
from mobly import asserts

from TC_SMOKECOTestBase import SmokeCoBaseTest

import matter.clusters as Clusters
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.decorators import async_test_body, has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

class TC_SMOKECO_2_5(SmokeCoBaseTest):

    @async_test_body
    async def setup_test(self):
        super().setup_test()
        self.gd_cluster = Clusters.GeneralDiagnostics
        
        # Interconnect smoke alarm
        self.pixit_test_event_interconnect_smoke_alarm = self.user_params.get(
            "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.INTERCONNECTSMOKEALARM", 0x005c000000000092)
        self.pixit_test_event_interconnect_smoke_alarm_clear = self.user_params.get(
            "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.INTERCONNECTSMOKEALARM.CLEAR", 0x005c0000000000a2)
        
        # Interconnect co alarm
        self.pixit_test_event_interconnect_co_alarm = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.INTERCONNECTCOALARM", 0x005c000000000094)
        self.pixit_test_event_interconnect_co_alarm_clear = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.INTERCONNECTCOALARM.CLEAR", 0x005c0000000000a4)

        # Contamination State
        self.pixit_test_event_contamination_state_low = self.user_params.get(
            "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CONTAMINATIONSTATE.HIGH", 0x005c000000000096)
        self.pixit_test_event_contamination_state_high = self.user_params.get(
            "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CONTAMINATIONSTATE.LOW", 0x005c000000000097)
        self.pixit_test_event_contamination_state_clear = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.CONTAMINATIONSTATE.CLEAR", 0x005c0000000000a6)
        
        # Smoke Sensitivity
        self.pixit_test_event_smokesensitivity_low = self.user_params.get(
            "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKESENSITIVITY.HIGH", 0x005c000000000098)
        self.pixit_test_event_smokesensitivity_high = self.user_params.get(
            "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKESENSITIVITY.LOW", 0x005c000000000099)
        self.pixit_test_event_smokesensitivity_clear = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKESENSITIVITY.CLEAR", 0x005c0000000000a8)
        
        # SmokeAlarm
        self.pixit_test_event_warning_smoke_alarm = self.user_params.get(
            "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKEALARM.WARNING", 0x005c000000000090)
        self.pixit_test_event_critical_smoke_alarm = self.user_params.get(
            "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKEALARM.CRITICAL", 0x005c00000000009c)
        self.pixit_test_event_clear_smoke_alarm = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.SMOKEALARM.CLEAR", 0x005c0000000000a0)

        # Smoke CO
        self.pixit_test_event_warning_co_alarm = self.user_params.get(
            "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.COALARM.WARNING", 0x005c0000000000a0)
        self.pixit_test_event_critical_co_alarm = self.user_params.get(
            "PIXIT.SMOKECO.TEST_EVENT_TRIGGER.COALARM.CRITICAL", 0x005c00000000009d)
        self.pixit_test_event_clear_co_alarm = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.COALARM.CLEAR", 0x005c0000000000a1)
                                                                    
        # Manual Device Mute
        self.pixit_test_event_manual_device_mute = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.MANUALDEVICEMUTE", 0x005c00000000009b)
        self.pixit_test_event_manual_device_mute_clear = self.user_params.get("PIXIT.SMOKECO.TEST_EVENT_TRIGGER.MANUALDEVICEMUTE.CLEAR", 0x005c0000000000ab)

        # Process pixit arguments and convert from bytes if needed
        self.process_pixit_attributes()
        

    def desc_TC_SMOKECO_2_5(self) -> str:
        return "[TC-SMOKECO-2.5] Secondary Functionality - Optional with DUT as Server"

    def steps_TC_SMOKECO_2_5(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH"),
            TestStep(2, "TH subscribes to InterconnectSmokeAlarm attribute from DUT", "Verify that InterconnectSmokeAlarm attribute has a value of 0 (Normal)"),
            TestStep(3, "TH reads ExpressedState attribute from DUT", "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(4, "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster", "Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep(5, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Interconnect Smoke Alarm Test Event"),
            TestStep(6, "TH waits for a report of InterconnectSmokeAlarm attribute from DUT with a timeout of 300 seconds", "Verify that InterconnectSmokeAlarm attribute has a value of 2 (Critical) or 1 (Warning)"),
            TestStep(7, "TH reads InterconnectSmokeAlarm event from DUT", "Verify that InterconnectSmokeAlarm event has a new event record and AlarmSeverityLevel field indicates the current value of InterconnectSmokeAlarm attribute"),
            TestStep(8, "TH reads ExpressedState attribute from DUT", "Verify that ExpressedState attribute has a value of 7 (InterconnectSmoke)"),
            TestStep(9, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Interconnect Smoke Alarm Test Event Clear"),
            TestStep(10, "TH waits for a report of InterconnectSmokeAlarm attribute from DUT with a timeout of 300 seconds", "Verify that InterconnectSmokeAlarm attribute has a value of 0 (Normal)"),
            TestStep(11, "TH reads ExpressedState attribute from DUT", "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(12, "TH reads AllClear event from DUT", "Verify that AllClear event has a new event record"),
            TestStep(13, "TH subscribes to InterconnectCOAlarm attribute from DUT", "Verify that InterconnectCOAlarm attribute has a value of 0 (Normal)"),
            TestStep(14, "TH reads ExpressedState attribute from DUT", "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(15, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Interconnect CO Alarm Test Event"),
            TestStep(16, "TH waits for a report of InterconnectCOAlarm attribute from DUT with a timeout of 300 seconds", "Verify that InterconnectCOAlarm attribute has a value of 2 (Critical) or 1 (Warning)"),
            TestStep(17, "TH reads InterconnectCOAlarm event from DUT", "Verify that InterconnectCOAlarm event has a new event record and AlarmSeverityLevel field indicates the current value of InterconnectCOAlarm attribute"),
            TestStep(18, "TH reads ExpressedState attribute from DUT", "Verify that ExpressedState attribute has a value of 8 (InterconnectCO)"),
            TestStep(19, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Interconnect CO Alarm Test Event Clear"),
            TestStep(20, "TH waits for a report of InterconnectCOAlarm attribute from DUT with a timeout of 300 seconds", "Verify that InterconnectCOAlarm attribute has a value of 0 (Normal)"),
            TestStep(21, "TH reads ExpressedState attribute from DUT", "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(22, "TH reads AllClear event from DUT", "Verify that AllClear event has a new event record"),
            TestStep(23, "TH subscribes to ContaminationState attribute from DUT", "Verify that ContaminationState attribute has a value of 0 (Normal)"),
            TestStep(24, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Contamination State (High) Test Event"),
            TestStep(25, "TH waits for a report of ContaminationState attribute from DUT with a timeout of 300 seconds", "Verify that ContaminationState attribute has a value of 2 (Warning) or 3 (Critical)"),
            TestStep(26, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Contamination State Test Event Clear"),
            TestStep(27, "TH waits for a report of ContaminationState attribute from DUT with a timeout of 300 seconds", "Verify that ContaminationState attribute has a value of 0 (Normal)"),
            TestStep(28, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Contamination State (Low) Test Event"),
            TestStep(29, "TH waits for a report of ContaminationState attribute from DUT with a timeout of 300 seconds", "Verify that ContaminationState attribute has a value of 1 (Low)"),
            TestStep(30, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Contamination State Test Event Clear"),
            TestStep(31, "TH waits for a report of ContaminationState attribute from DUT with a timeout of 300 seconds", "Verify that ContaminationState attribute has a value of 0 (Normal)"),
            TestStep(32, "TH subscribes to SmokeSensitivityLevel attribute from DUT", "Verify that SmokeSensitivityLevel attribute has a value of 1 (Standard)"),
            TestStep(33, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Smoke Sensitivity Level (High) Test Event"),
            TestStep(34, "TH waits for a report of SmokeSensitivityLevel attribute from DUT with a timeout of 300 seconds", "Verify that SmokeSensitivityLevel attribute has a value of 0 (High)"),
            TestStep(35, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Smoke Sensitivity Level Test Event Clear"),
            TestStep(36, "TH waits for a report of SmokeSensitivityLevel attribute from DUT with a timeout of 300 seconds", "Verify that SmokeSensitivityLevel attribute has a value of 1 (Standard)"),
            TestStep(37, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Smoke Sensitivity Level (Low) Test Event"),
            TestStep(38, "TH waits for a report of SmokeSensitivityLevel attribute from DUT with a timeout of 300 seconds", "Verify that SmokeSensitivityLevel attribute has a value of 2 (Low)"),
            TestStep(39, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Smoke Sensitivity Level Test Event Clear"),
            TestStep(40, "TH waits for a report of SmokeSensitivityLevel attribute from DUT with a timeout of 300 seconds", "Verify that SmokeSensitivityLevel attribute has a value of 1 (Standard)"),
            TestStep(41, "TH subscribes to DeviceMuted attribute from DUT", "Verify that DeviceMuted attribute has a value of 0 (NotMuted)"),
            TestStep(42, "TH reads FeatureMap attribute from DUT", "Verify that Smoke Alarm feature is supported"),
            TestStep(43, "TH subscribes to SmokeState attribute from DUT", "Verify that SmokeState attribute has a value of 0 (Normal)"),
            TestStep(44, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Warning Smoke Alarm Test Event"),
            TestStep(45, "TH waits for a report of SmokeState attribute from DUT with a timeout of 300 seconds", "Verify that SmokeState attribute has a value of 1 (Warning)"),
            TestStep(46, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Manual Device Mute Test Event"),
            TestStep(47, "TH waits for a report of DeviceMuted attribute from DUT with a timeout of 120 seconds", "Verify that DeviceMuted attribute has a value of 1 (Muted)"),
            TestStep(48, "TH reads AlarmMuted event from DUT", "Verify that AlarmMuted event has a new record"),
            TestStep(49, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Manual Device Mute Test Event Clear"),
            TestStep(50, "TH waits for a report of DeviceMuted attribute from DUT with a timeout of 120 seconds", "Verify that DeviceMuted attribute has a value of 0 (NotMuted)"),
            TestStep(51, "TH reads MuteEnded event from DUT", "Verify that MuteEnded event has a new record"),
            TestStep(52, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Critical Smoke Alarm Test Event"),
            TestStep(53, "TH waits for a report of SmokeState attribute from DUT with a timeout of 300 seconds", "Verify that SmokeState attribute has a value of 2 (Critical)"),
            TestStep(54, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Manual Device Mute Test Event"),
            TestStep(55, "TH waits 5 seconds and then reads DeviceMuted attribute from DUT", "Verify that DeviceMuted attribute has a value of 0 (NotMuted)"),
            TestStep(56, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Smoke Alarm Test Event Clear"),
            TestStep(57, "TH waits for a report of SmokeState attribute from DUT with a timeout of 300 seconds", "Verify that SmokeState attribute has a value of 0 (Normal)"),
            TestStep(58, "TH reads FeatureMap attribute from DUT", "Verify that CO Alarm feature is supported"),
            TestStep(59, "TH subscribes to COState attribute from DUT", "Verify that COState attribute has a value of 0 (Normal)"),
            TestStep(60, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Warning CO Alarm Test Event"),
            TestStep(61, "TH waits for a report of COState attribute from DUT with a timeout of 300 seconds", "Verify that COState attribute has a value of 1 (Warning)"),
            TestStep(62, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Manual Device Mute Test Event"),
            TestStep(63, "TH waits for a report of DeviceMuted attribute from DUT with a timeout of 120 seconds", "Verify that DeviceMuted attribute has a value of 1 (Muted)"),
            TestStep(64, "TH reads AlarmMuted event from DUT", "Verify that AlarmMuted event has a new record"),
            TestStep(65, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Manual Device Mute Test Event Clear"),
            TestStep(66, "TH waits for a report of DeviceMuted attribute from DUT with a timeout of 120 seconds", "Verify that DeviceMuted attribute has a value of 0 (NotMuted)"),
            TestStep(67, "TH reads MuteEnded event from DUT", "Verify that MuteEnded event has a new record"),
            TestStep(68, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Critical CO Alarm Test Event"),
            TestStep(69, "TH waits for a report of COState attribute from DUT with a timeout of 300 seconds", "Verify that COState attribute has a value of 2 (Critical)"),
            TestStep(70, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Manual Device Mute Test Event"),
            TestStep(71, "TH waits 5 seconds and then reads DeviceMuted attribute from DUT", "Verify that DeviceMuted attribute has a value of 0 (NotMuted)"),
            TestStep(72, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for CO Alarm Test Event Clear"),
            TestStep(73, "TH waits for a report of COState attribute from DUT with a timeout of 300 seconds", "Verify that COState attribute has a value of 0 (Normal)")
        ]

    def pics_TC_SMOKECO_2_5(self) -> list[str]:
        return [
            "SMOKECO.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.SmokeCoAlarm))
    async def test_TC_SMOKECO_2_5(self):
        
        # Commissioning already done
        self.step(1)

        self.step(2)
        # Interconnected Smoke Alarm
        interconnected_smoke_alarm_handler = AttributeSubscriptionHandler(
            expected_cluster=self.smokeco_cluster, expected_attribute=self.smokeco_cluster.Attributes.InterconnectSmokeAlarm)
        await interconnected_smoke_alarm_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)
        interconnected_smoke_alarm = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.InterconnectSmokeAlarm)
        asserts.assert_equal(interconnected_smoke_alarm,0)

        self.step(3)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state,self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        self.step(4)
        test_event_triggers_enabled = await self.read_general_diagnostics_test_event_triggers_enabled()
        asserts.assert_equal(test_event_triggers_enabled,1)

        # self.step(5)
        # await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_interconnect_smoke_alarm)

        # self.step(6)
        # interconnected_smoke_alarm_report_data = interconnected_smoke_alarm_handler.wait_for_attribute_report(timeout_sec=300)
        # asserts.assert_in(interconnected_smoke_alarm_report_data.value,[self.smokeco_cluster.Enums.AlarmStateEnum.kWarning,self.smokeco_cluster.Enums.AlarmStateEnum.kCritical])

        # self.step(7)
        # interconnected_smoke_alarm_event_data = await self.read_smokeco_event(self.smokeco_cluster.Events.InterconnectSmokeAlarm)
        # asserts.assert_equal(interconnected_smoke_alarm_event_data.alarmSeverityLevel,interconnected_smoke_alarm_report_data.value)

        # self.step(8)
        # expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        # asserts.assert_equal(expressed_state,self.smokeco_cluster.Enums.ExpressedStateEnum.kInterconnectSmoke)

        await self.assert_steps_event_trigger_actions(
            steps=[5,6,7,8],
            pixit_event_trigger=self.pixit_test_event_interconnect_smoke_alarm,
            smoke_report_handler=interconnected_smoke_alarm_handler,
            smoke_event=self.smokeco_cluster.Events.InterconnectSmokeAlarm,
            expected_report_data=[self.smokeco_cluster.Enums.INkWarning,self.smokeco_cluster.Enums.AlarmStateEnum.kCritical],
            expected_event_data="REPORT_DATA",
            expected_expressed_state=self.smokeco_cluster.Enums.ExpressedStateEnum.kInterconnectSmoke,
        )

        self.step(9)
        # await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_interconnect_smoke_alarm_clear)

        # self.step(10)
        # interconnected_smoke_alarm_report_data = interconnected_smoke_alarm_handler.wait_for_attribute_report(timeout_sec=300)
        # asserts.assert_equal(interconnected_smoke_alarm_report_data.value,self.smokeco_cluster.Enums.AlarmStateEnum.kNormal)

        # self.step(11)
        # expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        # asserts.assert_equal(expressed_state,self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        # self.step(12)
        # await self.read_smokeco_event(self.smokeco_cluster.Events.AllClear)
        await self.assert_steps_event_trigger_actions(
            steps=[15,16,17,18],
            pixit_event_trigger=self.pixit_test_event_interconnect_smoke_alarm_clear,
            smoke_report_handler=interconnected_smoke_alarm_handler,
            smoke_event=self.smokeco_cluster.Events.AllClear,
            expected_report_data=[self.smokeco_cluster.Enums.AlarmStateEnum.kWarning,self.smokeco_cluster.Enums.AlarmStateEnum.kNormal],
            expected_event_data="CLEAR_DATA",
            expected_expressed_state=self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal,
        )

        interconnected_smoke_alarm_handler.cancel()

        # Interconnected CO Alarm
        self.step(13)
        interconnected_co_alarm_handler = AttributeSubscriptionHandler(
            expected_cluster=self.smokeco_cluster, expected_attribute=self.smokeco_cluster.Attributes.InterconnectCOAlarm)
        await interconnected_co_alarm_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)
        interconnected_co_alarm = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.InterconnectCOAlarm)
        asserts.assert_equal(interconnected_co_alarm,0)

        self.step(14)
        expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state,self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        # self.step(15)
        # await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_interconnect_co_alarm)

        # self.step(16)
        # interconnected_co_alarm_report_data = interconnected_co_alarm_handler.wait_for_attribute_report(timeout_sec=300)
        # asserts.assert_in(interconnected_co_alarm_report_data.value,[self.smokeco_cluster.Enums.AlarmStateEnum.kWarning,self.smokeco_cluster.Enums.AlarmStateEnum.kCritical])

        # self.step(17)
        # interconnected_co_alarm_event_data = await self.read_smokeco_event(self.smokeco_cluster.Events.InterconnectCOAlarm)
        # asserts.assert_equal(interconnected_co_alarm_event_data.alarmSeverityLevel,interconnected_co_alarm_report_data.value)

        # self.step(18)
        # expressed_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ExpressedState)
        # asserts.assert_equal(expressed_state,self.smokeco_cluster.Enums.ExpressedStateEnum.kInterconnectCO)
        
        await self.assert_steps_event_trigger_actions(
            steps=[15,16,17,18],
            pixit_event_trigger=self.pixit_test_event_interconnect_co_alarm,
            smoke_report_handler=interconnected_co_alarm_handler,
            smoke_event=self.smokeco_cluster.Events.InterconnectCOAlarm,
            expected_report_data=[self.smokeco_cluster.Enums.AlarmStateEnum.kWarning,self.smokeco_cluster.Enums.AlarmStateEnum.kCritical],
            expected_event_data="REPORT_DATA",
            expected_expressed_state=self.smokeco_cluster.Enums.ExpressedStateEnum.kInterconnectCO,
        )

        # self.step(19)
        # await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_interconnect_co_alarm_clear)

        # self.step(20)
        # interconnected_co_alarm_report_data = interconnected_co_alarm_handler.wait_for_attribute_report(timeout_sec=300)
        # asserts.assert_equal(interconnected_co_alarm_report_data.value,self.smokeco_cluster.Enums.AlarmStateEnum.kNormal)

        # self.step(21)

        # self.step(22)

        await self.assert_steps_event_trigger_actions(
            steps=[19,20,21,22],
            pixit_event_trigger=self.pixit_test_event_interconnect_co_alarm_clear,
            smoke_report_handler=interconnected_co_alarm_handler,
            smoke_event=self.smokeco_cluster.Events.AllClear,
            expected_report_data=self.smokeco_cluster.Enums.AlarmStateEnum.kNormal,
            expected_event_data="CLEAR_DATA",
            expected_expressed_state=self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal,
        )
        # Cancel Attributesubcription 
        interconnected_co_alarm_handler.cancel()

        self.step(23)
        contamination_state_handler = AttributeSubscriptionHandler(
            expected_cluster=self.smokeco_cluster, expected_attribute=self.smokeco_cluster.Attributes.ContaminationState)
        await contamination_state_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)
        contamination_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.ContaminationState)
        asserts.assert_equal(contamination_state,0)

        #self.step(24)
        #await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_contamination_state_high)

        #self.step(25)
        #contamination_state_report_data = contamination_state_handler.wait_for_attribute_report(timeout_sec=300)
        await self.assert_steps_event_trigger_report_actions(
            steps=[24,25],
            pixit_event_trigger=self.pixit_test_event_contamination_state_high,
            smoke_report_handler=contamination_state_handler,
            expected_report_data=[self.smokeco_cluster.Enums.ContaminationStateEnum.kWarning,self.smokeco_cluster.Enums.ContaminationStateEnum.kCritical]
            )

        # self.step(26)

        # self.step(27)
        await self.assert_steps_event_trigger_report_actions(
            steps=[26,27],
            pixit_event_trigger=self.pixit_test_event_contamination_state_clear,
            smoke_report_handler=contamination_state_handler,
            expected_report_data=self.smokeco_cluster.Enums.ContaminationStateEnum.kNormal
            )

        await self.assert_steps_event_trigger_report_actions(
            steps=[28,29],
            pixit_event_trigger=self.pixit_test_event_contamination_state_low,
            smoke_report_handler=contamination_state_handler,
            expected_report_data=self.smokeco_cluster.Enums.ContaminationStateEnum.kLow
            )

        await self.assert_steps_event_trigger_report_actions(
            steps=[30,31],
            pixit_event_trigger=self.pixit_test_event_contamination_state_clear,
            smoke_report_handler=contamination_state_handler,
            expected_report_data=self.smokeco_cluster.Enums.ContaminationStateEnum.kNormal
            )
        contamination_state_handler.cancel()

        self.step(32)
        smokesensitivity_handler = AttributeSubscriptionHandler(
            expected_cluster=self.smokeco_cluster, expected_attribute=self.smokeco_cluster.Attributes.SmokeSensitivityLevel)
        await smokesensitivity_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)
        smokesensitivity_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.SmokeSensitivityLevel)
        asserts.assert_equal(smokesensitivity_state,1)

        await self.assert_steps_event_trigger_report_actions(
            steps=[33,34],
            pixit_event_trigger=self.pixit_test_event_smokesensitivity_high,
            smoke_report_handler=smokesensitivity_handler,
            expected_report_data=self.smokeco_cluster.Enums.SensitivityEnum.kHigh
            )

        await self.assert_steps_event_trigger_report_actions(
            steps=[35,36],
            pixit_event_trigger=self.pixit_test_event_smokesensitivity_clear,
            smoke_report_handler=smokesensitivity_handler,
            expected_report_data=self.smokeco_cluster.Enums.SensitivityEnum.kStandard
            )
        
        await self.assert_steps_event_trigger_report_actions(
            steps=[37,38],
            pixit_event_trigger=self.pixit_test_event_smokesensitivity_low,
            smoke_report_handler=smokesensitivity_handler,
            expected_report_data=self.smokeco_cluster.Enums.SensitivityEnum.kLow
            )

        await self.assert_steps_event_trigger_report_actions(
            steps=[39,40],
            pixit_event_trigger=self.pixit_test_event_smokesensitivity_clear,
            smoke_report_handler=smokesensitivity_handler,
            expected_report_data=self.smokeco_cluster.Enums.SensitivityEnum.kStandard
            )
        smokesensitivity_handler.cancel()

        self.step(41)
        device_muted_handler = AttributeSubscriptionHandler(
            expected_cluster=self.smokeco_cluster, expected_attribute=self.smokeco_cluster.Attributes.DeviceMuted)
        await device_muted_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint(), max_interval_sec=30)
        device_muted_state = await self.read_smokeco_attribute_expect_success(self.smokeco_cluster.Attributes.DeviceMuted)
        asserts.assert_equal(device_muted_state,0)
        
        # From step 41 to step 57 is SmokeAlarm
        if await self.feature_guard(endpoint=self.get_endpoint(),cluster=self.smokeco_cluster,feature_int=self.smokeco_cluster.Bitmaps.Feature.kSmokeAlarm):
            pass
        else:
            pass


        self.step(40)

        self.step(41)

        self.step(42)

        self.step(43)

        self.step(44)

        self.step(45)

        self.step(46)

        self.step(47)

        self.step(48)

        self.step(49)

        self.step(50)

        self.step(51)

        self.step(52)

        self.step(53)

        self.step(54)

        self.step(55)

        self.step(56)

        self.step(57)

        self.step(58)
        # From here to step 73 is COAlarm
        if await self.feature_guard(endpoint=self.get_endpoint(),cluster=self.smokeco_cluster,feature_int=self.smokeco_cluster.Bitmaps.Feature.kCoAlarm):
            pass
        else:
            pass

        self.step(59)

        self.step(60)

        self.step(61)

        self.step(62)

        self.step(63)

        self.step(64)

        self.step(65)

        self.step(66)

        self.step(67)

        self.step(68)

        self.step(69)

        self.step(70)

        self.step(71)

        self.step(72)

        self.step(73)



if __name__ == "__main__":
    default_matter_test_main()
