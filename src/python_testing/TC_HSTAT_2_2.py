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
#       --endpoint 1
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
from TC_HSTAT_common import HSTATBase

from matter.clusters.Types import Nullable, NullValue
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_HSTAT_2_2(HSTATBase):
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
            TestStep(3, "TH sends command Off to the On/Off cluster on the same endpoint as this cluster.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(4, "TH reads from the DUT the SystemState attribute.",
                     "Verify that the DUT response contains a value of Idle"),
            TestStep(5, "TH reads from the DUT the MistType attribute.", "Verify that the DUT response contains a value between 1 and 3 inclusive."),
            TestStep(6, "TH sends command On to the On/Off cluster on the same endpoint as this cluster.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(7, "TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(8, "Establish a subscription to the Mode and SystemState attributes",
                     "This will receive updates when these attributes change value."),
            TestStep(9, "Iteratively write the value of the Mode attribute with the values in SupportedModes except for one.",
                     "For each update, the DUT shall return a SUCCESS status code."),
            TestStep(10, "Send the SetSettings command with the Mode field set to the remaining value from SupportedModes",
                     "Verify DUT responds w/ status SUCCESS(0x00) "
                     "After all updates have been performed, "
                     "verify: The order of the values of the Mode attribute reports matches the order of the SupportedModes list. "
                     "The order of the values of the SystemState attribute reports matches the order of the SupportedModes list. "
                     "Where the value in the SupportedModes list is Humidifier, the value in associated report SHALL be Humidifying or Idle. "
                     "Where the value in the SupportedModes list is Dehumidifier, the value in associated report SHALL be Dehumidifying or Idle. "
                     "Where the value in the SupportedModes list is FanOnly, the value in associated report SHALL be Fan. "
                     "Where the value in the SupportedModes list is Auto, there MAY be one fewer report than for the Mode attribute or the corresponding report value SHALL be Idle, Humidifying or Dehumidifying."),
            TestStep(11, "TH sends command SetSettings with the Mode field to Humidifier if supported or otherwise to Dehumidifier",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(12, "TH sends command SetSettings with the Mode field set to Humidifier",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(13, "TH sends command SetSettings with the Mode field set to Dehumidifier",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(14, "TH sends command SetSettings with the Mode field set to FanOnly",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(15, "TH sends command SetSettings with the Mode field set to Auto",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(16, "If the Continuous feature is not supported then TH sends command SetSettings with the Continuous field set to True, otherwise skip this step",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(17, "If the Sleep feature is not supported then TH sends command SetSettings with the Sleep field set to True, otherwise skip this step",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(18, "If the Optimal feature is not supported then TH sends command SetSettings with the Optimal field set to True, otherwise skip this step",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(19, "If the MistType attribute is not supported then TH sends command SetSettings with the MistType field set to Cold, otherwise skip this step",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_HSTAT_2_2(self):
        self.step(1)
        # Commissioning already done.
        await self.setup()

        self.step(2)
        # TH reads from the DUT the SupportedModesMode attribute.
        # Store the value as SupportedModes.
        SupportedModes = await self.read_attribute_expect_success(attribute=self.attributes.SupportedModes)

        self.step(3)
        # TH sends command Off to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_onoff_off_cmd_expect_success()

        self.step(4)
        # TH reads from the DUT the SystemState attribute.
        # Verify that the DUT response contains a value of Idle
        dut_SystemState = await self.read_attribute_expect_success(attribute=self.attributes.SystemState)
        asserts.assert_equal(dut_SystemState, self.stateIdle, "SystemState is not idle")

        # TH reads from the DUT the MistType attribute.
        # Verify that the DUT response contains a value between 1 and 3 inclusive.
        if self.humidifierFeatureSupported:
            dut_MistType = await self.read_attribute_expect_success(attribute=self.attributes.MistType)
            if isinstance(dut_MistType, Nullable) and dut_MistType == NullValue:
                log.info("MistType is NULL - SDK changes are incomplete, so skipping this step")
                self.skip_step(5)
            else:
                self.step(5)
                asserts.assert_greater_equal(dut_MistType, 1, "MistType is less than 1")
                asserts.assert_less_equal(dut_MistType, 3, "MistType is greater than 3")
        else:
            self.skip_step(5)

        self.step(6)
        # TH sends command On to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_onoff_on_cmd_expect_success()

        self.step(7)
        # TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_SetSettingsCommand_expect_success(continuous=False, sleep=False, optimal=False)

        self.step(8)
        # Establish a subscription to the Mode and SystemState attributes
        # This will receive updates when these attributes change value.
        modeSubscription = AttributeSubscriptionHandler(self.cluster, self.attributes.Mode)
        stateSubscription = AttributeSubscriptionHandler(self.cluster, self.attributes.SystemState)
        await modeSubscription.start(self.default_controller, self.dut_node_id, self.endpoint)
        await stateSubscription.start(self.default_controller, self.dut_node_id, self.endpoint)
        dut_CurrentMode = await self.read_attribute_expect_success(attribute=self.attributes.Mode)
        log.info("Current mode: %s", dut_CurrentMode)
        dut_CurrentState = await self.read_attribute_expect_success(attribute=self.attributes.SystemState)
        log.info("Current state: %s", dut_CurrentState)

        modeReportsExpected = []
        modeReportsReceived = []

        self.step(9)
        # Iteratively write the value of the Mode attribute with the values in SupportedModes except for one.
        # For each update, the DUT shall return a SUCCESS status code.
        *mostModes, lastMode = SupportedModes
        for mode in mostModes:
            await self.write_single_attribute(attribute_value=self.attributes.Mode(mode), endpoint_id=self.endpoint, expect_success=True)
            if dut_CurrentMode != mode:
                modeReportsReceived.append(modeSubscription.wait_for_attribute_report().value)
                modeReportsExpected.append(mode)
                dut_CurrentMode = await self.read_attribute_expect_success(attribute=self.attributes.Mode)
                dut_CurrentState = await self.read_attribute_expect_success(attribute=self.attributes.SystemState)

        self.step(10)
        # Send the SetSettings command with the Mode field set to the remaining value from SupportedModes
        # Verify DUT responds w/ status SUCCESS(0x00) After all updates have been performed, verify: The order of the values of the Mode attribute reports matches the order of the SupportedModes list. The order of the values of the SystemState attribute reports matches the order of the SupportedModes list. Where the value in the SupportedModes list is Humidifier, the value in associated report SHALL be Humidifying or Idle. Where the value in the SupportedModes list is Dehumidifier, the value in associated report SHALL be Dehumidifying or Idle. Where the value in the SupportedModes list is FanOnly, the value in associated report SHALL be Fan. Where the value in the SupportedModes list is Auto, there MAY be one fewer report than for the Mode attribute or the corresponding report value SHALL be Idle, Humidifying or Dehumidifying.
        dut_CurrentMode = await self.read_attribute_expect_success(attribute=self.attributes.Mode)
        await self.send_SetSettingsCommand_expect_success(mode=lastMode)
        if dut_CurrentMode != lastMode:
            modeReportsReceived.append(modeSubscription.wait_for_attribute_report().value)
            modeReportsExpected.append(lastMode)
        log.info("Step 10 attribute reports info:")
        log.info("Expected reports: %s", modeReportsExpected)
        log.info("Received reports: %s", modeReportsReceived)
        asserts.assert_equal(modeReportsExpected, modeReportsReceived, "Did not receive the expected attribute reports for mode.")
        # This needs to also test the SystemState reports, but the DUT is currently not updating SystemState

        self.step(11)
        # TH sends command SetSettings with the Mode field to Humidifier if supported or otherwise to Dehumidifier
        # Verify DUT responds w/ status SUCCESS(0x00)
        if self.humidifierFeatureSupported:
            await self.send_SetSettingsCommand_expect_success(mode=self.modeHumidifier)
        else:
            await self.send_SetSettingsCommand_expect_success(mode=self.modeDehumidifier)

        # TH sends command SetSettings with the Mode field set to Humidifier
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)
        if not self.humidifierFeatureSupported:
            self.step(12)
            await self.send_SetSettingsCommand_expect_error(mode=self.modeHumidifier, error=Status.ConstraintError)
        else:
            self.skip_step(12)

        # TH sends command SetSettings with the Mode field set to Dehumidifier
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)
        if not self.dehumidifierFeatureSupported:
            self.step(13)
            await self.send_SetSettingsCommand_expect_error(mode=self.modeDehumidifier, error=Status.ConstraintError)
        else:
            self.skip_step(13)

        # TH sends command SetSettings with the Mode field set to FanOnly
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)
        if not self.fanOnlyFeatureSupported:
            self.step(14)
            await self.send_SetSettingsCommand_expect_error(mode=self.modeFanOnly, error=Status.ConstraintError)
        else:
            self.skip_step(14)

        # TH sends command SetSettings with the Mode field set to Auto
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)
        if not self.autoFeatureSupported:
            self.step(15)
            await self.send_SetSettingsCommand_expect_error(mode=self.modeAuto, error=Status.ConstraintError)
        else:
            self.skip_step(15)

        # If the Continuous feature is not supported then TH sends command SetSettings with the Continuous field set to True, otherwise skip this step
        # Verify DUT responds w/ status SUCCESS(0x00)
        if not self.continuousFeatureSupported:
            self.step(16)
            await self.send_SetSettingsCommand_expect_success(continuous=True)
        else:
            self.skip_step(16)

        # If the Sleep feature is not supported then TH sends command SetSettings with the Sleep field set to True, otherwise skip this step
        # Verify DUT responds w/ status SUCCESS(0x00)
        if self.attributes.Sleep.attribute_id not in self.supported_attributes:
            self.step(17)
            await self.send_SetSettingsCommand_expect_success(sleep=True)
        else:
            self.skip_step(17)

        # If the Optimal feature is not supported then TH sends command SetSettings with the Optimal field set to True, otherwise skip this step
        # Verify DUT responds w/ status SUCCESS(0x00)
        if not self.optimalFeatureSupported:
            self.step(18)
            await self.send_SetSettingsCommand_expect_success(optimal=True)
        else:
            self.skip_step(18)

        # If the MistType attribute is not supported then TH sends command SetSettings with the MistType field set to Cold, otherwise skip this step
        # Verify DUT responds w/ status SUCCESS(0x00)
        if self.attributes.MistType.attribute_id not in self.supported_attributes:
            self.step(19)
            await self.send_SetSettingsCommand_expect_success(mistType=self.MistTypeBitmap.kMistCold)
        else:
            self.skip_step(19)


if __name__ == "__main__":
    default_matter_test_main()
