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

import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

sensorTrigger = 0x0080_0000_0000_0000
sensorUntrigger = 0x0080_0000_0000_0001


class TC_BOOLCFG_4_3(MatterBaseTest):
    async def read_boolcfg_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOLCFG_4_3(self) -> str:
        return "[TC-BOOLCFG-4.3] AlarmsEnabled functionality for inactive alarms with DUT as Server"

    def steps_TC_BOOLCFG_4_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read FeatureMap attribute"),
            TestStep("2b", "Read AttributeList attribute"),
            TestStep(3, "Verify AlarmsEnabled is supported"),
            TestStep(4, "Create enabledAlarms and set to 0"),
            TestStep("5a", "Enable VIS alarm in enabledAlarms"),
            TestStep("5b", "Set AlarmsEnabled attribute to value of enabledAlarms using AlarmsToEnableDisable command"),
            TestStep(6, "Send TestEventTrigger with SensorTrigger event"),
            TestStep(7, "Read AlarmsActive attribute"),
            TestStep(8, "Send TestEventTrigger with SensorUntrigger event"),
            TestStep(9, "Read AlarmsActive attribute"),
            TestStep(10, "Set enabledAlarms to 0"),
            TestStep(11, "Set AlarmsEnabled attribute to value of enabledAlarms using AlarmsToEnableDisable command"),
            TestStep(12, "Send TestEventTrigger with SensorTrigger event"),
            TestStep(13, "Read AlarmsActive attribute"),
            TestStep(14, "Send TestEventTrigger with SensorUntrigger event"),
            TestStep("15a", "Enable AUD alarm in enabledAlarms"),
            TestStep("15b", "Set AlarmsEnabled attribute to value of enabledAlarms using AlarmsToEnableDisable command"),
            TestStep(16, "Send TestEventTrigger with SensorTrigger event"),
            TestStep(17, "Read AlarmsActive attribute"),
            TestStep(18, "Send TestEventTrigger with SensorUntrigger event"),
            TestStep(19, "Read AlarmsActive attribute"),
            TestStep(20, "Set enabledAlarms to 0"),
            TestStep(21, "Set AlarmsEnabled attribute to value of enabledAlarms using AlarmsToEnableDisable command"),
            TestStep(22, "Send TestEventTrigger with SensorTrigger event"),
            TestStep(23, "Read AlarmsActive attribute"),
            TestStep(24, "Send TestEventTrigger with SensorUntrigger event"),
        ]
        return steps

    def pics_TC_BOOLCFG_4_3(self) -> list[str]:
        pics = [
            "BOOLCFG.S",
        ]
        return pics

    @async_test_body
    async def test_TC_BOOLCFG_4_3(self):

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

        self.step(3)
        if attributes.AlarmsEnabled.attribute_id not in attribute_list:
            logging.info("AlarmsEnabled not supported skipping test case")

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
            logging.info("Test step skipped")

        self.step(7)
        if is_vis_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kVisual,
                                 "AlarmsActive does not match expected value")
        else:
            logging.info("Test step skipped")

        self.step(8)
        if is_vis_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(9)
        if is_vis_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, 0, "AlarmsActive does not match expected value")
        else:
            logging.info("Test step skipped")

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
            logging.info("Test step skipped")

        self.step(13)
        if is_vis_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, 0, "AlarmsActive does not match expected value")
        else:
            logging.info("Test step skipped")

        self.step(14)
        if is_vis_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step("15a")
        if is_aud_feature_supported:
            enabledAlarms |= Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kAudible
        else:
            logging.info("Test step skipped")

        self.step("15b")
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.BooleanStateConfiguration.Commands.EnableDisableAlarm(alarmsToEnableDisable=enabledAlarms), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(16)
        if is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorTrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(17)
        if is_aud_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kAudible,
                                 "AlarmsActive does not match expected value")
        else:
            logging.info("Test step skipped")

        self.step(18)
        if is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(19)
        if is_aud_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, 0, "AlarmsActive does not match expected value")
        else:
            logging.info("Test step skipped")

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
            logging.info("Test step skipped")

        self.step(23)
        if is_aud_feature_supported:
            activeAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsActive)
            asserts.assert_equal(activeAlarms, 0, "AlarmsActive does not match expected value")
        else:
            logging.info("Test step skipped")

        self.step(24)
        if is_aud_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=enableKey, eventTrigger=sensorUntrigger), endpoint=0)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
