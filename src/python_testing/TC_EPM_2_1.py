#
#    Copyright (c) 2024 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${EVSE_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_EnergyReporting_Utils import EnergyReportingBaseTestHelper

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

MIN_INT64_ALLOWED = -pow(2, 62)  # -(2^62)
MAX_INT64_ALLOWED = pow(2, 62)  # (2^62)


class TC_EPM_2_1(MatterBaseTest, EnergyReportingBaseTestHelper):

    def desc_TC_EPM_2_1(self) -> str:
        """Returns a description of this test"""
        return "5.1.2. [TC-EPM-2.1] Attributes with Server as DUT"

    def pics_TC_EPM_2_1(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EPM.S"]

    def steps_TC_EPM_2_1(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "TH reads PowerMode attribute",
                     "Verify that the DUT response contains an enum8 value"),
            TestStep("3", "TH reads NumberOfMeasurementTypes attribute",
                     "Verify that the DUT response contains an uint8 value."),
            TestStep("4", "TH reads Accuracy attribute",
                     "Verify that the DUT response contains a list of MeasurementAccuracyStruct entries ",
                     "Verify that the list has between 1 and NumberOfMeasurementTypes entries."),
            TestStep("5", "TH reads Ranges attribute",
                     "Verify that the DUT response contains a list of MeasurementRangeStruct entries ",
                     "Verify that the list has between 0 and NumberOfMeasurementTypes entries."),
            TestStep("6", "TH reads Voltage attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("7", "TH reads ActiveCurrent attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("8", "TH reads ReactiveCurrent attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("9", "TH reads ApparentCurrent attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of 0 to 2^62."),
            TestStep("10", "TH reads ActivePower attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("11", "TH reads ReactivePower attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("12", "TH reads ApparentPower attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("13", "TH reads RMSVoltage attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("14", "TH reads RMSCurrent attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("15", "TH reads RMSPower attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("16", "TH reads Frequency attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of 0 to 1000000."),
            TestStep("17", "TH reads HarmonicCurrents attribute",
                     "Verify that the DUT response contains a list of HarmonicMeasurementStruct entries."),
            TestStep("18", "TH reads HarmonicPhases attribute",
                     "Verify that the DUT response contains a list of HarmonicMeasurementStruct entries."),
            TestStep("19", "TH reads PowerFactor attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -10000 to 10000."),
            TestStep("20", "TH reads NeutralCurrent attribute",
                     "Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
        ]

    @async_test_body
    async def test_TC_EPM_2_1(self):

        self.step("1")
        # Commission DUT - already done

        supported_attributes = await self.get_supported_epm_attributes()

        self.step("2")
        power_mode = await self.read_epm_attribute_expect_success("PowerMode")
        log.info("Rx'd PowerMode: %s", power_mode)
        asserts.assert_not_equal(power_mode, Clusters.ElectricalPowerMeasurement.Enums.PowerModeEnum.kUnknown,
                                 "PowerMode must not be Unknown")

        self.step("3")
        number_of_measurements = await self.read_epm_attribute_expect_success("NumberOfMeasurementTypes")
        log.info("Rx'd NumberOfMeasurementTypes: %s", number_of_measurements)
        asserts.assert_greater_equal(number_of_measurements, 1,
                                     "NumberOfMeasurementTypes must be >= 1")

        self.step("4")
        accuracy = await self.read_epm_attribute_expect_success("Accuracy")
        log.info("Rx'd Accuracy: %s", accuracy)
        log.info("Checking Accuracy meets spec requirements")
        found_active_power = False
        for measurement in accuracy:
            log.info("measurementType:%s measured:%s minMeasuredValue:%s maxMeasuredValue:%s",
                     measurement.measurementType, measurement.measured, measurement.minMeasuredValue, measurement.maxMeasuredValue)

            # Scan all measurement types to check we have the mandatory kActivePower
            if (measurement.measurementType == Clusters.ElectricalPowerMeasurement.Enums.MeasurementTypeEnum.kActivePower):
                found_active_power = True

            # Check that the ranges are in order from minimum to maximum and don't have gaps
            asserts.assert_equal(measurement.minMeasuredValue, measurement.accuracyRanges[0].rangeMin,
                                 "minMeasuredValue must be the same as 1st accuracyRange rangeMin")

            for index, range_entry in enumerate(measurement.accuracyRanges):
                log.info("   [%s] rangeMin:%s rangeMax:%s percentMax:%s percentMin:%s percentTypical:%s fixedMax:%s fixedMin:%s "
                         "fixedTypical:%s",
                         index, range_entry.rangeMin, range_entry.rangeMax, range_entry.percentMax, range_entry.percentMin,
                         range_entry.percentTypical, range_entry.fixedMax, range_entry.fixedMin, range_entry.fixedTypical)
                asserts.assert_greater(
                    range_entry.rangeMax, range_entry.rangeMin, "rangeMax should be > rangeMin")
                if index == 0:
                    minimum_range = range_entry.rangeMin
                    maximum_range = range_entry.rangeMax
                    prev_range_max = range_entry.rangeMax
                else:
                    minimum_range = min(minimum_range, range_entry.rangeMin)
                    maximum_range = max(maximum_range, range_entry.rangeMax)
                    asserts.assert_equal(range_entry.rangeMin, prev_range_max + 1,
                                         f"Index[{index}] rangeMin was not +1 more then previous index's rangeMax {prev_range_max}")
                    prev_range_max = range_entry.rangeMax

            # Check that the last range rangeMax has the same value as the measurement.maxMeasuredValue
            asserts.assert_equal(measurement.maxMeasuredValue, prev_range_max,
                                 "maxMeasuredValue must be the same as the last accuracyRange rangeMax")
            asserts.assert_equal(maximum_range, measurement.maxMeasuredValue,
                                 "The maxMeasuredValue must be the same as any of the maximum of all rangeMax's")
            asserts.assert_equal(minimum_range, measurement.minMeasuredValue,
                                 "The minMeasuredValue must be the same as any of the minimum of all rangeMin's")

        asserts.assert_is(found_active_power, True,
                          "There must be an ActivePower measurement accuracy")
        asserts.assert_equal(len(accuracy), number_of_measurements,
                             "The number of accuracy entries should match the NumberOfMeasurementTypes")

        self.step("5")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.Ranges.attribute_id in supported_attributes):
            ranges = await self.read_epm_attribute_expect_success("Ranges")
            log.info("Rx'd Ranges: %s", ranges)
            # Check list length between 0 and NumberOfMeasurementTypes
            asserts.assert_greater_equal(len(ranges), 0)
            asserts.assert_less_equal(len(ranges), number_of_measurements)

        self.step("6")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.Voltage.attribute_id in supported_attributes):
            voltage = await self.check_epm_attribute_in_range("Voltage", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd Voltage: %s", voltage)

        self.step("7")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ActiveCurrent.attribute_id in supported_attributes):
            active_current = await self.check_epm_attribute_in_range("ActiveCurrent", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd ActiveCurrent: %s", active_current)

        self.step("8")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ReactiveCurrent.attribute_id in supported_attributes):
            reactive_current = await self.check_epm_attribute_in_range("ReactiveCurrent", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd ReactiveCurrent: %s", reactive_current)

        self.step("9")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ApparentCurrent.attribute_id in supported_attributes):
            apparent_current = await self.check_epm_attribute_in_range("ApparentCurrent", 0, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd ApparentCurrent: %s", apparent_current)

        self.step("10")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ActivePower.attribute_id in supported_attributes):
            active_power = await self.check_epm_attribute_in_range("ActivePower", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd ActivePower: %s", active_power)

        self.step("11")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ReactivePower.attribute_id in supported_attributes):
            reactive_power = await self.check_epm_attribute_in_range("ReactivePower", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd ReactivePower: %s", reactive_power)

        self.step("12")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ApparentPower.attribute_id in supported_attributes):
            apparent_power = await self.check_epm_attribute_in_range("ApparentPower", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd ApparentPower: %s", apparent_power)

        self.step("13")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.RMSVoltage.attribute_id in supported_attributes):
            rms_voltage = await self.check_epm_attribute_in_range("RMSVoltage", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd RMSVoltage: %s", rms_voltage)

        self.step("14")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.RMSCurrent.attribute_id in supported_attributes):
            rms_current = await self.check_epm_attribute_in_range("RMSCurrent", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd RMSCurrent: %s", rms_current)

        self.step("15")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.RMSPower.attribute_id in supported_attributes):
            rms_power = await self.check_epm_attribute_in_range("RMSPower", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd RMSPower: %s", rms_power)

        self.step("16")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.Frequency.attribute_id in supported_attributes):
            frequency = await self.check_epm_attribute_in_range("Frequency", 0, 1000000, allow_null=True)
            log.info("Rx'd Frequency: %s", frequency)

        self.step("17")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.HarmonicCurrents.attribute_id in supported_attributes):
            harmonic_currents = await self.read_epm_attribute_expect_success("HarmonicCurrents")
            log.info("Rx'd HarmonicCurrents: %s", harmonic_currents)
            asserts.assert_is(type(harmonic_currents), list)
            for index, entry in enumerate(harmonic_currents):
                log.info("   [%s] order:%s measurement:%s", index, entry.order, entry.measurement)
                asserts.assert_greater_equal(entry.order, 1)
                self.check_value_in_range(
                    "Measurement", entry.measurement, MIN_INT64_ALLOWED, MAX_INT64_ALLOWED)

        self.step("18")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.HarmonicPhases.attribute_id in supported_attributes):
            harmonic_phases = await self.read_epm_attribute_expect_success("HarmonicPhases")
            log.info("Rx'd HarmonicPhases: %s", harmonic_phases)
            asserts.assert_is(type(harmonic_phases), list)
            for index, entry in enumerate(harmonic_phases):
                log.info("   [%s] order:%s measurement:%s", index, entry.order, entry.measurement)
                asserts.assert_greater_equal(entry.order, 1)
                self.check_value_in_range(
                    "Measurement", entry.measurement, MIN_INT64_ALLOWED, MAX_INT64_ALLOWED)

        self.step("19")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.PowerFactor.attribute_id in supported_attributes):
            power_factor = await self.check_epm_attribute_in_range("PowerFactor", -10000, 10000, allow_null=True)
            log.info("Rx'd PowerFactor: %s", power_factor)

        self.step("20")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.NeutralCurrent.attribute_id in supported_attributes):
            neutral_current = await self.check_epm_attribute_in_range("NeutralCurrent", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            log.info("Rx'd NeutralCurrent: %s", neutral_current)


if __name__ == "__main__":
    default_matter_test_main()
