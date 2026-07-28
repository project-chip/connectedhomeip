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

from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.interaction_model import InteractionModelError, Status
from TC_HSTAT_Test_Base import HSTATBase

log = logging.getLogger(__name__)

# Auto-generated from test specification: [TC-HSTAT-2.3] Setpoint functionality with DUT as Server


class TC_HSTAT_2_3(MatterBaseTest, HSTATBase):

    def pics_TC_HSTAT_2_3(self) -> list[str]:
        return [
            "HSTAT.S",
            "HSTAT.S.F03"
        ]

    def desc_TC_HSTAT_2_3(self) -> str:
        return "[TC-HSTAT-2.3] Setpoint functionality with DUT as Server"

    def steps_TC_HSTAT_2_3(self):
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep(2, "TH sends command On to the On/Off cluster on the same endpoint as this cluster.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(3, "TH reads from the DUT the MinSetpoint attribute.", "Store the value as MinSetpointValue"),
            TestStep(4, "TH reads from the DUT the MaxSetpoint attribute.", "Store the value as MaxSetpointValue."),
            TestStep(5, "TH reads from the DUT the Step attribute.", "Store the value as StepValue."),
            TestStep(6, "TH sends command SetSettings with the Mode field set to Humidifier or Dehumidifier",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(7, "TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(8, "TH sends command SetSettings with the UserSetpoint field set to MinSetpointValue.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(9, "TH reads from the DUT the UserSetpoint attribute.",
                     "Verify that the DUT response contains a value of MinSetpointValue"),
            TestStep(10, "Individually subscribe to the UserSetpoint attribute",
                     "This will receive updates when this attribute changes value."),
            TestStep(11, "TH sends command SetSettings with the UserSetpoint field set to MaxSetpointValue.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(12, "TH reads from the DUT the UserSetpoint attribute.",
                     "Verify that the DUT response contains a value of MaxSetpointValue"),
            TestStep(13, "TH writes to the DUT the UserSetpoint attribute with MinSetpointValue + StepValue.",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(14, "TH reads from the DUT the UserSetpoint attribute.", "Verify that the DUT response contains a value of MinSetpointValue + StepValue. Verify: If no reports were received, fail the test. If (MaxSetpointValue - MinSetpointValue) = StepValue, verify 1 report was received and the value is MaxSetpointValue. If (MaxSetpointValue - MinSetpointValue) > StepValue, verify 2 reports were received and: The value for the first report is MaxSetpointValue. The value for the second report is MinSetpointValue + StepValue."),
            TestStep(15, "TH sends command SetSettings with the UserSetpoint field set to MinSetpointValue-1",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(16, "TH sends command SetSettings with the UserSetpoint field set to MaxSetpointValue+1",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
            TestStep(17, "TH sends command SetSettings with the UserSetpoint field set to MinSetpointValue+1",
                     "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_HSTAT_2_3(self):
        endpoint = self.get_endpoint()

        self.step(1)
        # Commission DUT to TH (can be skipped if done in a preceding test).
        await self.setup()

        self.step(2)
        # TH sends command On to the On/Off cluster on the same endpoint as this cluster.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_onoff_on_cmd_expect_success(endpoint=endpoint)

        self.step(3)
        # TH reads from the DUT the MinSetpoint attribute.
        # Store the value as MinSetpointValue
        dut_MinSetpoint = await self.read_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.MinSetpoint)

        self.step(4)
        # TH reads from the DUT the MaxSetpoint attribute.
        # Store the value as MaxSetpointValue.
        dut_MaxSetpoint = await self.read_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.MaxSetpoint)

        self.step(5)
        # TH reads from the DUT the Step attribute.
        # Store the value as StepValue.
        dut_Step = await self.read_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Step)

        self.step(6)
        # TH sends command SetSettings with the Mode field set to Humidifier or Dehumidifier
        # Verify DUT responds w/ status SUCCESS(0x00)
        if self.humidifierFeatureSupported:
            await self.write_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Mode(self.modeHumidifier))
        else:
            await self.write_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Mode(self.modeDehumidifier))

        self.step(7)
        # TH sends command SetSettings with the Continuous, Sleep, and Optimal fields set to False
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, continuous=False, sleep=False, optimal=False)

        self.step(8)
        # TH sends command SetSettings with the UserSetpoint field set to MinSetpointValue.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, userSetpoint=dut_MinSetpoint)

        self.step(9)
        # TH reads from the DUT the UserSetpoint attribute.
        # Verify that the DUT response contains a value of MinSetpointValue
        dut_Setpoint = await self.read_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.UserSetpoint)
        asserts.assert_equal(dut_Setpoint, dut_MinSetpoint, "UserSetpoint is not MinSetpoint as expacted")

        self.step(10)
        # Individually subscribe to the UserSetpoint attribute
        # This will receive updates when this attribute changes value.
        userSetpointSubscription = AttributeSubscriptionHandler(self.cluster, self.attributes.UserSetpoint)
        await userSetpointSubscription.start(self.default_controller, self.dut_node_id, endpoint)
        reportsReceived = [] 

        self.step(11)
        # TH sends command SetSettings with the UserSetpoint field set to MaxSetpointValue.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, userSetpoint=dut_MaxSetpoint)
        if dut_Setpoint != dut_MaxSetpoint:
            reportsReceived.append(userSetpointSubscription.wait_for_attribute_report().value)

        self.step(12)
        # TH reads from the DUT the UserSetpoint attribute.
        # Verify that the DUT response contains a value of MaxSetpointValue
        dut_Setpoint = await self.read_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.UserSetpoint)
        asserts.assert_equal(dut_Setpoint, dut_MaxSetpoint, "UserSetpoint is not MaxSetpoint as expacted")

        self.step(13)
        # TH writes to the DUT the UserSetpoint attribute with MinSetpointValue + StepValue.
        # Verify DUT responds w/ status SUCCESS(0x00)
        await self.send_SetSettingsCommand_expect_success(endpoint=endpoint, userSetpoint=dut_MinSetpoint+dut_Step)
        if dut_Setpoint != dut_MinSetpoint+dut_Step:
            reportsReceived.append(userSetpointSubscription.wait_for_attribute_report().value)

        self.step(14)
        # TH reads from the DUT the UserSetpoint attribute.
        # Verify that the DUT response contains a value of MinSetpointValue + StepValue.
        # Verify: If no reports were received, fail the test.
        # If (MaxSetpointValue - MinSetpointValue) = StepValue, verify 1 report was received and the value is MaxSetpointValue.
        # If (MaxSetpointValue - MinSetpointValue) > StepValue, verify 2 reports were received and: The value for the first report is MaxSetpointValue.
        # The value for the second report is MinSetpointValue + StepValue.
        dut_Setpoint = await self.read_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.UserSetpoint)
        asserts.assert_equal(dut_Setpoint, dut_MinSetpoint+dut_Step, "UserSetpoint is not MinSetpoint+Step as expacted")
        asserts.assert_greater_equal(len(reportsReceived), 0, "No reports received")
        asserts.assert_equal(reportsReceived[0], dut_MaxSetpoint, "First report is not MaxSetpoint")
        if len(reportsReceived) > 1:
            asserts.assert_equal(reportsReceived[1], dut_MinSetpoint+dut_Step, "Second report is not dut_MinSetpoint+dut_Step")

        self.step(15)
        # TH sends command SetSettings with the UserSetpoint field set to MinSetpointValue-1
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)
        await self.send_SetSettingsCommand_expect_error(endpoint=endpoint, userSetpoint=dut_MinSetpoint-1, error=Status.ConstraintError)

        self.step(16)
        # TH sends command SetSettings with the UserSetpoint field set to MaxSetpointValue+1
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)
        await self.send_SetSettingsCommand_expect_error(endpoint=endpoint, userSetpoint=dut_MaxSetpoint+1, error=Status.ConstraintError)

        self.step(17)
        # TH sends command SetSettings with the UserSetpoint field set to MinSetpointValue+1
        # Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)
        if dut_Step > 1:
            await self.send_SetSettingsCommand_expect_error(endpoint=endpoint, userSetpoint=dut_MinSetpoint-1, error=Status.ConstraintError)


if __name__ == '__main__':
    default_matter_test_main()
