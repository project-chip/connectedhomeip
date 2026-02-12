#
#    Copyright (c) 2023 Project CHIP Authors
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
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY:000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

sensorTrigger = 0x0080_0000_0000_0000
sensorUntrigger = 0x0080_0000_0000_0001


class TC_BOOLCFG_5_2(MatterBaseTest):
    async def read_boolcfg_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOLCFG_5_2(self) -> str:
        return "[TC-BOOLCFG-5.2] SuppressAlarm functionality for active alarms with DUT as Server"

    def steps_TC_BOOLCFG_5_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read FeatureMap attribute"),
            TestStep(3, "Verify SPRS feature is supported"),
            TestStep("3a", "Set up event subscription for BooleanStateConfiguration cluster"),
            TestStep(4, "Create enabledAlarms and set to 0"),
            TestStep("5a", "Enable VIS alarm in enabledAlarms"),
            TestStep("5b", "Enable AUD alarm in enabledAlarms"),
            TestStep("5c", "Set AlarmsEnabled attribute to value of enabledAlarms using AlarmsToEnableDisable command"),
            TestStep(6, "Send TestEventTrigger with SensorTrigger event"),
            TestStep("6a", "If BOOLCFG.S.E00, verify AlarmsStateChanged event was generated after trigger"),
            TestStep(7, "Suppress VIS alarm using SuppressAlarm command"),
            TestStep(8, "Read AlarmsSuppressed attribute"),
            TestStep("8a", "If BOOLCFG.S.E00, verify AlarmsStateChanged event was generated after VIS suppress"),
            TestStep(9, "Suppress AUD alarm using SuppressAlarm command"),
            TestStep(10, "Read AlarmsActive attribute"),
            TestStep("10a", "If BOOLCFG.S.E00, verify AlarmsStateChanged event was generated after AUD suppress"),
            TestStep(11, "Send TestEventTrigger with SensorUntrigger event")
        ]

    def pics_TC_BOOLCFG_5_2(self) -> list[str]:
        return [
            "BOOLCFG.S",
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_BOOLCFG_5_2(self):

        asserts.assert_true('PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY' in self.matter_test_config.global_test_params,
                            "PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY must be included on the command line in "
                            "the --hex-arg flag as PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY:<key>, "
                            "e.g. --hex-arg PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY:000102030405060708090a0b0c0d0e0f")

        endpoint = self.get_endpoint()
        node_id = self.dut_node_id
        dev_ctrl = self.default_controller
        enableKey = self.matter_test_config.global_test_params['PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY']

        self.step(1)
        cluster = Clusters.BooleanStateConfiguration
        attributes = cluster.Attributes

        self.step(2)
        feature_map = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_sprs_feature_supported = feature_map & cluster.Bitmaps.Feature.kAlarmSuppress
        is_vis_feature_supported = feature_map & cluster.Bitmaps.Feature.kVisual
        is_aud_feature_supported = feature_map & cluster.Bitmaps.Feature.kAudible

        # Check if AlarmsStateChanged event (BOOLCFG.S.E00) is supported
        alarms_state_changed_event_supported = self.check_pics("BOOLCFG.S.E00")

        self.step(3)
        if not is_sprs_feature_supported:
            log.info("AlarmSuppress feature not supported skipping test case")

            # Skipping all remainig steps
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                log.info("Test step skipped")

            return
        log.info("Test step skipped")

        # Set up event subscription
        self.step("3a")
        event_listener = None
        if alarms_state_changed_event_supported:
            event_listener = EventSubscriptionHandler(expected_cluster=cluster)
            await event_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30)
            log.info("Event subscription established for BooleanStateConfiguration cluster.")
        else:
            log.info("AlarmsStateChanged event not supported. Skipping event subscription setup.")
            self.mark_current_step_skipped()

        self.step(4)
        enabledAlarms = 0

        self.step("5a")
        if is_vis_feature_supported:
            enabledAlarms |= cluster.Bitmaps.AlarmModeBitmap.kVisual
        else:
            log.info("Test step skipped")

        self.step("5b")
        if is_aud_feature_supported:
            enabledAlarms |= cluster.Bitmaps.AlarmModeBitmap.kAudible
        else:
            log.info("Test step skipped")

        self.step("5c")
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(alarmsToEnableDisable=enabledAlarms), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(6)
        if is_vis_feature_supported or is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorTrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        # Verify AlarmsStateChanged event after trigger
        self.step("6a")
        if alarms_state_changed_event_supported and (is_vis_feature_supported or is_aud_feature_supported):
            event = event_listener.wait_for_event_report(
                cluster.Events.AlarmsStateChanged, timeout_sec=30.0)
            asserts.assert_not_equal(event.alarmsActive, 0,
                                     "AlarmsStateChanged event did not report non-zero alarmsActive after sensor trigger")
            log.info(f"Received AlarmsStateChanged event with alarmsActive = {event.alarmsActive}")
        else:
            log.info("AlarmsStateChanged event not supported or no alarms enabled. Skipping step 6a.")
            self.mark_current_step_skipped()

        if event_listener is not None:
            event_listener.reset()

        self.step(7)
        if is_vis_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.SuppressAlarm(alarmsToSuppress=cluster.Bitmaps.AlarmModeBitmap.kVisual), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step(8)
        if is_vis_feature_supported:
            alarms_suppressed_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsSuppressed)
            asserts.assert_not_equal((alarms_suppressed_dut & 0b01), 0, "Bit 0 in AlarmsSuppressed is not 1")
        else:
            log.info("Test step skipped")

        # Verify AlarmsStateChanged event after VIS suppress
        self.step("8a")
        if alarms_state_changed_event_supported and is_vis_feature_supported:
            event = event_listener.wait_for_event_report(
                cluster.Events.AlarmsStateChanged, timeout_sec=30.0)
            asserts.assert_is_not_none(event.alarmsSuppressed,
                                       "AlarmsStateChanged event missing alarmsSuppressed field after VIS suppress")
            log.info(f"Received AlarmsStateChanged event with alarmsSuppressed = {event.alarmsSuppressed}")
        else:
            log.info("Test step skipped")
            self.mark_current_step_skipped()

        if event_listener is not None:
            event_listener.reset()

        self.step(9)
        if is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.SuppressAlarm(alarmsToSuppress=cluster.Bitmaps.AlarmModeBitmap.kAudible), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step(10)
        if is_aud_feature_supported:
            alarms_suppressed_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsSuppressed)
            asserts.assert_not_equal((alarms_suppressed_dut & 0b10), 0, "Bit 1 in AlarmsSuppressed is not 1")
        else:
            log.info("Test step skipped")

        # Verify AlarmsStateChanged event after AUD suppress
        self.step("10a")
        if alarms_state_changed_event_supported and is_aud_feature_supported:
            event = event_listener.wait_for_event_report(
                cluster.Events.AlarmsStateChanged, timeout_sec=30.0)
            asserts.assert_is_not_none(event.alarmsSuppressed,
                                       "AlarmsStateChanged event missing alarmsSuppressed field after AUD suppress")
            log.info(f"Received AlarmsStateChanged event with alarmsSuppressed = {event.alarmsSuppressed}")
        else:
            log.info("Test step skipped")
            self.mark_current_step_skipped()

        self.step(11)
        if is_vis_feature_supported or is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass


if __name__ == "__main__":
    default_matter_test_main()
