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


from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

import logging
log = logging.getLogger(__name__)


class TC_HSTAT_2_1(MatterBaseTest):
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

    async def read_hstat_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.Humidistat
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_hstat_cmd_expect_success(self, endpoint, command) -> None:
        await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=1000)

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_HSTAT_2_1(self):
        endpoint = self.get_endpoint()

        self.step(1)
        # Commission DUT to TH (can be skipped if done in a preceding test).
        cluster = Clusters.Humidistat
        attributes = cluster.Attributes
        features = cluster.Bitmaps.Feature
        mistBitmap = cluster.Bitmaps.Mist
        supported_attributes = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)
        SetSettings = Clusters.Humidistat.Commands.SetSettings

        feature_map = await self.read_setting(attributes.FeatureMap)
        humidifierFeatureSupported = bool(feature_map & features.kHumidifier)
        dehumidifierFeatureSupported = bool(feature_map & features.kDehumidifier)
        continuousFeatureSupported = bool(feature_map & features.kContinuous)
        sensorFeatureSupported = bool(feature_map & features.kSensor)
        autoFeatureSupported = bool(feature_map & features.kAuto)
        fanOnlyFeatureSupported = bool(feature_map & features.kFan)
        optimalFeatureSupported = bool(feature_map & features.kOptimal)
        warmFeatureSupported = bool(feature_map & features.kWarmMist)
        coldFeatureSupported = bool(feature_map & features.kColdMist)
        condPumpFeatureSupported = bool(feature_map & features.kCondPump)
        log.info("DUT supports the Humidifier feature: %s", humidifierFeatureSupported)
        log.info("DUT supports the Dehumidifier feature: %s", dehumidifierFeatureSupported)
        log.info("DUT supports the Continuous feature: %s", continuousFeatureSupported)
        log.info("DUT supports the Sensor feature: %s", sensorFeatureSupported)
        log.info("DUT supports the Auto feature: %s", autoFeatureSupported)
        log.info("DUT supports the Fan feature: %s", fanOnlyFeatureSupported)
        log.info("DUT supports the Optimal feature: %s", optimalFeatureSupported)
        log.info("DUT supports the Warm feature: %s", warmFeatureSupported)
        log.info("DUT supports the Cold feature: %s", coldFeatureSupported)
        log.info("DUT supports the CondPump feature: %s", condPumpFeatureSupported)

        # some convenience definions
        modeHumidifier = cluster.Enums.ModeEnum.kHumidifier
        modeDehumidifier = cluster.Enums.ModeEnum.kDeumidifier
        modeAuto = cluster.Enums.ModeEnum.kAuto
        modeFanOnly = cluster.Enums.ModeEnum.kFanOnly
        stateHumidifying = cluster.Enums.SystemStateEnum.kHumidifying
        stateDehumidifying = cluster.Enums.SystemStateEnum.kDehumidifying
        stateFan = cluster.Enums.SystemStateEnum.kFan
        stateIdle = cluster.Enums.SystemStateEnum.kIdle

        self.step(2)
        # TH reads from the DUT the FeatureMap attribute. Already read above and implicitly checked.

        self.step(3)
        # TH reads from the DUT the SupportedModesMode attribute.
        SupportedModes = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SupportedModes)
        # Verify:
        #   - Each list item is supported by the FeatureMap.
        #   - All modes required by the FeatureMap are in the list.
        # Verify that the DUT response contains a list with up to 4 entries.
        asserts.assert_is_instance(SupportedModes, list, "SupportedModes attribute must be a list")
        asserts.assert_less_equal(len(SupportedModes), 4, "SupportedModes list shall contain up to 4 entries")
        # Verify that each list item is unique among other entries.
        asserts.assert_equal(len(SupportedModes), len(set(SupportedModes)), "SupportedModes list contains duplicate entries")
        humidifierModeSupport = False
        dehumidifierModeSupported = False
        autoModeSupported = False
        fanOnlyModeSupported = False
        #   - Each list item corresponds to a valid ModeEnum value.
        for mode in SupportedModes:
            #   - Each list item is supported by the FeatureMap.
            match mode:
                case modeHumidifier:
                    humidifierModeSupported = True
                    asserts.assert_true(humidifierFeatureSupported, "Humidifier mode was supported while the feature was not")
                case modeDehumidifier:
                    dehumidifierModeSupported = True
                    asserts.assert_true(dehumidifierFeatureSupported, "Dehumidifier mode was supported while the feature was not")
                case modeAuto:
                    autoModeSupported = True
                    asserts.assert_true(autoFeatureSupported, "Auto mode was supported while the feature was not")
                case modeFanOnly:
                    fanOnlyModeSupported = True
                    asserts.assert_true(fanOnlyFeatureSupported, "FanOnly mode was supported while the feature was not")
            asserts.assert_greater_equal(mode, 0, "SupportedModes entry is out of range")
            asserts.assert_less_equal(mode, 3, "SupportedModes entry is out of range")
        

        self.step(4)
        # TH reads from the DUT the Mode attribute.
        # Verify that the DUT response contains a value between 0 and 3 inclusive.
        dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
        asserts.assert_greater_equal(dut_Mode, 0, "Mode attribute is out of range")
        asserts.assert_less_equal(dut_Mode, 3, "Mode attributey is out of range")

        self.step(5)
        # TH reads from the DUT the SystemState attribute.
        # Verify that the DUT response contains a value between 0 and 3 inclusive.
        dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
        asserts.assert_greater_equal(dut_SystemState, 0, "SystemState attribute is out of range")
        asserts.assert_less_equal(dut_SystemState, 3, "SystemState attribute is out of range")

        self.step(6)
        # TH reads from the DUT the MinSetpoint attribute.
        # Verify that the DUT response contains a value between 0 and 99 inclusive. Store the value as MinSetpointValue
        if sensorFeatureSupported:
            dut_MinSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MinSetpoint)
            asserts.assert_greater_equal(dut_MinSetpoint, 0, "MinSetpoint attribute is out of range")
            asserts.assert_less_equal(dut_MinSetpoint, 99, "MinSetpoint attribute is out of range")

        self.step(7)
        # TH reads from the DUT the MaxSetpoint attribute.
        # Verify that the DUT response contains a value between MinSetpointValue + 1 and 100 inclusive. Store the value as MaxSetpointValue.
        if sensorFeatureSupported:
            dut_MaxSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MaxSetpoint)
            asserts.assert_greater_equal(dut_MaxSetpoint, dut_MinSetpoint+1, "MaxSetpoint attribute is out of range")
            asserts.assert_less_equal(dut_MaxSetpoint, 100, "MaxSetpoint attribute is out of range")

        self.step(8)
        # TH reads from the DUT the Step attribute.
        # Verify that the DUT response contains a value between 1 and 100 inclusive such that (MaxSetpointValue - MinSetpointValue) % value == 0. Store the value as StepValue.
        if sensorFeatureSupported:
            dut_Step = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Step)
            asserts.assert_greater_equal(dut_Step, 1, "Step attribute cannot be zero")
            asserts.assert_less_equal(dut_Step, 100, "Step attribute too large")
            asserts.assert_equal((dut_MaxSetpoint - dut_MinSetpoint) % dut_Step, 0,
                                 "Step attribute is not divisible by (MaxSetpoint - MinSetpoint)")

        self.step(9)
        # TH reads from the DUT the UserSetpoint attribute.
        # Verify that the DUT response contains a value between MinSetpointValue and MaxSetpointValue inclusive
        # such that (SetpointValue - MinSetpointValue) % StepValue == 0. Store the value as SetpointValue.
        if sensorFeatureSupported:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_greater_equal(dut_UserSetpoint, dut_MinSetpoint, "UserSetpoint attribute is less than MinSetpoint")
            asserts.assert_less_equal(dut_UserSetpoint, dut_MaxSetpoint, "UserSetpoint attribute is greater than MaxSetpoint")
            asserts.assert_equal((dut_UserSetpoint - dut_MinSetpoint) % dut_Step, 0,
                                 "UserSetpoint attribute is not divisible by (MaxSetpoint - MinSetpoint)")

        self.step(10)
        # TH reads from the DUT the TargetSetpoint attribute.
        # Verify that the DUT response contains a value between MinSetpointValue and MaxSetpointValue inclusive.
        if optimalFeatureSupported or (sensorFeatureSupported and attributes.TargetSetpoint.attribute_id in supported_attributes):
            dut_TargetSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetSetpoint)
            asserts.assert_greater_equal(dut_TargetSetpoint, dut_MinSetpoint, "TargetSetpoint attribute is less than MinSetpoint")
            asserts.assert_less_equal(dut_TargetSetpoint, dut_MaxSetpoint, "TargetSetpoint attribute is greater than MaxSetpoint")

        self.step(11)
        # TH reads from the DUT the MistType attribute.
        # Verify that the DUT response contains a value with at most the 2 least significant bits set.
        if humidifierFeatureSupported:
            dut_MistType = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MistType)
            asserts.assert_greater_equal(dut_MistType, 1, "MistType attribute out of range")
            asserts.assert_less_equal(dut_MistType, 2, "MistType attribute out of range")

        self.step(12)
        # TH reads from the DUT the Continuous attribute.
        # Verify that the DUT response contains a Boolean.
        if continuousFeatureSupported:
            dut_Continuous = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Continuous)
            asserts.assert_true(isinstance(dut_Continuous, bool), "Continuous attribute must be a Boolean")

        self.step(13)
        # TH reads from the DUT the Sleep attribute.
        # Verify that the DUT response contains a Boolean.
        if attributes.Sleep.attribute_id in supported_attributes:
            dut_Sleep = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Sleep)
            asserts.assert_true(isinstance(dut_Sleep, bool), "Sleep attribute must be a Boolean")

        self.step(14)
        # TH reads from the DUT the Optimal attribute.
        # Verify that the DUT response contains a Boolean.
        if optimalFeatureSupported:
            dut_Optimal = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Optimal)
            asserts.assert_true(isinstance(dut_Optimal, bool), "Optimal attribute must be a Boolean")

        self.step(15)
        # TH reads from the DUT the CondPumpEnabled attribute.
        # Verify that the DUT response contains a Boolean.
        if condPumpFeatureSupported:
            dut_CondPumpEnabled = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.CondPumpEnabled)
            asserts.assert_true(isinstance(dut_CondPumpEnabled, bool), "CondPumpEnabled attribute must be a Boolean")

        self.step(16)
        # TH reads from the DUT the CondRunCount attribute.
        # Verify that the DUT response contains an unsigned integer value.
        if condPumpFeatureSupported:
            dut_CondRunCount = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.CondRunCount)
            asserts.assert_greater_equal(dut_CondRunCount, 0, "CondRunCount attribute out of range")

        # move everything below this line

        self.step(next(step))  # Set Mode to Off
        await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mode=modeOff))

        self.step(next(step))  # Read Mode, should be Off
        dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
        asserts.assert_equal(dut_Mode, modeOff, "Mode attribute is not Off")

        self.step(next(step))  # Read SystemState, should be Off
        dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
        asserts.assert_equal(dut_SystemState, stateOff, "SystemState attribute is not Off")

        self.step(next(step))  # Set Mode to Humidifier
        if humidifierFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mode=modeHumidifier))

        self.step(next(step))  # Read Mode, should be Humidifier
        if humidifierFeatureSupported:
            dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
            asserts.assert_equal(dut_Mode, modeHumidifier, "Mode attribute is not Humidifier")

        self.step(next(step))  # Read SystemState, should be Idle or Humidifying
        if humidifierFeatureSupported:
            dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
            asserts.true(dut_SystemState in [stateIdle, stateHumidifying], "SystemState attribute is not Idle or Humidifying")
    
        self.step(next(step))  # Set Mode to Dehumidifier
        if dehumidifierFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mode=modeDehumidifier))

        self.step(next(step))  # Read Mode, should be Dehmidifier
        if dehumidifierFeatureSupported:
            dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
            asserts.assert_equal(dut_Mode, modeDehumidifier, "Mode attribute is not Dehumidifier")

        self.step(next(step))  # Read SystemState, should be Idle or Dehumidifying
        if humidifierFeatureSupported:
            dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
            asserts.true(dut_SystemState in [stateIdle, stateDehumidifying], "SystemState attribute is not Idle or Dehumidifying")

        self.step(next(step))  # Set Mode to Auto
        if autoFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mode=modeAuto))

        self.step(next(step))  # Read Mode, should be Auto
        if autoFeatureSupported:
            dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
            asserts.assert_equal(dut_Mode, modeAuto, "Mode attribute is not Auto")

        self.step(next(step))  # Read SystemState, should be Idle, Humidifying, or Dehumidifying
        if autoFeatureSupported:
            dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
            asserts.assert_true(dut_SystemState in [stateIdle, stateHumidifying, stateDehumidifying],
                                "SystemState attribute is not Idle, Humidifying, or Dehumifying")

        self.step(next(step))  # Set Mode to FanOnly
        if fanOnlyFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mode=modeFanOnly))

        self.step(next(step))  # Read Mode, should be FanOnly
        if fanOnlyFeatureSupported:
            dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
            asserts.assert_equal(dut_Mode, modeFanOnly, "Mode attribute is not FanOnly")

        self.step(next(step))  # Read SystemState, should be Fan
        if fanOnlyFeatureSupported:
            dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
            asserts.assert_equal(dut_SystemState, stateFan, "SystemState attribute is not Fan")

        self.step(next(step))  # Set Mode to Humidifier or Dehumidifier
        if humidifierFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mode=modeHumidifier))
        else:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mode=modeDehumidifier))

        self.step(next(step))  # Set MinSetpoint to UserSetpoint
        if sensorFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(userSetpoint=dut_MinSetpoint))

        self.step(next(step))  # Read UserSetpoint, confirm it is MinSetpoint
        if sensorFeatureSupported:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_equal(dut_UserSetpoint, dut_MinSetpoint, "UserSetpoint attribute not equal to MinSetpoint attribute")

        self.step(next(step))  # Set MaxSetpoint to UserSetpoint
        if sensorFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(userSetpoint=dut_MaxSetpoint))

        self.step(next(step))  # Read UserSetpoint, confirm it is MaxSetpoint
        if sensorFeatureSupported:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_equal(dut_UserSetpoint, dut_MaxSetpoint, "UserSetpoint attribute not equal to MaxSetpoint attribute")

        testUserValue = dut_MinSetpoint + dut_Step

        self.step(next(step))  # Set test value to UserSetpoint
        if sensorFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(userSetpoint=testUserValue))

        self.step(next(step))  # Read UserSetpoint, confirm it is test value
        if sensorFeatureSupported:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_equal(dut_UserSetpoint, testUserValue, "UserSetpoint attribute not as expected")

        self.step(next(step))  # Read Continuous attribute
        if continuousFeatureSupported:
            dut_Continuous = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Continuous)

        testContinuousValue = not dut_Continuous

        self.step(next(step))  # Set opposite value to Continuous
        if continuousFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(continuous=testContinuousValue))

        self.step(next(step))  # Read Continuous attribute and check value
        if continuousFeatureSupported:
            dut_Continuous = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Continuous)
            asserts.assert_equal(dut_Continuous, testContinuousValue, "Continuous attribute not as expected")

        self.step(next(step))  # Set Continuous to False
        if continuousFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(continuous=False))

        self.step(next(step))  # Read Sleep attribute
        if attributes.Sleep.attribute_id in supported_attributes:
            dut_Sleep = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Sleep)

        testSleepValue = not dut_Sleep

        self.step(next(step))  # Set opposite value to Sleep
        if attributes.Sleep.attribute_id in supported_attributes:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(sleep=testSleepValue))

        self.step(next(step))  # Read Sleep attribute and check value
        if attributes.Sleep.attribute_id in supported_attributes:
            dut_Sleep = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Sleep)
            asserts.assert_equal(dut_Sleep, testSleepValue, "Sleep attribute not as expected")

        self.step(next(step))  # Set Sleep to False
        if attributes.Sleep.attribute_id in supported_attributes:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(sleep=False))

        self.step(next(step))  # Read Optimal attribute
        if optimalFeatureSupported:
            dut_Optimal = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Optimal)

        testOptimalValue = not dut_Optimal

        self.step(next(step))  # Set opposite value to Optimal
        if optimalFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(optimal=testOptimalValue))

        self.step(next(step))  # Read Optimal attribute and check value
        if optimalFeatureSupported:
            dut_Optimal = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Optimal)
            asserts.assert_equal(dut_Optimal, testOptimalValue, "Optimal attribute not as expected")

        self.step(next(step))  # Set Optimal to False
        if optimalFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(optimal=False))

        self.step(next(step))  # Set Mode to Humidifier
        if humidifierFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mode=modeHumidifier))

        self.step(next(step))  # Set Warm to MistType
        if humidifierFeatureSupported and warmFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mistType=mistBitmap.kWarmMist))

        self.step(next(step))  # Check that MistType attribute is Warm
        if humidifierFeatureSupported and warmFeatureSupported:
            dut_MistType = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MistType)
            asserts.assert_true(bool(dut_MistType & mistBitmap.kWarmMist), "MistType not Warm")

        self.step(next(step))  # Set Cold to MistType
        if humidifierFeatureSupported and coldFeatureSupported:
            await self.send_hstat_cmd_expect_success(endpoint=endpoint, command=SetSettings(mistType=mistBitmap.kColdMist))

        self.step(next(step))  # Check that MistType attribute is Cold
        if humidifierFeatureSupported and coldFeatureSupported:
            dut_MistType = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MistType)
            asserts.assert_true(bool(dut_MistType & mistBitmap.kColdMist), "MistType not Cold")

if __name__ == "__main__":
    default_matter_test_main()
