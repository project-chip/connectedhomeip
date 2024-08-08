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


class TC_BOOLCFG_5_1(MatterBaseTest):
    async def read_boolcfg_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOLCFG_5_1(self) -> str:
        return "[TC-BOOLCFG-5.1] SuppressAlarm functionality for inactive alarms with DUT as Server"

    def steps_TC_BOOLCFG_5_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read FeatureMap attribute"),
            TestStep(3, "Verify SPRS feature is supported"),
            TestStep(4, "Create enabledAlarms and set to 0"),
            TestStep("5a", "Enable VIS alarm in enabledAlarms"),
            TestStep("5b", "Enable AUD alarm in enabledAlarms"),
            TestStep("5c", "Set AlarmsEnabled attribute to value of enabledAlarms using AlarmsToEnableDisable command"),
            TestStep(6, "Send TestEventTrigger with SensorUntrigger event"),
            TestStep("7a", "Suppress VIS alarm using SuppressAlarm command"),
            TestStep("7b", "Suppress VIS alarm using SuppressAlarm command"),
            TestStep(8, "Read AlarmsSuppressed attribute"),
            TestStep("9a", "Suppress AUD alarm using SuppressAlarm command"),
            TestStep("9b", "Suppress AUD alarm using SuppressAlarm command"),
            TestStep(10, "Read AlarmsSuppressed attribute"),
        ]
        return steps

    def pics_TC_BOOLCFG_5_1(self) -> list[str]:
        pics = [
            "BOOLCFG.S",
        ]
        return pics

    @async_test_body
    async def test_TC_BOOLCFG_5_1(self):

        asserts.assert_true('PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY' in self.matter_test_config.global_test_params,
                            "PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY must be included on the command line in "
                            "the --hex-arg flag as PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY:<key>, "
                            "e.g. --hex-arg PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY:000102030405060708090a0b0c0d0e0f")

        endpoint = self.user_params.get("endpoint", 1)
        enableKey = self.matter_test_config.global_test_params['PIXIT.BOOLCFG.TEST_EVENT_TRIGGER_KEY']

        self.step(1)
        attributes = Clusters.BooleanStateConfiguration.Attributes

        self.step(2)
        feature_map = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_sprs_feature_supported = feature_map & Clusters.BooleanStateConfiguration.Bitmaps.Feature.kAlarmSuppress
        is_vis_feature_supported = feature_map & Clusters.BooleanStateConfiguration.Bitmaps.Feature.kVisual
        is_aud_feature_supported = feature_map & Clusters.BooleanStateConfiguration.Bitmaps.Feature.kAudible

        self.step(3)
        if not is_sprs_feature_supported:
            logging.info("AlarmSuppress feature not supported skipping test case")

            # Skipping all remainig steps
            for step in self.get_test_steps(self.current_test_info.name)[self.current_step_index:]:
                self.step(step.test_plan_number)
                logging.info("Test step skipped")

            return
        else:
            logging.info("Test step skipped")

        self.step(4)
        enabledAlarms = 0

        self.step("5a")
        if is_vis_feature_supported:
            enabledAlarms |= Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kVisual
        else:
            logging.info("Test step skipped")

        self.step("5b")
        if is_aud_feature_supported:
            enabledAlarms |= Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kAudible
        else:
            logging.info("Test step skipped")

        self.step("5c")
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(alarmsToEnableDisable=enabledAlarms), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(6)
        if is_vis_feature_supported or is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass

        self.step("7a")
        if is_vis_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.SuppressAlarm(alarmsToSuppress=Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kVisual), endpoint=endpoint)
                asserts.fail("Received Success response when an INVALID_IN_STATE was expected")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step("7b")
        if not is_vis_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.SuppressAlarm(alarmsToSuppress=Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kVisual), endpoint=endpoint)
                asserts.fail("Received Success response when an CONSTRAINT_ERROR was expected")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(8)
        alarms_suppressed_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsSuppressed)
        asserts.assert_equal(alarms_suppressed_dut, 0, "AlarmsSuppressed is not the expected value")

        self.step("9a")
        if is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.SuppressAlarm(alarmsToSuppress=Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kAudible), endpoint=endpoint)
                asserts.fail("Received Success response when an INVALID_IN_STATE was expected")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.InvalidInState, "Unexpected error returned")
                pass

        self.step("9b")
        if not is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.SuppressAlarm(alarmsToSuppress=Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kAudible), endpoint=endpoint)
                asserts.fail("Received Success response when an CONSTRAINT_ERROR was expected")
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(10)
        alarms_suppressed_dut = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsSuppressed)
        asserts.assert_equal(alarms_suppressed_dut, 0, "AlarmsSuppressed is not the expected value")


if __name__ == "__main__":
    default_matter_test_main()
