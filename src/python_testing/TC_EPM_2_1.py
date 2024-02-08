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
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_EnergyReporting_Utils import EnergyReportingBaseTestHelper

logger = logging.getLogger(__name__)


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

        self.step("2")
        power_mode = await self.read_epm_attribute_expect_success("PowerMode")
        logger.info(f"Rx'd PowerMode: {power_mode}")
        asserts.assert_not_equal(power_mode, Clusters.ElectricalPowerMeasurement.Enums.PowerModeEnum.kUnknown,
                                 "PowerMode must not be Unknown")

        self.step("3")
        number_of_measurements = await self.read_epm_attribute_expect_success("NumberOfMeasurementTypes")
        logger.info(f"Rx'd NumberOfMeasurementTypes: {number_of_measurements}")
        asserts.assert_greater(number_of_measurements, 1,
                               "NumberOfMeasurementTypes must be > 1")

        self.step("4")
        accuracy = await self.read_epm_attribute_expect_success("Accuracy")
        logger.info(f"Rx'd Accuracy: {accuracy}")
        # todo check list length

        self.step("5")
        ranges = await self.read_epm_attribute_expect_success("Ranges")
        logger.info(f"Rx'd Ranges: {ranges}")
        # todo check list length between 0 and NumberOfMeasurementTypes

        self.step("6")
        voltage = await self.check_epm_attribute_in_range("Voltage", -2 ^ 62, 2 ^ 62)
        logger.info(f"Rx'd Voltage: {voltage}")


if __name__ == "__main__":
    default_matter_test_main()
