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


class TC_EEM_2_1(MatterBaseTest, EnergyReportingBaseTestHelper):

    def desc_TC_EEM_2_1(self) -> str:
        """Returns a description of this test"""
        return "5.1.2. [TC-EEM-2.1] Attributes with Server as DUT"

    def pics_TC_EEM_2_1(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EEM.S"]

    def steps_TC_EEM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads Accuracy attribute. Verify that the DUT response contains a MeasurementAccuracyStruct value."),
            TestStep("3", "TH reads CumulativeEnergyImported attribute. Verify that the DUT response contains either null or an EnergyMeasurementStruct value."),
            TestStep("4", "TH reads CumulativeEnergyExported attribute. Verify that the DUT response contains either null or an EnergyMeasurementStruct value."),
            TestStep("5", "TH reads PeriodicEnergyImported attribute. Verify that the DUT response contains either null or an EnergyMeasurementStruct value."),
            TestStep("6", "TH reads PeriodicEnergyExported attribute. Verify that the DUT response contains either null or an EnergyMeasurementStruct value."),
            TestStep("7", "TH reads CumulativeEnergyReset attribute. Verify that the DUT response contains either null or an CumulativeEnergyResetStruct value."),
        ]

        return steps

    @async_test_body
    async def test_TC_EEM_2_1(self):

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        accuracy = await self.read_eem_attribute_expect_success("Accuracy")
        logger.info(f"Rx'd Accuracy: {accuracy}")
        asserts.assert_not_equal(accuracy, NullValue, "Accuracy is not allowed to be null")
        asserts.assert_equal(accuracy.measurementType, Clusters.ElectricalEnergyMeasurement.Enums.MeasurementTypeEnum.kElectricalEnergy,
                             "Accuracy measurementType must be ElectricalEnergy")

        self.step("3")
        if self.pics_guard(self.check_pics("EEM.S.A0001")):
            cumulativeEnergyImported = await self.read_eem_attribute_expect_success("CumulativeEnergyImported")
            logger.info(f"Rx'd CumulativeEnergyImported: {cumulativeEnergyImported}")

        self.step("4")
        if self.pics_guard(self.check_pics("EEM.S.A0002")):
            cumulativeEnergyExported = await self.read_eem_attribute_expect_success("CumulativeEnergyExported")
            logger.info(f"Rx'd CumulativeEnergyExported: {cumulativeEnergyExported}")

        self.step("5")
        if self.pics_guard(self.check_pics("EEM.S.A0003")):
            periodicEnergyImported = await self.read_eem_attribute_expect_success("PeriodicEnergyImported")
            logger.info(f"Rx'd PeriodicEnergyImported: {periodicEnergyImported}")

        self.step("6")
        if self.pics_guard(self.check_pics("EEM.S.A0004")):
            periodicEnergyExported = await self.read_eem_attribute_expect_success("PeriodicEnergyExported")
            logger.info(f"Rx'd PeriodicEnergyExported: {periodicEnergyExported}")

        self.step("7")
        if self.pics_guard(self.check_pics("EEM.S.A0005")):
            cumulativeEnergyReset = await self.read_eem_attribute_expect_success("CumulativeEnergyReset")
            logger.info(f"Rx'd CumulativeEnergyReset: {cumulativeEnergyReset}")


if __name__ == "__main__":
    default_matter_test_main()
