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
#     app-args: --discriminator 1234 --KVS /tmp/kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --app-pipe /tmp/smokeco_2_3_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --app-pipe /tmp/smokeco_2_3_fifo
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

from support_modules.smokeco_support import SmokeCoBaseTest

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body, has_feature, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_SMOKECO_2_3(SmokeCoBaseTest):

    @async_test_body
    async def setup_test(self):
        super().setup_test()
        self.gd_cluster = Clusters.GeneralDiagnostics

    def desc_TC_SMOKECO_2_3(self) -> str:
        return "[TC-SMOKECO-2.3] Primary Functionality - CO Alarm with DUT as Server"

    def steps_TC_SMOKECO_2_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).",
                     "DUT is commissioned to TH.", is_commissioning=True),
            TestStep(2, "TH subscribes to COState attribute from DUT", "Verify that COState attribute has a value of 0 (Normal)"),
            TestStep(3, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(4, "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep(5, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Warning CO Alarm Test Event"),
            TestStep(6, "TH waits for a report of COState attribute from DUT with a timeout of 300 seconds",
                     "Verify that COState attribute has a value of 1 (Warning)"),
            TestStep(7, "SMOKECO.S.A0000(ExpressedState) TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 2 (COAlarm)"),
            TestStep(8, "TH reads COAlarm event from DUT",
                     "Verify that COAlarm event has a new event record and AlarmSeverityLevel field indicates the COState attribute current value of 1"),
            TestStep(9, "Start manually DUT self-test"),
            TestStep(10, "After a few seconds, TH reads TestInProgress attribute from DUT",
                     "Verify that TestInProgress attribute has a value of 0 (False)"),
            TestStep(11, "SMOKECO.S.C00.Rsp(SelfTestRequest) TH sends SelfTestRequest command to DUT"),
            TestStep(12, "After a few seconds, TH receives the cluster-specific Status Code from DUT",
                     "Verify that Status Code shows BUSY"),
            TestStep(13, "TH reads TestInProgress attribute from DUT",
                     "Verify that TestInProgress attribute has a value of 0 (False)"),
            TestStep(14, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Critical CO Alarm Test Event"),
            TestStep(15, "TH waits for a report of COState attribute from DUT with a timeout of 300 seconds",
                     "Verify that COState attribute has a value of 2 (Critical)"),
            TestStep(16, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 2 (COAlarm)"),
            TestStep(17, "TH reads COAlarm event from DUT",
                     "Verify that COAlarm event has a new event record and AlarmSeverityLevel field indicates the COState attribute current value of 2"),
            TestStep(18, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for CO Alarm Test Event Clear"),
            TestStep(19, "TH waits for a report of COState attribute from DUT with a timeout of 300 seconds",
                     "Verify that COState attribute has a value of 0 (Normal)"),
            TestStep(20, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(21, "TH reads AllClear event from DUT", "Verify that AllClear event has a new event record."),
        ]

    def pics_TC_SMOKECO_2_3(self) -> list[str]:
        return [
            "SMOKECO.S",
        ]

    @run_if_endpoint_matches(has_feature(cluster=Clusters.SmokeCoAlarm, feature=Clusters.SmokeCoAlarm.Bitmaps.Feature.kCoAlarm))
    async def test_TC_SMOKECO_2_3(self):

        # Runs the test using the base template
        await self.alarm_primary_functionality_base_test(
            state_attribute=self.smokeco_cluster.Attributes.COState,
            alarm_event=self.smokeco_cluster.Events.COAlarm,
            expressed_state_enum_value=self.smokeco_cluster.Enums.ExpressedStateEnum.kCOAlarm,
            pixit_warning=self.pixit_test_event_warning_co_alarm,
            pixit_critical=self.pixit_test_event_critical_co_alarm,
            pixit_clear=self.pixit_test_event_clear_co_alarm
        )


if __name__ == "__main__":
    default_matter_test_main()
