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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --enable-key 000102030405060708090a0b0c0d0e0f --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY:000102030405060708090a0b0c0d0e0f --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

sensorTrigger = 0x0080_0000_0000_0000
sensorUntrigger = 0x0080_0000_0000_0001


class TC_BOOLCFG_4_2(MatterBaseTest):
    async def read_boolcfg_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOLCFG_4_2(self) -> str:
        return "[TC-BOOLCFG-4.2] AlarmsActive attribute with DUT as Server"

    def steps_TC_BOOLCFG_4_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "Read AttributeList attribute"),
            TestStep("3a", "Create variable named enabledAlarms"),
            TestStep("3b", "If VIS is supported, set bit 0 to 1"),
            TestStep("3c", "If AUD is supported, set bit 1 to 1"),
            TestStep("3d", "Set AlarmsEnabled attribute to value of enabledAlarms using EnableDisableAlarm command"),
            TestStep(4, "Send TestEventTrigger with SensorTrigger event"),
            TestStep(5, "Read AlarmsActive attribute"),
            TestStep("6a", "Verify VIS alarm is active, if supported"),
            TestStep("6b", "Verify VIS alarm is not active, if not supported"),
            TestStep("7a", "Verify AUD alarm is active, if supported"),
            TestStep("7b", "Verify AUD alarm is not active, if not supported"),
            TestStep(8, "Send TestEventTrigger with SensorUntrigger event"),
            TestStep(9, "Read AlarmsActive attribute"),
        ]
        return steps

    def pics_TC_BOOLCFG_4_2(self) -> list[str]:
        pics = [
            "BOOLCFG.S",
        ]
        return pics

    @async_test_body
    async def test_TC_BOOLCFG_4_2(self):

        asserts.assert_true('PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY' in self.matter_test_config.global_test_params,
                            "PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY must be included on the command line in "
                            "the --hex-arg flag as PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY:<key>, "
                            "e.g. --hex-arg PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY:000102030405060708090a0b0c0d0e0f")

        endpoint = self.user_params.get("endpoint", 1)
        enableKey = self.matter_test_config.global_test_params['PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY']

        self.step(1)
        attributes = Clusters.BooleanStateConfiguration.Attributes

        self.step("2a")
        feature_map = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_vis_feature_supported = feature_map & Clusters.BooleanStateConfiguration.Bitmaps.Feature.kVisual
        is_aud_feature_supported = feature_map & Clusters.BooleanStateConfiguration.Bitmaps.Feature.kAudible

        self.step("2b")
        attribute_list = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step("3a")
        enabledAlarms = 0

        self.step("3b")
        if attributes.AlarmsEnabled.attribute_id in attribute_list and is_vis_feature_supported:
            enabledAlarms |= Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kVisual
        else:
            logging.info("Test step skipped")

        self.step("3c")
        if attributes.AlarmsEnabled.attribute_id in attribute_list and is_aud_feature_supported:
            enabledAlarms |= Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kAudible
        else:
            logging.info("Test step skipped")

        self.step("3d")
        if attributes.AlarmsEnabled.attribute_id in attribute_list:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(alarmsToEnableDisable=enabledAlarms), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(4)
        if is_vis_feature_supported or is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorTrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(5)
        activeAlarms = 0

        if is_vis_feature_supported or is_aud_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_not_equal(activeAlarms, 0, "AlarmsActive is 0")
        else:
            logging.info("Test step skipped")

        self.step("6a")
        if is_vis_feature_supported:
            asserts.assert_not_equal(
                (activeAlarms & Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kVisual), 0, "Bit 0 in AlarmsActive is not 1")
        else:
            logging.info("Test step skipped")

        self.step("6b")
        if not is_vis_feature_supported:
            asserts.assert_equal((activeAlarms & Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kVisual),
                                 0, "Bit 0 in AlarmsActive is not 0")
        else:
            logging.info("Test step skipped")

        self.step("7a")
        if is_aud_feature_supported:
            asserts.assert_not_equal(
                (activeAlarms & Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kAudible), 0, "Bit 1 in AlarmsActive is not 1")
        else:
            logging.info("Test step skipped")

        self.step("7b")
        if not is_aud_feature_supported:
            asserts.assert_equal((activeAlarms & Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kAudible),
                                 0, "Bit 1 in AlarmsActive is not 0")
        else:
            logging.info("Test step skipped")

        self.step(8)
        if is_vis_feature_supported or is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(9)
        if is_vis_feature_supported or is_aud_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, 0, "AlarmsActive is not 0")
        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
