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


class TC_BOOLCFG_4_2(MatterBaseTest):
    async def read_boolcfg_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOLCFG_4_2(self) -> str:
        return "[TC-BOOLCFG-4.2] AlarmsActive attribute with DUT as Server"

    def steps_TC_BOOLCFG_4_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "Read AttributeList attribute"),
            TestStep("3a", "Create variable named enabledAlarms"),
            TestStep("3b", "If VIS is supported, set bit 0 to 1"),
            TestStep("3c", "If AUD is supported, set bit 1 to 1"),
            TestStep("3d", "Set AlarmsEnabled attribute to value of enabledAlarms using EnableDisableAlarm command"),
            TestStep("3e", "Set up event subscription for BooleanStateConfiguration cluster"),
            TestStep(4, "Send TestEventTrigger with SensorTrigger event"),
            TestStep(5, "Read AlarmsActive attribute"),
            TestStep("6a", "Verify VIS alarm is active, if supported"),
            TestStep("6b", "Verify VIS alarm is not active, if not supported"),
            TestStep("7a", "Verify AUD alarm is active, if supported"),
            TestStep("7b", "Verify AUD alarm is not active, if not supported"),
            TestStep("7c", "If BOOLCFG.S.E00, verify AlarmsStateChanged event was generated with expected alarmsActive"),
            TestStep(8, "Send TestEventTrigger with SensorUntrigger event"),
            TestStep(9, "Read AlarmsActive attribute"),
            TestStep("9a", "If BOOLCFG.S.E00, verify AlarmsStateChanged event was generated with alarmsActive == 0"),
        ]

    def pics_TC_BOOLCFG_4_2(self) -> list[str]:
        return [
            "BOOLCFG.S",
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_BOOLCFG_4_2(self):

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
        log.info(f"AlarmsStateChanged event supported (BOOLCFG.S.E00): {alarms_state_changed_event_supported}")

        self.step("3a")
        enabledAlarms = 0

        self.step("3b")
        if attributes.AlarmsEnabled.attribute_id in attribute_list and is_vis_feature_supported:
            enabledAlarms |= cluster.Bitmaps.AlarmModeBitmap.kVisual
        else:
            log.info("Test step skipped")

        self.step("3c")
        if attributes.AlarmsEnabled.attribute_id in attribute_list and is_aud_feature_supported:
            enabledAlarms |= cluster.Bitmaps.AlarmModeBitmap.kAudible
        else:
            log.info("Test step skipped")

        self.step("3d")
        if attributes.AlarmsEnabled.attribute_id in attribute_list:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(alarmsToEnableDisable=enabledAlarms), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        # Set up event subscription
        self.step("3e")
        event_listener = None
        if alarms_state_changed_event_supported:
            event_listener = EventSubscriptionHandler(expected_cluster=cluster)
            await event_listener.start(dev_ctrl, node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30)
            log.info("Event subscription established for BooleanStateConfiguration cluster.")
        else:
            log.info("AlarmsStateChanged event not supported. Skipping event subscription setup.")
            self.mark_current_step_skipped()

        self.step(4)
        if is_vis_feature_supported or is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorTrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step(5)
        activeAlarms = 0

        if is_vis_feature_supported or is_aud_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_not_equal(activeAlarms, 0, "AlarmsActive is 0")
        else:
            log.info("Test step skipped")

        self.step("6a")
        if is_vis_feature_supported:
            asserts.assert_not_equal(
                (activeAlarms & cluster.Bitmaps.AlarmModeBitmap.kVisual), 0, "Bit 0 in AlarmsActive is not 1")
        else:
            log.info("Test step skipped")

        self.step("6b")
        if not is_vis_feature_supported:
            asserts.assert_equal((activeAlarms & cluster.Bitmaps.AlarmModeBitmap.kVisual),
                                 0, "Bit 0 in AlarmsActive is not 0")
        else:
            log.info("Test step skipped")

        self.step("7a")
        if is_aud_feature_supported:
            asserts.assert_not_equal(
                (activeAlarms & cluster.Bitmaps.AlarmModeBitmap.kAudible), 0, "Bit 1 in AlarmsActive is not 1")
        else:
            log.info("Test step skipped")

        self.step("7b")
        if not is_aud_feature_supported:
            asserts.assert_equal((activeAlarms & cluster.Bitmaps.AlarmModeBitmap.kAudible),
                                 0, "Bit 1 in AlarmsActive is not 0")
        else:
            log.info("Test step skipped")

        # Verify AlarmsStateChanged event after sensor trigger
        self.step("7c")
        if alarms_state_changed_event_supported and (is_vis_feature_supported or is_aud_feature_supported):
            event = event_listener.wait_for_event_report(
                cluster.Events.AlarmsStateChanged, timeout_sec=30.0)
            asserts.assert_not_equal(event.alarmsActive, 0,
                                     "AlarmsStateChanged event did not report non-zero alarmsActive after sensor trigger")
            log.info(f"Received AlarmsStateChanged event with alarmsActive = {event.alarmsActive}")
        else:
            log.info("AlarmsStateChanged event not supported or no alarms enabled. Skipping step 7c.")
            self.mark_current_step_skipped()

        # Reset event listener for next cycle
        if event_listener is not None:
            event_listener.reset()

        self.step(8)
        if is_vis_feature_supported or is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            log.info("Test step skipped")

        self.step(9)
        if is_vis_feature_supported or is_aud_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, 0, "AlarmsActive is not 0")
        else:
            log.info("Test step skipped")

        # Verify AlarmsStateChanged event after sensor untrigger
        self.step("9a")
        if alarms_state_changed_event_supported and (is_vis_feature_supported or is_aud_feature_supported):
            event = event_listener.wait_for_event_report(
                cluster.Events.AlarmsStateChanged, timeout_sec=30.0)
            asserts.assert_equal(event.alarmsActive, 0,
                                 "AlarmsStateChanged event did not report alarmsActive == 0 after sensor untrigger")
            log.info("Received AlarmsStateChanged event with alarmsActive = 0")
        else:
            log.info("AlarmsStateChanged event not supported or no alarms enabled. Skipping step 9a.")
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
