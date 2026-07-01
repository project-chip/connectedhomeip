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
from itertools import count

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_HSTAT_2_1(MatterBaseTest):
    def pics_TC_BOOLCFG_2_1(self) -> list[str]:
        return [
            "HSTAT.S",
        ]

    def desc_TC_HSTAT_2_1(self) -> str:
        return "[TC-HSTAT-2.1] Primary functionality with DUT as Server"

    def steps_TC_FAN_2_1(self):
        step = count(1)
        return [TestStep(next(step), "Commissioning already done.", is_commissioning=True),
                TestStep(next(step), "TH reads from the DUT the Mode attribute.",
                         "Verify that the DUT response contains a value between 0 and 4 inclusive."),
                TestStep(next(step), "TH reads from the DUT the SystemState attribute.",
                         "Verify that the DUT response contains a value between 0 and 4 inclusive."),
                TestStep(next(step), "TH reads from the DUT the FeatureMap attribute.",
                         "Verify that the DUT response contains the FeatureMap attribute. Store the value as FeatureMap. " + \
                         "Confirm that one or both of HSTAT.S.F00(HUM) or HSTAT.S.F01(DEHUM) are set. " + \
                         "Confirm that one or both of HSTAT.S.F02(CONT) or HSTAT.S.F03(SENSOR) are set. " + \
                         "If HSTAT.S.F04(AUTO) is set, confirm that HSTAT.S.F00(HUM), HSTAT.S.F01(DEHUM), and HSTAT.S.F03(SENSOR) are all set. " + \
                         "If HSTAT.S.F07(OPT) is set, confirm that HSTAT.S.F03(SENSOR) is set. " + \
                         "If HSTAT.S.F00(HUM) is set, confirm that one or both of HSTAT.S.F08(WARM) and HSTAT.S.F09(COLD) are set."),
                TestStep(next(step), "TH reads from the DUT the MinSetpoint attribute.",
                         "Verify that the DUT response contains a value between 0 and 99 inclusive. " + \
                         "Store the value as MinSetpointValue"),
                TestStep(next(step), "TH reads from the DUT the MaxSetpoint attribute.",
                         "Verify that the DUT response contains a value between MinSetpointValue and 100. " + \
                         "Store the value as MaxSetpointValue."),
                TestStep(next(step), "TH reads from the DUT the Step attribute.",
                         "Store the value as StepValue. " + \
                         "StepValue is between 1 and (MaxSetpointValue - MinSetpointValue) inclusive. " + \
                         "StepValue is such that (MaxSetpointValue - MinSetpointValue) % value == 0."),
                TestStep(next(step), "Store the value as StepValue.",
                         "Store the value as SetpointValue. " + \
                         "SetpointValue is between MinSetpointValue and MaxSetpointValue inclusive. " + \
                         "SetpointValue is such that (SetpointValue - MinSetpointValue) % StepValue == 0."),
                TestStep(next(step), "Store the value as SetpointValue.",
                         "Verify that the DUT response contains a value between MinSetpointValue and MaxSetpointValue inclusive."),
                TestStep(next(step), "TH reads from the DUT the MistType attribute.",
                         "Verify that the DUT response contains a value or 0 or 1"),
                TestStep(next(step), "TH reads from the DUT the Continuous attribute.",
                         "Verify that the DUT response contains a value or True or False."),
                TestStep(next(step), "Verify that the DUT response contains a value or True or False",
                         "Verify that the DUT response contains a value or True or False."),
                TestStep(next(step), "TH reads from the DUT the Optimal attribute.",
                         "Verify that the DUT response contains a value or True or False"),
                TestStep(next(step), "TH writes to the DUT the Mode attribute with a value of Off",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the Mode attribute.",
                         "Verify that the DUT response contains Off."),
                TestStep(next(step), "TH reads from the DUT the SystemState attribute.",
                         "Verify that the DUT response contains Off."),
                TestStep(next(step), "TH writes to the DUT the Mode attribute with a value of Humidifier"
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the Mode attribute.",
                         "Verify that the DUT response contains Humidifier."),
                TestStep(next(step), "TH reads from the DUT the SystemState attribute.",
                         "Verify that the DUT response contains Humidifying."),
                TestStep(next(step), "TH writes to the DUT the Mode attribute with a value of Dehumidifier",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the Mode attribute.",
                         "Verify that the DUT response contains Dehumidifier."),
                TestStep(next(step), "TH reads from the DUT the SystemState attribute.",
                         "Verify that the DUT response contains Dehumidifying."),
                TestStep(next(step), "TH writes to the DUT the Mode attribute with a value of Auto",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the Mode attribute.",
                         "Verify that the DUT response is Auto."),
                TestStep(next(step), "TH reads from the DUT the SystemState attribute.",
                         "Verify that the DUT response contains Idle, Humidifying, or Dehumidifying."),
                TestStep(next(step), "TH writes to the DUT the Mode attribute with a value of FanOnly",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the Mode attribute.",
                         "Verify that the DUT response is FanOnly."),
                TestStep(next(step), "TH reads from the DUT the SystemState attribute.",
                         "Verify that the DUT response contains Fan."),
                TestStep(next(step), "TH writes to the DUT the Mode attribute with a value of Humidifier or Dehumidifier",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH writes to the DUT the UserSetpoint attribute with value MinSetpointValue."
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the UserSetpoint attribute.",
                         "Verify that the DUT response contains MinSetpointValue."),
                TestStep(next(step), "TH writes to the DUT the UserSetpoint attribute with value MaxSetpointValue."
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the UserSetpoint attribute.",
                         "Verify that the DUT response contains MaxSetpointValue."),
                TestStep(next(step), "TH writes to the DUT the UserSetpoint attribute with value MinSetpointValue + StepValue.",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the UserSetpoint attribute.",
                         "Verify that the DUT response contains MinSetpointValue + StepValue."),
                TestStep(next(step), "TH reads from the DUT the Continuous attribute."
                         "Store the value as ContState"),
                TestStep(next(step), "TH writes to the DUT the Continuous attribute with a value of !ContState.",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the Continuous attribute.",
                         "Verify that the DUT response contains !ContState."),
                TestStep(next(step), "TH writes to the DUT the Continuous attribute with a value of False.",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the Sleep attribute.",
                         "Store the value as SleepState"),
                TestStep(next(step), "TH writes to the DUT the Sleep attribute with a value of !SleepState.",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the Sleep attribute.",
                         "Verify that the DUT response contains !SleepState."),
                TestStep(next(step), "TH writes to the DUT the Sleep attribute with a value of False.",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the Optimal attribute.",
                         "Store the value as OptState"),
                TestStep(next(step), "TH writes to the DUT the Optimal attribute with a value of !OptState.",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "Store the value as OptState",
                         "Verify that the DUT response contains !OptState."),
                TestStep(next(step), "TH writes to the DUT the Optimal attribute with a value of False.",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH writes to the DUT the UserSetpoint attribute with value MinSetpointValue + 1.",
                         "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
                TestStep(next(step), "TH writes to the DUT the UserSetpoint attribute with value MinSetpointValue - 1.",
                         "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
                TestStep(next(step), "TH writes to the DUT the UserSetpoint attribute with value MaxSetpointValue + 1.",
                         "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
                TestStep(next(step), "TH writes to the DUT the Mode attribute with a value of Humidifier",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH writes to the DUT the MistType attribute with a value of MistWarm",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the MistType attribute.",
                         "Verify that the DUT response contains MistWarm."),
                TestStep(next(step), "TH writes to the DUT the MistType attribute with a value of MistCold",
                         "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(next(step), "TH reads from the DUT the MistType attribute.",
                         "Verify that the DUT response contains a value of MistCold."),
                TestStep(next(step), "TH writes to the DUT the MistType attribute with a value of MistWarm.",
                         "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
                TestStep(next(step), "TH writes to the DUT the MistType attribute with a value of MistCold.",
                         "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
                ]

    async def read_hstat_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.Humidistat
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_HSTAT_2_1(self):
        step = count(1)

        endpoint = self.get_endpoint()

        self.step(next(step))
        # Commissioning already done.
        cluster = Clusters.Humidistat
        attributes = cluster.Attributes
        features = cluster.Bitmaps.Feature
        mistBitmap = cluster.Bitmaps.Mist
        supported_attributes = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        feature_map = await self.read_setting(attributes.FeatureMap)
        supports_humidifier = bool(feature_map & features.kHumidifier)
        supports_dehumidifier = bool(feature_map & features.kDehumidifier)
        supports_continuous = bool(feature_map & features.kContinuous)
        supports_sensor = bool(feature_map & features.kSensor)
        supports_auto = bool(feature_map & features.kAuto)
        supports_fan = bool(feature_map & features.kFan)
        supports_optimal = bool(feature_map & features.kOptimal)
        supports_warm = bool(feature_map & features.kWarmMist)
        supports_cold = bool(feature_map & features.kColdMist)
        log.info("DUT supports the Humidifier feature: %s", supports_humidifier)
        log.info("DUT supports the Dehumidifier feature: %s", supports_dehumidifier)
        log.info("DUT supports the Continuous feature: %s", supports_continuous)
        log.info("DUT supports the Sensor feature: %s", supports_sensor)
        log.info("DUT supports the Auto feature: %s", supports_auto)
        log.info("DUT supports the Fan feature: %s", supports_fan)
        log.info("DUT supports the Optimal feature: %s", supports_optimal)
        log.info("DUT supports the Warm feature: %s", supports_warm)
        log.info("DUT supports the Cold feature: %s", supports_cold)

        # some convenience definions
        modeOff = cluster.Enums.ModeEnum.kOff
        modeHumidifier = cluster.Enums.ModeEnum.kHumidifier
        modeDehumidifier = cluster.Enums.ModeEnum.kDeumidifier
        modeAuto = cluster.Enums.ModeEnum.kAuto
        modeFanOnly = cluster.Enums.ModeEnum.kFanOnly
        stateOff = cluster.Enums.SystemStateEnum.kOff
        stateHumidifying = cluster.Enums.SystemStateEnum.kHumidifying
        stateDehumidifying = cluster.Enums.SystemStateEnum.kDehumidifying
        stateFan = cluster.Enums.SystemStateEnum.kFan
        stateIdle = cluster.Enums.SystemStateEnum.kIdle

        self.step(next(step))  # Check Mode attribute
        dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
        asserts.assert_greater_equal(dut_Mode, 0, "Mode attribute is out of range")
        asserts.assert_less_equal(dut_Mode, 4, "Mode attribute is out of range")

        self.step(next(step))  # Check SystemState attribute
        dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
        asserts.assert_greater_equal(dut_SystemState, 0, "SystemState attribute is out of range")
        asserts.assert_less_equal(dut_SystemState, 4, "SystemState attribute is out of range")

        self.step(next(step))  # Verify feature map
        asserts.assert_true(supports_humidifier or supports_dehumidifier, "Must support Humidifier and/or Dehumidifier")
        asserts.assert_true(supports_continuous or supports_sensor, "Must support Continuous and/or Sensor")
        if supports_auto:
            asserts.assert_true(supports_humidifier and supports_dehumidifier and supports_sensor,
                                "Must support Humidifier, Dehumidifier, and Sensor if Auto is supported")
        if supports_optimal:
            asserts.assert_true(supports_sensor, "Must support Sensor if Optimal is supported")
        if supports_humidifier:
            asserts.assert_true(supports_warm or supports_cold, "Must support Warm and/or Cold")

        self.step(next(step))  # Check MinSetpoint attribute
        if supports_sensor:
            dut_MinSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MinSetpoint)
            asserts.assert_greater_equal(dut_MinSetpoint, 0, "MinSetpoint attribute is out of range")
            asserts.assert_less_equal(dut_MinSetpoint, 99, "MinSetpoint attribute is out of range")

        self.step(next(step))  # Check MaxSetpoint attribute
        if supports_sensor:
            dut_MaxSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MaxSetpoint)
            asserts.assert_greater_equal(dut_MaxSetpoint, dut_MinSetpoint, "MaxSetpoint attribute is out of range")
            asserts.assert_less_equal(dut_MaxSetpoint, 100, "MaxSetpoint attribute is out of range")

        self.step(next(step))  # Check Step attribute
        if supports_sensor:
            dut_Step = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Step)
            asserts.assert_greater_equal(dut_Step, 1, "Step attribute cannot be zero")
            asserts.assert_less_equal(dut_Step, dut_MaxSetpoint - dut_MinSetpoint, "Step attribute is greater than MaxSetpoint")
            asserts.assert_equal(dut_Step % (dut_MaxSetpoint - dut_MinSetpoint), 0,
                                 "Step attribute is not divisible by (MaxSetpoint - MinSetpoint)")

        self.step(next(step))  # Check UserSetpoint attribute
        if supports_sensor:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_greater_equal(dut_UserSetpoint, dut_MinSetpoint, "UserSetpoint attribute is less than MinSetpoint")
            asserts.assert_less_equal(dut_UserSetpoint, dut_MaxSetpoint, "UserSetpoint attribute is greater than MaxSetpoint")
            asserts.assert_equal(dut_UserSetpoint % (dut_MaxSetpoint - dut_MinSetpoint), 0,
                                 "UserSetpoint attribute is not divisible by (MaxSetpoint - MinSetpoint)")

        self.step(next(step))  # Check TargetSetpoint attribute
        if supports_optimal or (supports_sensor and attributes.TargetSetpoint.attribute_id in supported_attributes):
            dut_TargetSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetSetpoint)
            asserts.assert_greater_equal(dut_TargetSetpoint, dut_MinSetpoint, "TargetSetpoint attribute is less than MinSetpoint")
            asserts.assert_less_equal(dut_TargetSetpoint, dut_MaxSetpoint, "TargetSetpoint attribute is greater than MaxSetpoint")
            asserts.assert_equal(dut_TargetSetpoint % (dut_MaxSetpoint - dut_MinSetpoint), 0,
                                 "TargetSetpoint attribute is not divisible by (MaxSetpoint - MinSetpoint)")

        self.step(next(step))  # Check MistType attribute
        if supports_humidifier:
            dut_MistType = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MistType)
            asserts.assert_greater_equal(dut_MistType, 0, "MistType attribute must be 0 or 1")
            asserts.assert_less_equal(dut_MistType, 1, "MistType attribute must be 0 or 1")

        self.step(next(step))  # Check Continuous attribute
        if supports_continuous:
            dut_Continuous = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Continuous)
            asserts.assert_true(dut_Continuous or not dut_Continuous, "Continuous attribute must be True or False")

        self.step(next(step))  # Check Sleep attribute
        if attributes.Sleep.attribute_id in supported_attributes:
            dut_Sleep = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Sleep)
            asserts.assert_true(dut_Sleep or not dut_Sleep, "Sleep attribute must be True or False")

        self.step(next(step))  # Check Optimal attribute
        if supports_optimal:
            dut_Optimal = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Optimal)
            asserts.assert_true(dut_Optimal or not dut_Optimal, "Optimal attribute must be True or False")

        self.step(next(step))  # Write Mode to Off
        await self.write_single_attribute(attribute_value=attributes.Mode(modeOff), endpoint_id=endpoint)

        self.step(next(step))  # Read Mode, should be Off
        dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
        asserts.assert_equal(dut_Mode, modeOff, "Mode attribute is not Off")

        self.step(next(step))  # Read SystemState, should be Off
        if supports_humidifier:
            dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
            asserts.assert_equal(dut_SystemState, stateOff, "SystemState attribute is not Humidifying")

        self.step(next(step))  # Write Mode to Humidifier
        if supports_humidifier:
            await self.write_single_attribute(attribute_value=attributes.Mode(modeHumidifier), endpoint_id=endpoint)

        self.step(next(step))  # Read Mode, should be Humidifier
        if supports_humidifier:
            dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
            asserts.assert_equal(dut_Mode, modeHumidifier, "Mode attribute is not Humidifier")

        self.step(next(step))  # Read SystemState, should be Humidifying
        if supports_humidifier:
            dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
            asserts.assert_equal(dut_SystemState, stateHumidifying, "SystemState attribute is not Humidifying")

        self.step(next(step))  # Write Mode to Dehumidifier
        if supports_dehumidifier:
            await self.write_single_attribute(attribute_value=attributes.Mode(modeDehumidifier), endpoint_id=endpoint)

        self.step(next(step))  # Read Mode, should be Dehmidifier
        if supports_dehumidifier:
            dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
            asserts.assert_equal(dut_Mode, modeDehumidifier, "Mode attribute is not Dehumidifier")

        self.step(next(step))  # Read SystemState, should be Dehumidifying
        if supports_humidifier:
            dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
            asserts.assert_equal(dut_SystemState, stateDehumidifying, "SystemState attribute is not Dehumidifying")

        self.step(next(step))  # Write Mode to Auto
        if supports_auto:
            await self.write_single_attribute(attribute_value=attributes.Mode(modeAuto), endpoint_id=endpoint)

        self.step(next(step))  # Read Mode, should be Auto
        if supports_auto:
            dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
            asserts.assert_equal(dut_Mode, modeAuto, "Mode attribute is not Auto")

        self.step(next(step))  # Read SystemState, should be Idle, Humidifying, or Dehumidifying
        if supports_auto:
            dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
            asserts.assert_true(dut_SystemState in [stateIdle, stateHumidifying, stateDehumidifying],
                                "SystemState attribute is not Idle, Humidifying, or Dehumifying")

        self.step(next(step))  # Write Mode to FanOnly
        if supports_fan:
            await self.write_single_attribute(attribute_value=attributes.Mode(modeFanOnly), endpoint_id=endpoint)

        self.step(next(step))  # Read Mode, should be FanOnly
        if supports_fan:
            dut_Mode = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Mode)
            asserts.assert_equal(dut_Mode, modeFanOnly, "Mode attribute is not FanOnly")

        self.step(next(step))  # Read SystemState, should be Fan
        if supports_fan:
            dut_SystemState = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.SystemState)
            asserts.assert_equal(dut_SystemState, stateFan, "SystemState attribute is not Fan")

        self.step(next(step))  # Write Mode to Humidifier or Dehumidifier
        if supports_humidifier:
            await self.write_single_attribute(attribute_value=attributes.Mode(modeHumidifier), endpoint_id=endpoint)
        else:
            await self.write_single_attribute(attribute_value=attributes.Mode(modeDehumidifier), endpoint_id=endpoint)

        self.step(next(step))  # Write MinSetpoint to UserSetpoint
        if supports_sensor:
            await self.write_single_attribute(attribute_value=attributes.UserSetpoint(dut_MinSetpoint), endpoint_id=endpoint)

        self.step(next(step))  # Read UserSetpoint, confirm it is MinSetpoint
        if supports_sensor:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_equal(dut_UserSetpoint, dut_MinSetpoint, "UserSetpoint attribute not equal to MinSetpoint attribute")

        self.step(next(step))  # Write MaxSetpoint to UserSetpoint
        if supports_sensor:
            await self.write_single_attribute(attribute_value=attributes.UserSetpoint(dut_MaxSetpoint), endpoint_id=endpoint)

        self.step(next(step))  # Read UserSetpoint, confirm it is MaxSetpoint
        if supports_sensor:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_equal(dut_UserSetpoint, dut_MaxSetpoint, "UserSetpoint attribute not equal to MaxSetpoint attribute")

        testUserValue = dut_MinSetpoint + dut_Step

        self.step(next(step))  # Write test value to UserSetpoint
        if supports_sensor:
            await self.write_single_attribute(attribute_value=attributes.UserSetpoint(testUserValue), endpoint_id=endpoint)

        self.step(next(step))  # Read UserSetpoint, confirm it is test value
        if supports_sensor:
            dut_UserSetpoint = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserSetpoint)
            asserts.assert_equal(dut_UserSetpoint, testUserValue, "UserSetpoint attribute not as expected")

        self.step(next(step))  # Read Continuous attribute
        if supports_continuous:
            dut_Continuous = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Continuous)

        testContinuousValue = not dut_Continuous

        self.step(next(step))  # Write opposite value to Continuous
        if supports_continuous:
            await self.write_single_attribute(attribute_value=attributes.Continuous(testContinuousValue), endpoint_id=endpoint)

        self.step(next(step))  # Read Continuous attribute and check value
        if supports_continuous:
            dut_Continuous = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Continuous)
            asserts.assert_equal(dut_Continuous, testContinuousValue, "Continuous attribute not as expected")

        self.step(next(step))  # Write Continuous to False
        if supports_continuous:
            await self.write_single_attribute(attribute_value=attributes.Continuous(False), endpoint_id=endpoint)

        self.step(next(step))  # Read Sleep attribute
        if attributes.Sleep.attribute_id in supported_attributes:
            dut_Sleep = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Sleep)

        testSleepValue = not dut_Sleep

        self.step(next(step))  # Write opposite value to Sleep
        if attributes.Sleep.attribute_id in supported_attributes:
            await self.write_single_attribute(attribute_value=attributes.Sleep(testSleepValue), endpoint_id=endpoint)

        self.step(next(step))  # Read Sleep attribute and check value
        if attributes.Sleep.attribute_id in supported_attributes:
            dut_Sleep = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Sleep)
            asserts.assert_equal(dut_Sleep, testSleepValue, "Sleep attribute not as expected")

        self.step(next(step))  # Write Sleep to False
        if attributes.Sleep.attribute_id in supported_attributes:
            await self.write_single_attribute(attribute_value=attributes.Sleep(False), endpoint_id=endpoint)

        self.step(next(step))  # Read Optimal attribute
        if supports_optimal:
            dut_Optimal = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Optimal)

        testOptimalValue = not dut_Optimal

        self.step(next(step))  # Write opposite value to Optimal
        if supports_optimal:
            await self.write_single_attribute(attribute_value=attributes.Optimal(testOptimalValue), endpoint_id=endpoint)

        self.step(next(step))  # Read Optimal attribute and check value
        if supports_optimal:
            dut_Optimal = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.Optimal)
            asserts.assert_equal(dut_Optimal, testOptimalValue, "Optimal attribute not as expected")

        self.step(next(step))  # Write SleeOptimalp to False
        if supports_optimal:
            await self.write_single_attribute(attribute_value=attributes.Optimal(False), endpoint_id=endpoint)

        self.step(next(step))  # Write bad step value to UserSetpoint
        if supports_sensor and dut_Step != 1:
            retval = await self.write_single_attribute(attribute_value=attributes.UserSetpoint(dut_MinSetpoint + 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(retval, Status.CONSTRAINT_ERROR,
                                 "Write of bad step to UserSetpoint DID NOT cause a CONSTRAINT_ERROR as expected")

        self.step(next(step))  # Write low error value to UserSetpoint
        if supports_sensor:
            retval = await self.write_single_attribute(attribute_value=attributes.UserSetpoint(dut_MinSetpoint - 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(retval, Status.CONSTRAINT_ERROR,
                                 "Write too low value to UserSetpoint DID NOT cause a CONSTRAINT_ERROR as expected")

        self.step(next(step))  # Write high error value to UserSetpoint
        if supports_sensor:
            retval = await self.write_single_attribute(attribute_value=attributes.UserSetpoint(dut_MaxSetpoint + 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(retval, Status.CONSTRAINT_ERROR,
                                 "Write too high value to UserSetpoint DID NOT cause a CONSTRAINT_ERROR as expected")

        self.step(next(step))  # Write Mode to Humidifier
        if supports_humidifier:
            await self.write_single_attribute(attribute_value=attributes.Mode(modeHumidifier), endpoint_id=endpoint)

        self.step(next(step))  # Write Warm to MistType
        if supports_humidifier and supports_warm:
            await self.write_single_attribute(attribute_value=attributes.MistType(mistBitmap.kWarmMist), endpoint_id=endpoint)

        self.step(next(step))  # Check that MistType attribute is Warm
        if supports_humidifier and supports_warm:
            dut_MistType = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MistType)
            asserts.assert_true(bool(dut_MistType & mistBitmap.kWarmMist), "MistType not Warm")

        self.step(next(step))  # Write Cold to MistType
        if supports_humidifier and supports_cold:
            await self.write_single_attribute(attribute_value=attributes.MistType(mistBitmap.kColdMist), endpoint_id=endpoint)

        self.step(next(step))  # Check that MistType attribute is Cold
        if supports_humidifier and supports_cold:
            dut_MistType = await self.read_hstat_attribute_expect_success(endpoint=endpoint, attribute=attributes.MistType)
            asserts.assert_true(bool(dut_MistType & mistBitmap.kColdMist), "MistType not Cold")

        self.step(next(step))  # Write Warm to MistType when it is not supported
        if supports_humidifier and not supports_warm:
            retval = await self.write_single_attribute(attribute_value=attributes.MistType(mistBitmap.kWarmMist), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(retval, Status.CONSTRAINT_ERROR, "Should not have been able to set the mist type to Warm")

        self.step(next(step))  # Write Cold to MistType when it is not supported
        if supports_humidifier and not supports_cold:
            retval = await self.write_single_attribute(attribute_value=attributes.MistType(mistBitmap.kColdMist), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(retval, Status.CONSTRAINT_ERROR, "Should not have been able to set the mist type to Cold")


if __name__ == "__main__":
    default_matter_test_main()
