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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 0
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
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
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler

log = logging.getLogger(__name__)


class TC_HSTAT_2_2(MatterBaseTest):
    def pics_TC_HSTAT_2_2(self) -> list[str]:
        return [
            "HSTAT.S",
        ]

    def desc_TC_HSTAT_2_2(self) -> str:
        return "[TC-HSTAT-2.2] Mode functionality with DUT as Server"

    def steps_TC_HSTAT_2_2(self):
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the SupportedModesMode attribute.", "Store the value as SupportedModes."),
            TestStep(3, "TH sends command Off to the On/Off cluster on the same endpoint as this cluster.", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(4, "TH reads from the DUT the SystemState attribute.", "Verify that the DUT response contains a value of Idle"),
            TestStep(5, "TH reads from the DUT the MistType attribute.", "Verify that the DUT response contains the NULL value."),
            TestStep(6, "TH sends command On to the On/Off cluster on the same endpoint as this cluster.", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(7, "TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(8, "Individually subscribe to the Mode and SystemState attributes", "This will receive updates when these attributes change value."),
            TestStep(9, "Iteratively write the value of the Mode attribute with the values in SupportedModes except for one.", "For each update, the DUT shall return a SUCCESS status code."),
            TestStep(10, "Send the SetSettings command with the Mode field set to the remaining value from SupportedModes", "Verify DUT responds w/ status SUCCESS(0x00) After all updates have been performed, verify: The order of the values of the Mode attribute reports matches the order of the SupportedModes list. The order of the values of the SystemState attribute reports matches the order of the SupportedModes list. Where the value in the SupportedModes list is Humidifier, the value in associated report SHALL be Humidifying or Idle. Where the value in the SupportedModes list is Dehumidifier, the value in associated report SHALL be Dehumidifying or Idle. Where the value in the SupportedModes list is FanOnly, the value in associated report SHALL be Fan. Where the value in the SupportedModes list is Auto, there MAY be one fewer report than for the Mode attribute or the corresponding report value SHALL be Idle, Humidifying or Dehumidifying."),
            TestStep(11, "TH sends command SetSettings with the Mode field set to Humidifier", "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(12, "TH sends command SetSettings with the Mode field set to Dehumidifier", "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(13, "TH sends command SetSettings with the Mode field set to FanOnly", "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(14, "TH sends command SetSettings with the Mode field set to Auto", "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(15, "TH sends command SetSettings with the Continuous field set to True", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(16, "TH sends command SetSettings with the Sleep field set to True", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(17, "TH sends command SetSettings with the Optimal field set to True", "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(18, "TH sends command SetSettings with the MistType field set to Cold", "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

    async def read_hstat_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.Humidistat
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def write_hstat_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.Humidistat
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, attribute)])
        err_msg = "Received error status {} when writing {}:{}".format(str(result[0].Status), str(cluster), str(attribute))
        asserts.assert_equal(result[0].Status, Status.Success, err_msg)

    async def send_hstat_cmd_expect_success(self, endpoint, command) -> None:
        await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=1000)

    async def send_hstat_cmd_expect_error(self, endpoint, command, error: Status) -> None:
        try:
            await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=1000)
            asserts.assert_true(False, "Unexpected command success, command=%s", command)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, error, "Unexpected error returned")
            pass

    async def send_onoff_on_cmd_expect_success(self, endpoint) -> None:
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.On(), endpoint=endpoint, timedRequestTimeoutMs=1000)

    async def send_onoff_off_cmd_expect_success(self, endpoint) -> None:
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.Off(), endpoint=endpoint, timedRequestTimeoutMs=1000)

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_HSTAT_2_2(self):
        endpoint = self.get_endpoint()

        self.step(1)
        # Commissioning already done.
        cluster = Clusters.Humidistat
        attributes = cluster.Attributes
        features = cluster.Bitmaps.Feature
        mistBitmap = cluster.Bitmaps.MistTypeBitmap
        SetSettings = Clusters.Humidistat.Commands.SetSettings
        SystemModes = cluster.Enums.SystemModeEnum

        # supported_attributes = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        feature_map = await self.read_setting(attributes.FeatureMap)
        supports_humidifier = bool(feature_map & features.kHumidifier)
        supports_dehumidifier = bool(feature_map & features.kDehumidifier)
        # supports_continuous = bool(feature_map & features.kContinuous)
        supports_sensor = bool(feature_map & features.kSensor)
        supports_auto = bool(feature_map & features.kAuto)
        supports_fan = bool(feature_map & features.kFan)
        # supports_optimal = bool(feature_map & features.kOptimal)
        supports_warm = bool(feature_map & features.kWarmMist)
        supports_cold = bool(feature_map & features.kColdMist)

        # some convenience definions
        modeHumidifier = cluster.Enums.ModeEnum.kHumidifier
        modeDehumidifier = cluster.Enums.ModeEnum.kDeumidifier
        modeAuto = cluster.Enums.ModeEnum.kAuto
        modeFanOnly = cluster.Enums.ModeEnum.kFanOnly
        # stateOff = cluster.Enums.SystemStateEnum.kOff
        # stateHumidifying = cluster.Enums.SystemStateEnum.kHumidifying
        # stateDehumidifying = cluster.Enums.SystemStateEnum.kDehumidifying
        # stateFan = cluster.Enums.SystemStateEnum.kFan
        stateIdle = cluster.Enums.SystemStateEnum.kIdle

        self.step(2)
        # TH reads from the DUT the SupportedModesMode attribute.
        # Store the value as SupportedModes.
        SupportedModes = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SupportedModes)

        self.step(3)
        # TH sends command Off to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_onoff_off_cmd_expect_success(endpoint=endpoint)

        self.step(4)
        # TH reads from the DUT the SystemState attribute.
        # Verify that the DUT response contains a value of Idle
        dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
        asserts.assert_equal(dut_SystemState, stateIdle, "SystemState is not idle")

        self.step(5)
        # TH reads from the DUT the MistType attribute.
        # Verify that the DUT response contains the NULL value.
        if humidifierFeatureSupported:
            dut_MistType = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MistType)
            asserts.assert_equal(dut_MistType, NullValue, "MistType is not NULL and should be")

        self.step(6)
        # TH sends command On to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_onoff_on_cmd_expect_success(endpoint=endpoint)

        self.step(7)
        # TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(Continuous=False, Sleep=False, Optimal=False))

        self.step(8)
        # Individually subscribe to the Mode and SystemState attributes
        # This will receive updates when these attributes change value.
        self.subscriptions = [
            AttributeSubscriptionHandler(cluster, attributes.Mode),
            AttributeSubscriptionHandler(cluster, attributes.SystemState)
        ]
        for sub in self.subscriptions:
            await sub.start(self.default_controller, self.dut_node_id, self.endpoint)

        self.step(9)
        # Iteratively write the value of the Mode attribute with the values in SupportedModes except for one.
        # For each update, the DUT shall return a SUCCESS status code.
        *mostModes, lastMode = SupportedModes
        for mode in mostModes:
            await self.write_hstat_attribute_expect_success(attributes.Mode(mode))

        self.step(10)
        # Send the SetSettings command with the Mode field set to the remaining value from SupportedModes
        # Verify DUT responds w/ status SUCCESS(0x00) After all updates have been performed, verify: The order of the values of the Mode attribute reports matches the order of the SupportedModes list. The order of the values of the SystemState attribute reports matches the order of the SupportedModes list. Where the value in the SupportedModes list is Humidifier, the value in associated report SHALL be Humidifying or Idle. Where the value in the SupportedModes list is Dehumidifier, the value in associated report SHALL be Dehumidifying or Idle. Where the value in the SupportedModes list is FanOnly, the value in associated report SHALL be Fan. Where the value in the SupportedModes list is Auto, there MAY be one fewer report than for the Mode attribute or the corresponding report value SHALL be Idle, Humidifying or Dehumidifying.
        await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(Mode=lastMode))
        log.info("Step 10 attribute reports info:")
        for sub in self.subscriptions:
            log.info("Attribute reports received: %s", len(self.subscriptions.attribute_queue.queue))
            for report in sub.attribute_queue.queue:
                log.info("Attribute: %s, value: %s", report.attribute, report.value)

        self.step(11)
        # TH sends command SetSettings with the Mode field set to Humidifier
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)

        self.step(12)
        # TH sends command SetSettings with the Mode field set to Dehumidifier
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)

        self.step(13)
        # TH sends command SetSettings with the Mode field set to FanOnly
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)

        self.step(14)
        # TH sends command SetSettings with the Mode field set to Auto
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)

        self.step(15)
        # TH sends command SetSettings with the Continuous field set to True
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(16)
        # TH sends command SetSettings with the Sleep field set to True
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(17)
        # TH sends command SetSettings with the Optimal field set to True
        # Verify DUT responds w/ status SUCCESS(0x00)

        self.step(18)
        # TH sends command SetSettings with the MistType field set to Cold
        # Verify DUT responds w/ status SUCCESS(0x00)
        # Move from here down somewhere else

        self.step(next(step))  # Set Mode to Humidifier or Dehumidifier
        if supports_humidifier:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mode=modeHumidifier))
        else:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mode=modeDehumidifier))

        self.step(next(step))  # Read MinSetpoint attribute
        if supports_sensor:
            dut_MinSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MinSetpoint)

        self.step(next(step))  # Read MaxSetpoint attribute
        if supports_sensor:
            dut_MaxSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MaxSetpoint)

        self.step(next(step))  # Read Step attribute
        if supports_sensor:
            dut_Step = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Step)

        self.step(next(step))  # Set UserSetpoint to MinSetpoint - 1
        if supports_sensor:
            try:
                await self.send_single_cmd(cmd=SetSettings(UserSetpoint=dut_MinSetpoint), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(next(step))  # Read UserSetpoint, confirm it is MinSetpoint
        if supports_sensor:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_equal(dut_UserSetpoint, dut_MinSetpoint, "UserSetpoint attribute not equal to MinSetpoint attribute")

        self.step(next(step))  # Set UserSetpoint to MinSetpoint + Step
        if supports_sensor:
            try:
                await self.send_single_cmd(cmd=SetSettings(UserSetpoint=dut_MinSetpoint+dut_Step), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(next(step))  # Read UserSetpoint, confirm it is MinSetpoint + Step
        if supports_sensor:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_equal(dut_UserSetpoint, dut_MinSetpoint+dut_Step,
                                 "UserSetpoint attribute not equal to MinSetpoint + StepValue")

        self.step(next(step))  # Set UserSetpoint to MinSetpoint + 1
        if supports_sensor and dut_Step > 1:
            try:
                await self.send_single_cmd(cmd=SetSettings(UserSetpoint=dut_MinSetpoint+1), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")

        self.step(next(step))  # Read UserSetpoint, confirm it is MinSetpoint + 1
        if supports_sensor and dut_Step > 1:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_equal(dut_UserSetpoint, dut_MinSetpoint+1, "UserSetpoint attribute not equal to MinSetpoint + 1")

        self.step(next(step))  # Set UserSetpoint to MinSetpoint - 1
        if supports_sensor:
            try:
                await self.send_single_cmd(cmd=SetSettings(UserSetpoint=dut_MinSetpoint-1), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.CONSTRAINT_ERROR, "Unexpected error returned")
                pass

        self.step(next(step))  # Test that unit does not support humidifier
        if not supports_humidifier:
            try:
                await self.send_single_cmd(cmd=SetSettings(Mode=modeHumidifier), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.CONSTRAINT_ERROR, "Unexpected error returned")
                pass

        self.step(next(step))  # Test that unit does not support dehumidifier
        if not supports_dehumidifier:
            try:
                await self.send_single_cmd(cmd=SetSettings(Mode=modeDehumidifier), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.CONSTRAINT_ERROR, "Unexpected error returned")
                pass

        self.step(next(step))  # Test that unit does not support fan only
        if not supports_fan:
            try:
                await self.send_single_cmd(cmd=SetSettings(Mode=modeFanOnly), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.CONSTRAINT_ERROR, "Unexpected error returned")
                pass

        self.step(next(step))  # Test that unit does not support auto
        if not supports_auto:
            try:
                await self.send_single_cmd(cmd=SetSettings(Mode=modeAuto), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.CONSTRAINT_ERROR, "Unexpected error returned")
                pass

        self.step(next(step))  # Test that unit does not support Warm mist
        if not supports_warm:
            try:
                await self.send_single_cmd(cmd=SetSettings(MistType=mistBitmap.kWarmMist), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.INVALID_IN_STATE, "Unexpected error returned")
                pass

        self.step(next(step))  # Test that unit does not support Cold mist
        if not supports_cold:
            try:
                await self.send_single_cmd(cmd=SetSettings(MistType=mistBitmap.kColdMist), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.INVALID_IN_STATE, "Unexpected error returned")
                pass


if __name__ == "__main__":
    default_matter_test_main()
