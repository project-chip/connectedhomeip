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
#     app-args: --discriminator 1234 --KVS /tmp/kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --int-arg PIXIT.SMOKECO.HIEST_PRI_ALARM_1:1
#       --int-arg PIXIT.SMOKECO.HIEST_PRI_ALARM_2:7
#       --int-arg PIXIT.SMOKECO.HIEST_PRI_ALARM_3:7
#       --int-arg PIXIT.SMOKECO.HIEST_PRI_ALARM_4:7
#       --int-arg PIXIT.SMOKECO.HIEST_PRI_ALARM_5:3
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#
import logging

from mobly import asserts
from support_modules.smokeco_support import SmokeCoBaseTest

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body, has_feature, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, AttributeValue
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_SMOKECO_2_6(SmokeCoBaseTest):

    attribute_report_timeout = 300

    @async_test_body
    async def setup_test(self):
        super().setup_test()
        # ALARM PRIORITY
        # These PIXIT values might change depending of the manufacturer or also if one or both of the
        # interconnect alarms attributes are disabled. Update accordigly here or in the arguments if required.
        self.pixit_hiest_pri_alarm_1 = self.user_params.get("PIXIT.SMOKECO.HIEST_PRI_ALARM_1", 1)
        self.pixit_hiest_pri_alarm_2 = self.user_params.get("PIXIT.SMOKECO.HIEST_PRI_ALARM_2", 7)
        self.pixit_hiest_pri_alarm_3 = self.user_params.get("PIXIT.SMOKECO.HIEST_PRI_ALARM_3", 7)
        self.pixit_hiest_pri_alarm_4 = self.user_params.get("PIXIT.SMOKECO.HIEST_PRI_ALARM_4", 7)
        self.pixit_hiest_pri_alarm_5 = self.user_params.get("PIXIT.SMOKECO.HIEST_PRI_ALARM_5", 3)

        # Verify we have these attributes are available to use in during the test case
        attribute_list = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.AttributeList)
        self.has_interconnected_co_alarm = self.smokeco_cluster.Attributes.InterconnectCOAlarm.attribute_id in attribute_list
        self.has_interconnected_smoke_alarm = self.smokeco_cluster.Attributes.InterconnectSmokeAlarm.attribute_id in attribute_list

    def desc_TC_SMOKECO_2_6(self) -> str:
        return "[TC-SMOKECO-2.6] ExpressedState Attribute - Multiple Alarms with DUT as Server"

    def steps_TC_SMOKECO_2_6(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).",
                     "DUT is commissioned to TH.", is_commissioning=True),
            TestStep(2, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)"),
            TestStep(3, "TH subscribes to BatteryAlert attribute from DUT",
                     "Verify that BatteryAlert attribute has a value of 0 (Normal)"),
            TestStep(4, "TH subscribes to InterconnectSmokeAlarm attribute from DUT",
                     "Verify that InterconnectSmokeAlarm attribute has a value of 0 (Normal)"),
            TestStep(5, "TH subscribes to InterconnectCOAlarm attribute from DUT",
                     "Verify that InterconnectCOAlarm attribute has a value of 0 (Normal)"),
            TestStep(6, "TH subscribes to COState attribute from DUT", "Verify that COState attribute has a value of 0 (Normal)"),
            TestStep(7, "TH subscribes to SmokeState attribute from DUT",
                     "Verify that SmokeState attribute has a value of 0 (Normal)"),
            TestStep(8, "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Verify that TestEventTriggersEnabled attribute has a value of 1 (True)"),
            TestStep(9, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Warning Battery Alert Test Event"),
            TestStep(10, "TH waits for a report of BatteryAlert attribute from DUT with a timeout of 300 seconds",
                     "Verify that BatteryAlert attribute has a value of 1 (Warning)"),
            TestStep(11, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Interconnect Smoke Alarm Test Event"),
            TestStep(12, "TH waits for a report of InterconnectSmokeAlarm attribute from DUT with a timeout of 300 seconds",
                     "Verify that InterconnectSmokeAlarm attribute has a value of 2 (Critical) or 1 (Warning)"),
            TestStep(13, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Interconnect CO Alarm Test Event"),
            TestStep(14, "TH waits for a report of InterconnectCOAlarm attribute from DUT with a timeout of 300 seconds",
                     "Verify that InterconnectCOAlarm attribute has a value of 2 (Critical) or 1 (Warning)"),
            TestStep(15, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Warning CO Alarm Test Event"),
            TestStep(16, "TH waits for a report of COState attribute from DUT with a timeout of 300 seconds",
                     "Verify that COState attribute has a value of 1 (Warning)"),
            TestStep(17, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Warning Smoke Alarm Test Event"),
            TestStep(18, "TH waits for a report of SmokeState attribute from DUT with a timeout of 300 seconds",
                     "Verify that SmokeState attribute has a value of 1 (Warning)"),
            TestStep(19, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value that indicates PIXIT.SMOKECO.HIEST_PRI_ALARM"),
            TestStep(20, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Smoke Alarm Test Event Clear"),
            TestStep(21, "TH waits for a report of SmokeState attribute from DUT with a timeout of 300 seconds",
                     "Verify that SmokeState attribute has a value of 0 (Normal)"),
            TestStep(22, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value that indicates PIXIT.SMOKECO.HIEST_PRI_ALARM"),
            TestStep(23, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for CO Alarm Test Event clear"),
            TestStep(24, "TH waits for a report of COState attribute from DUT with a timeout of 300 seconds",
                     "Verify that COState attribute has a value of 0 (Normal)"),
            TestStep(25, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value that indicates PIXIT.SMOKECO.HIEST_PRI_ALARM"),
            TestStep(26, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Interconnect CO Alarm Test Event Clear"),
            TestStep(27, "TH waits for a report of InterconnectCOAlarm attribute from DUT with a timeout of 300 seconds",
                     "Verify that InterconnectCOAlarm attribute has a value of 0 (Normal)"),
            TestStep(28, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value that indicates PIXIT.SMOKECO.HIEST_PRI_ALARM"),
            TestStep(29, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Interconnect Smoke Alarm Test Event Clear"),
            TestStep(30, "TH waits for a report of InterconnectSmokeAlarm attribute from DUT with a timeout of 300 seconds",
                     "Verify that InterconnectSmokeAlarm attribute has a value of 0 (Normal)"),
            TestStep(31, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value that indicates PIXIT.SMOKECO.HIEST_PRI_ALARM"),
            TestStep(32, "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SMOKECO.TEST_EVENT_TRIGGER for Battery Alert Test Event Clear"),
            TestStep(33, "TH waits for a report of BatteryAlert attribute from DUT with a timeout of 300 seconds",
                     "Verify that BatteryAlert attribute has a value of 0 (Normal)"),
            TestStep(34, "TH reads ExpressedState attribute from DUT",
                     "Verify that ExpressedState attribute has a value of 0 (Normal)")
        ]

    def pics_TC_SMOKECO_2_6(self) -> list[str]:
        return [
            "SMOKECO.S",
        ]

    def _get_attribute_matcher(self, attribute: AttributeValue, enum_status_list: list) -> AttributeMatcher:
        """Match the report agains a attribute and value.

        Args:
            attribute (AttributeValue): Attribute to match from the reports retrieved.
            enum_status_list (list): Value if the Attribute

        Returns:
            AttributeMatcher: Attribute Matcher to provide.
        """
        def predicate(report: AttributeValue) -> bool:
            return report.attribute == attribute and report.value in enum_status_list
        return AttributeMatcher.from_callable(description=f"Waiting for Attribute report for {attribute} with status: {enum_status_list}", matcher=predicate)

    @run_if_endpoint_matches(
        has_feature(cluster=Clusters.SmokeCoAlarm, feature=Clusters.SmokeCoAlarm.Bitmaps.Feature.kCoAlarm)
        and has_feature(cluster=Clusters.SmokeCoAlarm, feature=Clusters.SmokeCoAlarm.Bitmaps.Feature.kSmokeAlarm))
    async def test_TC_SMOKECO_2_6(self):
        # Commissioning already done
        self.step(1)

        self.step(2)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        # The following is for  Step 3 to Step 7
        # Instead of subscribe to each one of the attributes individually and creating multiple AttributeSubscriptions
        # this will subscribe to SmokeCo Cluster Attributes and then wait for reports filtering by Attibute and Value.
        self.step(3)
        # Subscribe to BatteryAlert Attibute ( Already done at smokeco_attribute_handler)
        smokeco_attribute_handler = AttributeSubscriptionHandler(expected_cluster=self.smokeco_cluster)
        await smokeco_attribute_handler.start(dev_ctrl=self.default_controller, node_id=self.dut_node_id,
                                              endpoint=self.get_endpoint(), max_interval_sec=30)

        self.step(4)
        # Subscribe to InterconnectSmokeAlarm Attibute ( Already done at smokeco_attribute_handler)
        if not self.has_interconnected_smoke_alarm:
            self.mark_current_step_skipped()
        self.step(5)
        # Subscribe to InterconnectCOAlarm Attibute ( Already done at smokeco_attribute_handler)
        if not self.has_interconnected_co_alarm:
            self.mark_current_step_skipped()

        self.step(6)
        # Subscribe to COState Attibute ( Already done at smokeco_attribute_handler)

        self.step(7)
        # Subscribe to SmokeStae Attibute ( Already done at smokeco_attribute_handler)

        self.step(8)
        # Reads General Diagnostic Cluster
        test_event_trigger_enabled = await self.read_single_attribute_check_success(
            cluster=self.gd_cluster,
            attribute=self.gd_cluster.Attributes.TestEventTriggersEnabled,
            dev_ctrl=self.default_controller,
            endpoint=0)
        asserts.assert_true(test_event_trigger_enabled, "TestEventTriggersEnabled are not enabled")

        self.step(9)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_battery_warning)

        self.step(10)
        smokeco_attribute_handler.await_all_expected_report_matches(
            expected_matchers=[self._get_attribute_matcher(
                self.smokeco_cluster.Attributes.BatteryAlert, [self.smokeco_cluster.Enums.AlarmStateEnum.kWarning])],
            timeout_sec=self.attribute_report_timeout
        )

        if self.has_interconnected_smoke_alarm:
            self.step(11)
            await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_interconnected_smoke_alarm)

            self.step(12)
            smokeco_attribute_handler.await_all_expected_report_matches(
                expected_matchers=[self._get_attribute_matcher(
                    self.smokeco_cluster.Attributes.InterconnectSmokeAlarm, [self.smokeco_cluster.Enums.AlarmStateEnum.kWarning, self.smokeco_cluster.Enums.AlarmStateEnum.kCritical])],
                timeout_sec=self.attribute_report_timeout
            )
        else:
            self.mark_step_range_skipped(11, 12)

        if self.has_interconnected_co_alarm:
            self.step(13)
            await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_interconnected_co_alarm)

            self.step(14)
            smokeco_attribute_handler.await_all_expected_report_matches(
                expected_matchers=[self._get_attribute_matcher(
                    self.smokeco_cluster.Attributes.InterconnectCOAlarm, [self.smokeco_cluster.Enums.AlarmStateEnum.kWarning, self.smokeco_cluster.Enums.AlarmStateEnum.kCritical])],
                timeout_sec=self.attribute_report_timeout
            )
        else:
            self.mark_step_range_skipped(13, 14)

        self.step(15)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_warning_co_alarm)

        self.step(16)
        smokeco_attribute_handler.await_all_expected_report_matches(
            expected_matchers=[self._get_attribute_matcher(
                self.smokeco_cluster.Attributes.COState, [self.smokeco_cluster.Enums.AlarmStateEnum.kWarning])],
            timeout_sec=self.attribute_report_timeout
        )

        self.step(17)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_warning_smoke_alarm)

        self.step(18)
        smokeco_attribute_handler.await_all_expected_report_matches(
            expected_matchers=[self._get_attribute_matcher(
                self.smokeco_cluster.Attributes.SmokeState, [self.smokeco_cluster.Enums.AlarmStateEnum.kWarning])],
            timeout_sec=self.attribute_report_timeout
        )

        # Check which is the HIGHEST PRIORITY ALARM WHEN ALL ARE ACTIVE
        self.step(19)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.pixit_hiest_pri_alarm_1)

        self.step(20)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_clear_smoke_alarm)

        self.step(21)
        smokeco_attribute_handler.await_all_expected_report_matches(
            expected_matchers=[self._get_attribute_matcher(
                self.smokeco_cluster.Attributes.SmokeState, [self.smokeco_cluster.Enums.AlarmStateEnum.kNormal])],
            timeout_sec=self.attribute_report_timeout
        )

        self.step(22)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.pixit_hiest_pri_alarm_2)

        self.step(23)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_clear_co_alarm)

        self.step(24)
        smokeco_attribute_handler.await_all_expected_report_matches(
            expected_matchers=[self._get_attribute_matcher(
                self.smokeco_cluster.Attributes.COState, [self.smokeco_cluster.Enums.AlarmStateEnum.kNormal])],
            timeout_sec=self.attribute_report_timeout
        )

        self.step(25)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.pixit_hiest_pri_alarm_3)

        if self.has_interconnected_co_alarm:
            self.step(26)
            await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_interconnected_co_alarm_clear)

            self.step(27)
            smokeco_attribute_handler.await_all_expected_report_matches(
                expected_matchers=[self._get_attribute_matcher(
                    self.smokeco_cluster.Attributes.InterconnectCOAlarm, [self.smokeco_cluster.Enums.AlarmStateEnum.kNormal])],
                timeout_sec=self.attribute_report_timeout
            )
        else:
            self.mark_step_range_skipped(26, 27)

        self.step(28)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.pixit_hiest_pri_alarm_4)

        if self.has_interconnected_smoke_alarm:
            self.step(29)
            await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_interconnected_smoke_alarm_clear)

            self.step(30)
            smokeco_attribute_handler.await_all_expected_report_matches(
                expected_matchers=[self._get_attribute_matcher(
                    self.smokeco_cluster.Attributes.InterconnectSmokeAlarm, [self.smokeco_cluster.Enums.AlarmStateEnum.kNormal])],
                timeout_sec=self.attribute_report_timeout
            )
        else:
            self.mark_step_range_skipped(29, 30)

        self.step(31)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.pixit_hiest_pri_alarm_5)

        self.step(32)
        await self.send_test_event_triggers(eventTrigger=self.pixit_test_event_battery_clear)

        self.step(33)
        smokeco_attribute_handler.await_all_expected_report_matches(
            expected_matchers=[self._get_attribute_matcher(
                self.smokeco_cluster.Attributes.BatteryAlert, [self.smokeco_cluster.Enums.AlarmStateEnum.kNormal])],
            timeout_sec=self.attribute_report_timeout
        )

        self.step(34)
        expressed_state = await self.read_smokeco_attribute_expect_success(attribute=self.smokeco_cluster.Attributes.ExpressedState)
        asserts.assert_equal(expressed_state, self.smokeco_cluster.Enums.ExpressedStateEnum.kNormal)

        # Cancel AttributeReport handler
        smokeco_attribute_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
