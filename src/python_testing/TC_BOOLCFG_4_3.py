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
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

sensorTrigger = 0x0080_0000_0000_0000
sensorUntrigger = 0x0080_0000_0000_0001


class TC_BOOLCFG_4_3(MatterBaseTest):
    async def read_boolcfg_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOLCFG_4_3(self) -> str:
        return "[TC-BOOLCFG-4.3] AlarmsEnabled functionality for inactive alarms with DUT as Server"

    def steps_TC_BOOLCFG_4_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "Read AttributeList attribute"),
            TestStep(3, "Verify AlarmsEnabled is supported"),
            TestStep("3a", "Set up event subscription for BooleanStateConfiguration cluster"),
            TestStep(4, "Create enabledAlarms and set to 0"),
            TestStep("5a", "Enable VIS alarm in enabledAlarms"),
            TestStep("5b", "Set AlarmsEnabled attribute to value of enabledAlarms using AlarmsToEnableDisable command"),
            TestStep(6, "Send TestEventTrigger with SensorTrigger event"),
            TestStep(7, "Read AlarmsActive attribute"),
            TestStep("7a", "If BOOLCFG.S.E00, verify AlarmsStateChanged event was generated"),
            TestStep(8, "Send TestEventTrigger with SensorUntrigger event"),
            TestStep(9, "Read AlarmsActive attribute"),
            TestStep("9a", "If BOOLCFG.S.E00, verify AlarmsStateChanged event was generated with alarmsActive == 0"),
            TestStep(10, "Set enabledAlarms to 0"),
            TestStep(11, "Set AlarmsEnabled attribute to value of enabledAlarms using AlarmsToEnableDisable command"),
            TestStep(12, "Send TestEventTrigger with SensorTrigger event"),
            TestStep(13, "Read AlarmsActive attribute"),
            TestStep(14, "Send TestEventTrigger with SensorUntrigger event"),
            TestStep("15a", "Enable AUD alarm in enabledAlarms"),
            TestStep("15b", "Set AlarmsEnabled attribute to value of enabledAlarms using AlarmsToEnableDisable command"),
            TestStep(16, "Send TestEventTrigger with SensorTrigger event"),
            TestStep(17, "Read AlarmsActive attribute"),
            TestStep("17a", "If BOOLCFG.S.E00, verify AlarmsStateChanged event was generated"),
            TestStep(18, "Send TestEventTrigger with SensorUntrigger event"),
            TestStep(19, "Read AlarmsActive attribute"),
            TestStep("19a", "If BOOLCFG.S.E00, verify AlarmsStateChanged event was generated with alarmsActive == 0"),
            TestStep(20, "Set enabledAlarms to 0"),
            TestStep(21, "Set AlarmsEnabled attribute to value of enabledAlarms using AlarmsToEnableDisable command"),
            TestStep(22, "Send TestEventTrigger with SensorTrigger event"),
            TestStep(23, "Read AlarmsActive attribute"),
            TestStep(24, "Send TestEventTrigger with SensorUntrigger event"),
        ]

    def pics_TC_BOOLCFG_4_3(self) -> list[str]:
        return [
            "BOOLCFG.S",
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_BOOLCFG_4_3(self):

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

        self.step("2a")
        feature_map = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_vis_feature_supported = feature_map & cluster.Bitmaps.Feature.kVisual
        is_aud_feature_supported = feature_map & cluster.Bitmaps.Feature.kAudible

        self.step("2b")
        attribute_list = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        # Check if AlarmsStateChanged event (BOOLCFG.S.E00) is supported
        alarms_state_changed_event_supported = self.check_pics("BOOLCFG.S.E00")

        self.step(3)
        if attributes.AlarmsEnabled.attribute_id not in attribute_list:
            log.info("AlarmsEnabled not supported skipping test case")

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
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(alarmsToEnableDisable=enabledAlarms), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(6)
        if is_vis_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorTrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step(7)
        if is_vis_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, cluster.Bitmaps.AlarmModeBitmap.kVisual,
                                 "AlarmsActive does not match expected value")
        else:
            log.info("Test step skipped")

        # Verify AlarmsStateChanged event after VIS trigger
        self.step("7a")
        if alarms_state_changed_event_supported and is_vis_feature_supported:
            event = event_listener.wait_for_event_report(
                cluster.Events.AlarmsStateChanged, timeout_sec=30.0)
            asserts.assert_not_equal(event.alarmsActive, 0,
                                     "AlarmsStateChanged event did not report non-zero alarmsActive after sensor trigger")
            log.info(f"Received AlarmsStateChanged event with alarmsActive = {event.alarmsActive}")
        else:
            log.info("Test step skipped")
            self.mark_current_step_skipped()

        if event_listener is not None:
            event_listener.reset()

        self.step(8)
        if is_vis_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step(9)
        if is_vis_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, 0, "AlarmsActive does not match expected value")
        else:
            log.info("Test step skipped")

        # Verify AlarmsStateChanged event after VIS untrigger
        self.step("9a")
        if alarms_state_changed_event_supported and is_vis_feature_supported:
            event = event_listener.wait_for_event_report(
                cluster.Events.AlarmsStateChanged, timeout_sec=30.0)
            asserts.assert_equal(event.alarmsActive, 0,
                                 "AlarmsStateChanged event did not report alarmsActive == 0 after sensor untrigger")
            log.info("Received AlarmsStateChanged event with alarmsActive = 0")
        else:
            log.info("Test step skipped")
            self.mark_current_step_skipped()

        if event_listener is not None:
            event_listener.reset()

        self.step(10)
        enabledAlarms = 0

        self.step(11)
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(alarmsToEnableDisable=enabledAlarms), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(12)
        if is_vis_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorTrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step(13)
        if is_vis_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, 0, "AlarmsActive does not match expected value")
        else:
            log.info("Test step skipped")

        self.step(14)
        if is_vis_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step("15a")
        if is_aud_feature_supported:
            enabledAlarms |= cluster.Bitmaps.AlarmModeBitmap.kAudible
        else:
            log.info("Test step skipped")

        self.step("15b")
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(alarmsToEnableDisable=enabledAlarms), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        if event_listener is not None:
            event_listener.reset()

        self.step(16)
        if is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorTrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step(17)
        if is_aud_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, cluster.Bitmaps.AlarmModeBitmap.kAudible,
                                 "AlarmsActive does not match expected value")
        else:
            log.info("Test step skipped")

        # Verify AlarmsStateChanged event after AUD trigger
        self.step("17a")
        if alarms_state_changed_event_supported and is_aud_feature_supported:
            event = event_listener.wait_for_event_report(
                cluster.Events.AlarmsStateChanged, timeout_sec=30.0)
            asserts.assert_not_equal(event.alarmsActive, 0,
                                     "AlarmsStateChanged event did not report non-zero alarmsActive after sensor trigger")
            log.info(f"Received AlarmsStateChanged event with alarmsActive = {event.alarmsActive}")
        else:
            log.info("Test step skipped")
            self.mark_current_step_skipped()

        if event_listener is not None:
            event_listener.reset()

        self.step(18)
        if is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step(19)
        if is_aud_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, 0, "AlarmsActive does not match expected value")
        else:
            log.info("Test step skipped")

        # Verify AlarmsStateChanged event after AUD untrigger
        self.step("19a")
        if alarms_state_changed_event_supported and is_aud_feature_supported:
            event = event_listener.wait_for_event_report(
                cluster.Events.AlarmsStateChanged, timeout_sec=30.0)
            asserts.assert_equal(event.alarmsActive, 0,
                                 "AlarmsStateChanged event did not report alarmsActive == 0 after sensor untrigger")
            log.info("Received AlarmsStateChanged event with alarmsActive = 0")
        else:
            log.info("Test step skipped")
            self.mark_current_step_skipped()

        if event_listener is not None:
            event_listener.reset()

        self.step(20)
        enabledAlarms = 0

        self.step(21)
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(alarmsToEnableDisable=enabledAlarms), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(22)
        if is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorTrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step(23)
        if is_aud_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, 0, "AlarmsActive does not match expected value")
        else:
            log.info("Test step skipped")

        self.step(24)
        if is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
