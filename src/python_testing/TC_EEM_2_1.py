#
#    Copyright (c) 2025 Project CHIP Authors
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
#     app: ${ENERGY_MANAGEMENT_APP}
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

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_EnergyReporting_Utils import EnergyReportingBaseTestHelper

logger = logging.getLogger(__name__)

cluster = Clusters.ElectricalEnergyMeasurement


class TC_EEM_2_1(MatterBaseTest, EnergyReportingBaseTestHelper):

    def desc_TC_EEM_2_1(self) -> str:
        """Returns a description of this test"""
        return "5.1.2. [TC-EEM-2.1] Attributes with Server as DUT"

    def pics_TC_EEM_2_1(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["EEM.S"]

    def steps_TC_EEM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "TH reads Accuracy attribute",
                     "Verify that the DUT response contains a MeasurementAccuracyStruct value."),
            TestStep("3", "TH reads CumulativeEnergyImported attribute",
                     "Verify that the DUT response contains either null or an EnergyMeasurementStruct value."),
            TestStep("4", "TH reads CumulativeEnergyExported attribute",
                     "Verify that the DUT response contains either null or an EnergyMeasurementStruct value."),
            TestStep("5", "TH reads PeriodicEnergyImported attribute",
                     "Verify that the DUT response contains either null or an EnergyMeasurementStruct value."),
            TestStep("6", "TH reads PeriodicEnergyExported attribute",
                     "Verify that the DUT response contains either null or an EnergyMeasurementStruct value."),
            TestStep("7", "TH reads CumulativeEnergyReset attribute",
                     "Verify that the DUT response contains either null or an CumulativeEnergyResetStruct value."),
        ]

        return steps

    @async_test_body
    async def test_TC_EEM_2_1(self):

        endpoint = self.get_endpoint()

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        accuracy = await self.read_eem_attribute_expect_success("Accuracy")
        logger.info(f"Rx'd Accuracy: {accuracy}")
        asserts.assert_not_equal(
            accuracy, NullValue, "Accuracy is not allowed to be null")
        asserts.assert_true(isinstance(
            accuracy, Globals.Structs.MeasurementAccuracyStruct), "accuracy must be of type MeasurementAccuracyStruct")
        asserts.assert_equal(accuracy.measurementType, Clusters.ElectricalEnergyMeasurement.Enums.MeasurementTypeEnum.kElectricalEnergy,
                             "Accuracy measurementType must be ElectricalEnergy")
        await self.test_checkMeasurementAccuracyStruct(endpoint=endpoint, cluster=cluster, struct=accuracy)

        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.CumulativeEnergyImported):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CumulativeEnergyImported)

        self.step("3")
        if self.pics_guard(self.check_pics("EEM.S.A0001")):
            cumulativeEnergyImported = await self.read_eem_attribute_expect_success("CumulativeEnergyImported")
            logger.info(
                f"Rx'd CumulativeEnergyImported: {cumulativeEnergyImported}")
            if cumulativeEnergyImported is not NullValue:
                asserts.assert_true(isinstance(
                    cumulativeEnergyImported, cluster.Structs.EnergyMeasurementStruct), "cumulativeEnergyImported must be of type EnergyMeasurementStruct")
                await self.test_checkEnergyMeasurementStruct(endpoint=endpoint, cluster=cluster, struct=cumulativeEnergyImported)

        self.step("4")
        if self.pics_guard(self.check_pics("EEM.S.A0002")):
            cumulativeEnergyExported = await self.read_eem_attribute_expect_success("CumulativeEnergyExported")
            logger.info(
                f"Rx'd CumulativeEnergyExported: {cumulativeEnergyExported}")
            if cumulativeEnergyExported is not NullValue:
                asserts.assert_true(isinstance(
                    cumulativeEnergyExported, cluster.Structs.EnergyMeasurementStruct), "cumulativeEnergyExported must be of type EnergyMeasurementStruct")
                await self.test_checkEnergyMeasurementStruct(endpoint=endpoint, cluster=cluster, struct=cumulativeEnergyExported)

        self.step("5")
        if self.pics_guard(self.check_pics("EEM.S.A0003")):
            periodicEnergyImported = await self.read_eem_attribute_expect_success("PeriodicEnergyImported")
            logger.info(
                f"Rx'd PeriodicEnergyImported: {periodicEnergyImported}")
            if periodicEnergyImported is not NullValue:
                asserts.assert_true(isinstance(
                    periodicEnergyImported, cluster.Structs.EnergyMeasurementStruct), "periodicEnergyImported must be of type EnergyMeasurementStruct")
                await self.test_checkEnergyMeasurementStruct(endpoint=endpoint, cluster=cluster, struct=periodicEnergyImported)

        self.step("6")
        if self.pics_guard(self.check_pics("EEM.S.A0004")):
            periodicEnergyExported = await self.read_eem_attribute_expect_success("PeriodicEnergyExported")
            logger.info(
                f"Rx'd PeriodicEnergyExported: {periodicEnergyExported}")
            if periodicEnergyExported is not NullValue:
                asserts.assert_true(isinstance(
                    periodicEnergyExported, cluster.Structs.EnergyMeasurementStruct), "periodicEnergyExported must be of type EnergyMeasurementStruct")
                await self.test_checkEnergyMeasurementStruct(endpoint=endpoint, cluster=cluster, struct=periodicEnergyExported)

        self.step("7")
        if self.pics_guard(self.check_pics("EEM.S.A0005")):
            cumulativeEnergyReset = await self.read_eem_attribute_expect_success("CumulativeEnergyReset")
            logger.info(f"Rx'd CumulativeEnergyReset: {cumulativeEnergyReset}")
            if cumulativeEnergyReset is not NullValue and val is not None:
                asserts.assert_true(isinstance(
                    cumulativeEnergyReset, cluster.Structs.CumulativeEnergyResetStruct), "cumulativeEnergyReset must be of type CumulativeEnergyResetStruct")
                await self.test_checkCumulativeEnergyResetStruct(endpoint=endpoint, cluster=cluster, struct=cumulativeEnergyReset)

    async def test_checkCumulativeEnergyResetStruct(self,
                                                    endpoint: int = None,
                                                    cluster: Clusters.ElectricalEnergyMeasurement = None,
                                                    struct: Clusters.ElectricalEnergyMeasurement.Structs.CumulativeEnergyResetStruct = None):
        if struct.importedResetTimestamp is not NullValue and struct.importedResetTimestamp is not None:
            matter_asserts.assert_valid_uint32(struct.importedResetTimestamp, 'ImportedResetTimestamp')
        if struct.exportedResetTimestamp is not NullValue and struct.exportedResetTimestamp is not None:
            matter_asserts.assert_valid_uint32(struct.exportedResetTimestamp, 'ExportedResetTimestamp')
        if struct.importedResetSystime is not NullValue and struct.importedResetSystime is not None:
            matter_asserts.assert_valid_uint64(struct.importedResetSystime, 'ImportedResetSystime')
        if struct.exportedResetSystime is not NullValue and struct.exportedResetSystime is not None:
            matter_asserts.assert_valid_uint64(struct.exportedResetSystime, 'ExportedResetSystime')

    async def test_checkEnergyMeasurementStruct(self,
                                                endpoint: int = None,
                                                cluster: Clusters.ElectricalEnergyMeasurement = None,
                                                struct: Clusters.ElectricalEnergyMeasurement.Structs.EnergyMeasurementStruct = None):
        matter_asserts.assert_valid_int64(struct.energy, 'Energy')
        asserts.assert_greater_equal(struct.energy, 0)
        asserts.assert_less_equal(struct.energy, 2e62)
        if struct.startTimestamp is not None:
            matter_asserts.assert_valid_uint32(struct.startTimestamp, 'StartTimestamp')
        if struct.endTimestamp is not None:
            matter_asserts.assert_valid_uint32(struct.endTimestamp, 'EndTimestamp')
            asserts.assert_greater_equal(struct.endTimestamp, struct.StartTimestamp + 1)
        if struct.startSystime is not None:
            matter_asserts.assert_valid_uint64(struct.startSystime, 'StartSystime')
        if struct.endSystime is not None:
            matter_asserts.assert_valid_uint64(struct.endSystime, 'EndSystime')
            asserts.assert_greater_equal(struct.endSystime, struct.StartSystime + 1)
        if struct.apparentEnergy is not None:
            matter_asserts.assert_valid_int64(struct.apparentEnergy, 'ApparentEnergy')
            asserts.assert_greater_equal(struct.apparentEnergy, 0)
            asserts.assert_less_equal(struct.apparentEnergy, 2e62)
        if struct.reactiveEnergy is not None:
            matter_asserts.assert_valid_int64(struct.reactiveEnergy, 'ReactiveEnergy')
            asserts.assert_greater_equal(struct.reactiveEnergy, 0)
            asserts.assert_less_equal(struct.reactiveEnergy, 2e62)

    async def test_checkMeasurementAccuracyRangeStruct(self,
                                                       endpoint: int = None,
                                                       cluster: Clusters.ElectricalEnergyMeasurement = None,
                                                       struct: Globals.Structs.MeasurementAccuracyRangeStruct = None):
        matter_asserts.assert_valid_int64(struct.rangeMin, 'RangeMin')
        asserts.assert_greater_equal(struct.rangeMin, -2e62)
        asserts.assert_less_equal(struct.rangeMin, 2e62)
        matter_asserts.assert_valid_int64(struct.rangeMax, 'RangeMax')
        asserts.assert_greater_equal(struct.rangeMax, -2e62)
        asserts.assert_less_equal(struct.rangeMax, 2e62)
        if struct.percentMax is not None:
            matter_asserts.assert_valid_uint16(struct.percentMax, 'PercentMax')
        if struct.percentMin is not None:
            matter_asserts.assert_valid_uint16(struct.percentMin, 'PercentMin')
            asserts.assert_less_equal(struct.percentMin, struct.PercentTypical)
        if struct.percentTypical is not None:
            matter_asserts.assert_valid_uint16(struct.percentTypical, 'PercentTypical')
            asserts.assert_greater_equal(struct.percentTypical, struct.PercentMin)
            asserts.assert_less_equal(struct.percentTypical, struct.PercentMax)
        if struct.fixedMax is not None:
            matter_asserts.assert_valid_uint64(struct.fixedMax, 'FixedMax')
            asserts.assert_less_equal(struct.fixedMax, 2e62 - 1)
        if struct.fixedMin is not None:
            matter_asserts.assert_valid_uint64(struct.fixedMin, 'FixedMin')
            asserts.assert_less_equal(struct.fixedMin, struct.FixedMax)
        if struct.fixedTypical is not None:
            matter_asserts.assert_valid_uint64(struct.fixedTypical, 'FixedTypical')
            asserts.assert_greater_equal(struct.fixedTypical, struct.FixedMin)
            asserts.assert_less_equal(struct.fixedTypical, struct.FixedMax)

    async def test_checkMeasurementAccuracyStruct(self,
                                                  endpoint: int = None,
                                                  cluster: Clusters.ElectricalEnergyMeasurement = None,
                                                  struct: Globals.Structs.MeasurementAccuracyStruct = None):
        matter_asserts.assert_valid_enum(
            struct.measurementType, "MeasurementType attribute must return a MeasurementTypeEnum", Globals.Enums.MeasurementTypeEnum)
        matter_asserts.assert_valid_bool(struct.measured, 'Measured')
        matter_asserts.assert_valid_int64(struct.minMeasuredValue, 'MinMeasuredValue')
        asserts.assert_greater_equal(struct.minMeasuredValue, -2e62)
        asserts.assert_less_equal(struct.minMeasuredValue, 2e62)
        matter_asserts.assert_valid_int64(struct.maxMeasuredValue, 'MaxMeasuredValue')
        asserts.assert_greater_equal(struct.maxMeasuredValue, -2e62)
        asserts.assert_less_equal(struct.maxMeasuredValue, 2e62)
        matter_asserts.assert_list(struct.accuracyRanges, "AccuracyRanges attribute must return a list")
        matter_asserts.assert_list_element_type(
            struct.accuracyRanges, "AccuracyRanges attribute must contain MeasurementAccuracyRangeStruct elements", Globals.Structs.MeasurementAccuracyRangeStruct)
        for item in struct.accuracyRanges:
            await self.test_checkMeasurementAccuracyRangeStruct(endpoint=endpoint, cluster=cluster, struct=item)
        asserts.assert_greater_equal(len(struct.accuracyRanges), 1, "AccuracyRanges must have at least 1 entries!")


if __name__ == "__main__":
    default_matter_test_main()
