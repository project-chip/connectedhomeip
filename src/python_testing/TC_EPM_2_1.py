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

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EnergyReporting_Utils import EnergyReportingBaseTestHelper

logger = logging.getLogger(__name__)

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
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads PowerMode attribute. Verify that the DUT response contains an enum8 value"),
            TestStep("3", "TH reads NumberOfMeasurementTypes attribute. Verify that the DUT response contains an uint8 value."),
            TestStep("4", "TH reads Accuracy attribute. Verify that the DUT response contains a list of MeasurementAccuracyStruct entries - Verify that the list has between 1 and NumberOfMeasurementTypes entries."),
            TestStep("5", "TH reads Ranges attribute. Verify that the DUT response contains a list of MeasurementRangeStruct entries - Verify that the list has between 0 and NumberOfMeasurementTypes entries."),
            TestStep("6", "TH reads Voltage attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("7", "TH reads ActiveCurrent attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("8", "TH reads ReactiveCurrent attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("9", "TH reads ApparentCurrent attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of 0 to 2^62."),
            TestStep("10", "TH reads ActivePower attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("11", "TH reads ReactivePower attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("12", "TH reads ApparentPower attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("13", "TH reads RMSVoltage attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("14", "TH reads RMSCurrent attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("15", "TH reads RMSPower attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
            TestStep("16", "TH reads Frequency attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of 0 to 1000000."),
            TestStep("17", "TH reads HarmonicCurrents attribute. Verify that the DUT response contains a list of HarmonicMeasurementStruct entries."),
            TestStep("18", "TH reads HarmonicPhases attribute. Verify that the DUT response contains a list of HarmonicMeasurementStruct entries."),
            TestStep("19", "TH reads PowerFactor attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -10000 to 10000."),
            TestStep("20", "TH reads NeutralCurrent attribute. Verify that the DUT response contains either null or an int64 value. Value has to be between a range of -2^62 to 2^62."),
        ]

        return steps

    @async_test_body
    async def test_TC_EPM_2_1(self):

        self.step("1")
        # Commission DUT - already done

        supported_attributes = await self.get_supported_epm_attributes()

        self.step("2")
        power_mode = await self.read_epm_attribute_expect_success("PowerMode")
        logger.info(f"Rx'd PowerMode: {power_mode}")
        asserts.assert_not_equal(power_mode, Clusters.ElectricalPowerMeasurement.Enums.PowerModeEnum.kUnknown,
                                 "PowerMode must not be Unknown")

        self.step("3")
        number_of_measurements = await self.read_epm_attribute_expect_success("NumberOfMeasurementTypes")
        logger.info(f"Rx'd NumberOfMeasurementTypes: {number_of_measurements}")
        asserts.assert_greater_equal(number_of_measurements, 1,
                                     "NumberOfMeasurementTypes must be >= 1")

        self.step("4")
        accuracy = await self.read_epm_attribute_expect_success("Accuracy")
        logger.info(f"Rx'd Accuracy: {accuracy}")
        logger.info("Checking Accuracy meets spec requirements")
        found_active_power = False
        for measurement in accuracy:
            logging.info(
                f"measurementType:{measurement.measurementType} measured:{measurement.measured} minMeasuredValue:{measurement.minMeasuredValue} maxMeasuredValue:{measurement.maxMeasuredValue}")

            # Scan all measurement types to check we have the mandatory kActivePower
            if (measurement.measurementType == Clusters.ElectricalPowerMeasurement.Enums.MeasurementTypeEnum.kActivePower):
                found_active_power = True

            # Check that the ranges are in order from minimum to maximum and don't have gaps
            asserts.assert_equal(measurement.minMeasuredValue, measurement.accuracyRanges[0].rangeMin,
                                 "minMeasuredValue must be the same as 1st accuracyRange rangeMin")

            for index, range_entry in enumerate(measurement.accuracyRanges):
                logging.info(f"   [{index}] rangeMin:{range_entry.rangeMin} rangeMax:{range_entry.rangeMax} percentMax:{range_entry.percentMax} percentMin:{range_entry.percentMin} percentTypical:{range_entry.percentTypical} fixedMax:{range_entry.fixedMax} fixedMin:{range_entry.fixedMin} fixedTypical:{range_entry.fixedTypical}")
                asserts.assert_greater(range_entry.rangeMax, range_entry.rangeMin, "rangeMax should be > rangeMin")
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

        asserts.assert_is(found_active_power, True, "There must be an ActivePower measurement accuracy")
        asserts.assert_equal(len(accuracy), number_of_measurements,
                             "The number of accuracy entries should match the NumberOfMeasurementTypes")

        self.step("5")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.Ranges.attribute_id in supported_attributes):
            ranges = await self.read_epm_attribute_expect_success("Ranges")
            logger.info(f"Rx'd Ranges: {ranges}")
            # Check list length between 0 and NumberOfMeasurementTypes
            asserts.assert_greater_equal(len(ranges), 0)
            asserts.assert_less_equal(len(ranges), number_of_measurements)

        self.step("6")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.Voltage.attribute_id in supported_attributes):
            voltage = await self.check_epm_attribute_in_range("Voltage", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd Voltage: {voltage}")

        self.step("7")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ActiveCurrent.attribute_id in supported_attributes):
            active_current = await self.check_epm_attribute_in_range("ActiveCurrent", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd ActiveCurrent: {active_current}")

        self.step("8")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ReactiveCurrent.attribute_id in supported_attributes):
            reactive_current = await self.check_epm_attribute_in_range("ReactiveCurrent", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd ReactiveCurrent: {reactive_current}")

        self.step("9")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ApparentCurrent.attribute_id in supported_attributes):
            apparent_current = await self.check_epm_attribute_in_range("ApparentCurrent", 0, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd ApparentCurrent: {apparent_current}")

        self.step("10")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ActivePower.attribute_id in supported_attributes):
            active_power = await self.check_epm_attribute_in_range("ActivePower", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd ActivePower: {active_power}")

        self.step("11")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ReactivePower.attribute_id in supported_attributes):
            reactive_power = await self.check_epm_attribute_in_range("ReactivePower", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd ReactivePower: {reactive_power}")

        self.step("12")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.ApparentPower.attribute_id in supported_attributes):
            apparent_power = await self.check_epm_attribute_in_range("ApparentPower", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd ApparentPower: {apparent_power}")

        self.step("13")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.RMSVoltage.attribute_id in supported_attributes):
            rms_voltage = await self.check_epm_attribute_in_range("RMSVoltage", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd RMSVoltage: {rms_voltage}")

        self.step("14")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.RMSCurrent.attribute_id in supported_attributes):
            rms_current = await self.check_epm_attribute_in_range("RMSCurrent", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd RMSCurrent: {rms_current}")

        self.step("15")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.RMSPower.attribute_id in supported_attributes):
            rms_power = await self.check_epm_attribute_in_range("RMSPower", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd RMSPower: {rms_power}")

        self.step("16")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.Frequency.attribute_id in supported_attributes):
            frequency = await self.check_epm_attribute_in_range("Frequency", 0, 1000000, allow_null=True)
            logger.info(f"Rx'd Frequency: {frequency}")

        self.step("17")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.HarmonicCurrents.attribute_id in supported_attributes):
            harmonic_currents = await self.read_epm_attribute_expect_success("HarmonicCurrents")
            logger.info(f"Rx'd HarmonicCurrents: {harmonic_currents}")
            asserts.assert_is(type(harmonic_currents), list)
            for index, entry in enumerate(harmonic_currents):
                logging.info(f"   [{index}] order:{entry.order} measurement:{entry.measurement}")
                asserts.assert_greater_equal(entry.order, 1)
                self.check_value_in_range("Measurement", entry.measurement, MIN_INT64_ALLOWED, MAX_INT64_ALLOWED)

        self.step("18")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.HarmonicPhases.attribute_id in supported_attributes):
            harmonic_phases = await self.read_epm_attribute_expect_success("HarmonicPhases")
            logger.info(f"Rx'd HarmonicPhases: {harmonic_phases}")
            asserts.assert_is(type(harmonic_phases), list)
            for index, entry in enumerate(harmonic_phases):
                logging.info(f"   [{index}] order:{entry.order} measurement:{entry.measurement}")
                asserts.assert_greater_equal(entry.order, 1)
                self.check_value_in_range("Measurement", entry.measurement, MIN_INT64_ALLOWED, MAX_INT64_ALLOWED)

        self.step("19")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.PowerFactor.attribute_id in supported_attributes):
            power_factor = await self.check_epm_attribute_in_range("PowerFactor", -10000, 10000, allow_null=True)
            logger.info(f"Rx'd PowerFactor: {power_factor}")

        self.step("20")
        if self.pics_guard(Clusters.ElectricalPowerMeasurement.Attributes.NeutralCurrent.attribute_id in supported_attributes):
            neutral_current = await self.check_epm_attribute_in_range("NeutralCurrent", MIN_INT64_ALLOWED, MAX_INT64_ALLOWED, allow_null=True)
            logger.info(f"Rx'd NeutralCurrent: {neutral_current}")


if __name__ == "__main__":
    default_matter_test_main()
