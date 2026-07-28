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
import inspect

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from TC_HSTAT_Test_Base import HSTATBase

log = logging.getLogger(__name__)


class TC_HSTAT_2_1(MatterBaseTest, HSTATBase):
    def pics_TC_HSTAT_2_1(self) -> list[str]:
        return [
            "HSTAT.S",
        ]

    def desc_TC_HSTAT_2_1(self) -> str:
        return "[TC-HSTAT-2.1] Primary functionality with DUT as Server"

    def steps_TC_HSTAT_2_1(self):
        return [
            TestStep(1, "Commissioning already done.", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the FeatureMap attribute.",
                        "Verify that the DUT response contains the FeatureMap attribute. Store the value as FeatureMap."),
            TestStep(3, "TH reads from the DUT the SupportedModesMode attribute.",
                        "Verify that the DUT response contains a list with up to 4 entries. Verify: - Each list item corresponds to a valid ModeEnum value. - Each list item is unique among other entries. - Each list item is supported by the FeatureMap. - All modes required by the FeatureMap are in the list."),
            TestStep(4, "TH reads from the DUT the Mode attribute.",
                        "Verify that the DUT response contains a value between 0 and 4 inclusive."),
            TestStep(5, "TH reads from the DUT the SystemState attribute.",
                        "Verify that the DUT response contains a value between 0 and 4 inclusive."),
            TestStep(6, "TH reads from the DUT the MinSetpoint attribute.",
                        "Verify that the DUT response contains a value between 0 and 99 inclusive. Store the value as MinSetpointValue"),
            TestStep(7, "TH reads from the DUT the MaxSetpoint attribute.",
                        "Verify that the DUT response contains a value between MinSetpointValue + 1 and 100 inclusive. Store the value as MaxSetpointValue."),
            TestStep(8, "TH reads from the DUT the Step attribute.",
                        "Verify that the DUT response contains a value between 1 and 100 inclusive such that (MaxSetpointValue - MinSetpointValue) % value == 0. Store the value as StepValue."),
            TestStep(9, "TH reads from the DUT the UserSetpoint attribute.",
                        "Verify that the DUT response contains a value between MinSetpointValue and MaxSetpointValue inclusive such that (SetpointValue - MinSetpointValue) % StepValue == 0. Store the value as SetpointValue."),
            TestStep(10, "TH reads from the DUT the TargetSetpoint attribute.",
                     "Verify that the DUT response contains a value between MinSetpointValue and MaxSetpointValue inclusive."),
            TestStep(11, "TH reads from the DUT the MistType attribute.",
                     "Verify that the DUT response contains a value with at most the 2 least significant bits set."),
            TestStep(12, "TH reads from the DUT the Continuous attribute.",
                     "Verify that the DUT response contains a Boolean."),
            TestStep(13, "TH reads from the DUT the Sleep attribute.",
                     "Verify that the DUT response contains a Boolean."),
            TestStep(14, "TH reads from the DUT the Optimal attribute.",
                     "Verify that the DUT response contains a Boolean."),
            TestStep(15, "TH reads from the DUT the CondPumpEnabled attribute.",
                     "Verify that the DUT response contains a Boolean."),
            TestStep(16, "TH reads from the DUT the CondRunCount attribute.",
                     "Verify that the DUT response contains an unsigned integer value."),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_HSTAT_2_1(self):
        endpoint = self.get_endpoint()

        self.step(1)
        # Commission DUT to TH (can be skipped if done in a preceding test).
        await self.async_hstat_setup()

        self.step(2)
        # TH reads from the DUT the FeatureMap attribute. Already read above and implicitly checked.

        self.step(3)
        # TH reads from the DUT the SupportedModesMode attribute.
        SupportedModes = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.SupportedModes)
        # Verify:
        #   - Each list item is supported by the FeatureMap.
        #   - All modes required by the FeatureMap are in the list.
        # Verify that the DUT response contains a list with up to 4 entries.
        asserts.assert_is_instance(SupportedModes, list, "SupportedModes attribute must be a list")
        asserts.assert_less_equal(len(SupportedModes), 4, "SupportedModes list shall contain up to 4 entries")
        # Verify that each list item is unique among other entries.
        asserts.assert_equal(len(SupportedModes), len(set(SupportedModes)), "SupportedModes list contains duplicate entries")
        humidifierModeSupported = False
        dehumidifierModeSupported = False
        autoModeSupported = False
        fanOnlyModeSupported = False
        #   - Each list item corresponds to a valid ModeEnum value.
        for mode in SupportedModes:
            #   - Each list item is supported by the FeatureMap.
            match mode:
                case self.ModeEnum.kHumidifier:
                    humidifierModeSupported = True
                    asserts.assert_true(self.humidifierFeatureSupported, "Humidifier mode was supported while the feature was not")
                case self.ModeEnum.kDehumidifier:
                    dehumidifierModeSupported = True
                    asserts.assert_true(self.dehumidifierFeatureSupported, "Dehumidifier mode was supported while the feature was not")
                case self.ModeEnum.kAuto:
                    autoModeSupported = True
                    asserts.assert_true(self.autoFeatureSupported, "Auto mode was supported while the feature was not")
                case self.ModeEnum.kFanOnly:
                    fanOnlyModeSupported = True
                    asserts.assert_true(self.fanOnlyFeatureSupported, "FanOnly mode was supported while the feature was not")
                case _:
                    asserts.fail("Unknown mode value encountered in SupportModes")
            asserts.assert_greater_equal(mode, 0, "SupportedModes entry is out of range")
            asserts.assert_less_equal(mode, 3, "SupportedModes entry is out of range")
        asserts.assert_equal(humidifierModeSupported, self.humidifierFeatureSupported,
                                "Humidifier mode was supported while the feature was not")
        asserts.assert_equal(dehumidifierModeSupported, self.dehumidifierFeatureSupported,
                             "Dehumidifier mode was supported while the feature was not")
        asserts.assert_equal(autoModeSupported, self.autoFeatureSupported, "Auto mode was supported while the feature was not")
        asserts.assert_equal(fanOnlyModeSupported, self.fanOnlyFeatureSupported,
                             "FanOnly mode was supported while the feature was not")

        self.step(4)
        # TH reads from the DUT the Mode attribute.
        # Verify that the DUT response contains a value between 0 and 3 inclusive.
        dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Mode)
        asserts.assert_greater_equal(dut_Mode, 0, "Mode attribute is out of range")
        asserts.assert_less_equal(dut_Mode, 3, "Mode attributey is out of range")

        self.step(5)
        # TH reads from the DUT the SystemState attribute.
        # Verify that the DUT response contains a value between 0 and 3 inclusive.
        dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.SystemState)
        asserts.assert_greater_equal(dut_SystemState, 0, "SystemState attribute is out of range")
        asserts.assert_less_equal(dut_SystemState, 3, "SystemState attribute is out of range")

        self.step(6)
        # TH reads from the DUT the MinSetpoint attribute.
        # Verify that the DUT response contains a value between 0 and 99 inclusive. Store the value as MinSetpointValue
        if self.sensorFeatureSupported:
            dut_MinSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.MinSetpoint)
            asserts.assert_greater_equal(dut_MinSetpoint, 0, "MinSetpoint attribute is out of range")
            asserts.assert_less_equal(dut_MinSetpoint, 99, "MinSetpoint attribute is out of range")

        self.step(7)
        # TH reads from the DUT the MaxSetpoint attribute.
        # Verify that the DUT response contains a value between MinSetpointValue + 1 and 100 inclusive. Store the value as MaxSetpointValue.
        if self.sensorFeatureSupported:
            dut_MaxSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.MaxSetpoint)
            asserts.assert_greater_equal(dut_MaxSetpoint, dut_MinSetpoint+1, "MaxSetpoint attribute is out of range")
            asserts.assert_less_equal(dut_MaxSetpoint, 100, "MaxSetpoint attribute is out of range")

        self.step(8)
        # TH reads from the DUT the Step attribute.
        # Verify that the DUT response contains a value between 1 and 100 inclusive such that (MaxSetpointValue - MinSetpointValue) % value == 0. Store the value as StepValue.
        if self.sensorFeatureSupported:
            dut_Step = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Step)
            asserts.assert_greater_equal(dut_Step, 1, "Step attribute cannot be zero")
            asserts.assert_less_equal(dut_Step, 100, "Step attribute too large")
            asserts.assert_equal((dut_MaxSetpoint - dut_MinSetpoint) % dut_Step, 0,
                                 "Step attribute is not divisible by (MaxSetpoint - MinSetpoint)")

        self.step(9)
        # TH reads from the DUT the UserSetpoint attribute.
        # Verify that the DUT response contains a value between MinSetpointValue and MaxSetpointValue inclusive
        # such that (SetpointValue - MinSetpointValue) % StepValue == 0. Store the value as SetpointValue.
        if self.sensorFeatureSupported:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.UserSetpoint)
            asserts.assert_greater_equal(dut_UserSetpoint, dut_MinSetpoint, "UserSetpoint attribute is less than MinSetpoint")
            asserts.assert_less_equal(dut_UserSetpoint, dut_MaxSetpoint, "UserSetpoint attribute is greater than MaxSetpoint")
            asserts.assert_equal((dut_UserSetpoint - dut_MinSetpoint) % dut_Step, 0,
                                 "UserSetpoint attribute is not divisible by (MaxSetpoint - MinSetpoint)")

        self.step(10)
        # TH reads from the DUT the TargetSetpoint attribute.
        # Verify that the DUT response contains a value between MinSetpointValue and MaxSetpointValue inclusive.
        if self.optimalFeatureSupported or (self.sensorFeatureSupported and self.attributes.TargetSetpoint.attribute_id in self.supported_attributes):
            dut_TargetSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.TargetSetpoint)
            asserts.assert_greater_equal(dut_TargetSetpoint, dut_MinSetpoint, "TargetSetpoint attribute is less than MinSetpoint")
            asserts.assert_less_equal(dut_TargetSetpoint, dut_MaxSetpoint, "TargetSetpoint attribute is greater than MaxSetpoint")

        self.step(11)
        # TH reads from the DUT the MistType attribute.
        # Verify that the DUT response contains a value with at most the 2 least significant bits set.
        if self.humidifierFeatureSupported:
            dut_MistType = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.MistType)
            log.info("MistType is %s", dut_MistType)
            if dut_MistType != NullValue:
                asserts.assert_greater_equal(dut_MistType, 1, "MistType attribute out of range")
                asserts.assert_less_equal(dut_MistType, 2, "MistType attribute out of range")

        self.step(12)
        # TH reads from the DUT the Continuous attribute.
        # Verify that the DUT response contains a Boolean.
        if self.continuousFeatureSupported:
            dut_Continuous = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Continuous)
            asserts.assert_true(isinstance(dut_Continuous, bool), "Continuous attribute must be a Boolean")

        self.step(13)
        # TH reads from the DUT the Sleep attribute.
        # Verify that the DUT response contains a Boolean.
        if self.attributes.Sleep.attribute_id in self.supported_attributes:
            dut_Sleep = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Sleep)
            asserts.assert_true(isinstance(dut_Sleep, bool), "Sleep attribute must be a Boolean")

        self.step(14)
        # TH reads from the DUT the Optimal attribute.
        # Verify that the DUT response contains a Boolean.
        if self.optimalFeatureSupported:
            dut_Optimal = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.Optimal)
            asserts.assert_true(isinstance(dut_Optimal, bool), "Optimal attribute must be a Boolean")

        self.step(15)
        # TH reads from the DUT the CondPumpEnabled attribute.
        # Verify that the DUT response contains a Boolean.
        if self.condPumpFeatureSupported:
            dut_CondPumpEnabled = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.CondPumpEnabled)
            asserts.assert_true(isinstance(dut_CondPumpEnabled, bool), "CondPumpEnabled attribute must be a Boolean")

        self.step(16)
        # TH reads from the DUT the CondRunCount attribute.
        # Verify that the DUT response contains an unsigned integer value.
        if self.condPumpFeatureSupported:
            dut_CondRunCount = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=self.attributes.CondRunCount)
            asserts.assert_greater_equal(dut_CondRunCount, 0, "CondRunCount attribute out of range")

if __name__ == "__main__":
    default_matter_test_main()
