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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --featureSet 0xa
#       --application evse
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

import copy
import logging
import random

import chip.clusters as Clusters
from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, run_if_endpoint_matches, has_cluster, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff

class SETRF_2_1(MatterBaseTest):

    def desc_SETRF_2_1(self) -> str:
        """Returns a description of this test"""
        return "Attributes with Server as DUT"

    def pics_SETRF_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["SETRF.S"]

    def steps_SETRF_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Read TariffInfo attribute"),
            TestStep("2", "Read TariffUnit attribute"),
            TestStep("3", "Read StartDate attribute"),
            TestStep("4", "Read DayEntries attribute"),
            TestStep("5", "Read DayPatterns attribute"),
            TestStep("6", "Read CalendarPeriods attribute"),
            TestStep("7", "Read IndividualDays attribute"),
            TestStep("8", "Read CurrentDay attribute"),
            TestStep("9", "Read NextDay attribute"),
            TestStep("10", "Read CurrentDayEntry attribute"),
            TestStep("11", "Read CurrentDayEntryDate attribute"),
            TestStep("12", "Read NextDayEntry attribute"),
            TestStep("13", "Read NextDayEntryDate attribute"),
            TestStep("14", "Read TariffComponents attribute"),
            TestStep("15", "Read TariffPeriods attribute"),
            TestStep("16", "Read CurrentTariffComponents attribute"),
            TestStep("17", "Read NextTariffComponents attribute"),
            TestStep("18", "Read DefaultRandomizationOffset attribute"),
            TestStep("19", "Read DefaultRandomizationType attribute"),
        ]
        return steps

    StartDate = None
    StartTime = None

    @run_if_endpoint_matches(has_cluster(Clusters.CommodityTariff))
    async def test_SETRF_2_1(self):
        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        self.step("1")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo)
        if val is not NullValue:
            asserts.assert_true(isinstance(val, cluster.Structs.TariffInformationStruct), f"val must be of type TariffInformationStruct")
            await self.test_checkTariffInformationStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("2")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit)
        if val is not NullValue:
            matter_asserts.assert_valid_enum(val, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)

        self.step("3")
        self.StartDate = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate)
        if self.StartDate is not NullValue:
            matter_asserts.assert_valid_uint32(self.StartDate, 'StartDate')

        self.step("4")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayEntries attribute must return a list")
            matter_asserts.assert_list_element_type(val,  "DayEntries attribute must contain DayEntryStruct elements", cluster.Structs.DayEntryStruct)
            for item in val:
                await self.test_checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("5")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayPatterns attribute must return a list")
            matter_asserts.assert_list_element_type(val,  "DayPatterns attribute must contain DayPatternStruct elements", cluster.Structs.DayPatternStruct)
            for item in val:
                await self.test_checkDayPatternStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("6")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods)
        if val is not NullValue:
            matter_asserts.assert_list(val, "CalendarPeriods attribute must return a list")
            matter_asserts.assert_list_element_type(val,  "CalendarPeriods attribute must contain CalendarPeriodStruct elements", cluster.Structs.CalendarPeriodStruct)
            for item in val:
                await self.test_checkCalendarPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)
            asserts.assert_greater_equal(len(val), 1, "CalendarPeriods must have at least 1 entries!")
            asserts.assert_less_equal(len(val), 4, "CalendarPeriods must have at most 4 entries!")

        self.step("7")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays)
        if val is not NullValue:
            matter_asserts.assert_list(val, "IndividualDays attribute must return a list")
            matter_asserts.assert_list_element_type(val,  "IndividualDays attribute must contain DayStruct elements", cluster.Structs.DayStruct)
            for item in val:
                await self.test_checkDayStruct(endpoint=endpoint, cluster=cluster, struct=item)
            asserts.assert_less_equal(len(val), 50, "IndividualDays must have at most 50 entries!")

        self.step("8")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)
        if val is not NullValue:
            asserts.assert_true(isinstance(val, cluster.Structs.DayStruct), f"val must be of type DayStruct")
            await self.test_checkDayStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("9")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        if val is not NullValue:
            asserts.assert_true(isinstance(val, cluster.Structs.DayStruct), f"val must be of type DayStruct")
            await self.test_checkDayStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("10")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntry)
        if val is not NullValue:
            asserts.assert_true(isinstance(val, cluster.Structs.DayEntryStruct), f"val must be of type DayEntryStruct")
            await self.test_checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("11")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'CurrentDayEntryDate')

        self.step("12")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntry)
        if val is not NullValue:
            asserts.assert_true(isinstance(val, cluster.Structs.DayEntryStruct), f"val must be of type DayEntryStruct")
            await self.test_checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("13")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'NextDayEntryDate')

        self.step("14")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(val,  "TariffComponents attribute must contain TariffComponentStruct elements", cluster.Structs.TariffComponentStruct)
            for item in val:
                await self.test_checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)
            asserts.assert_greater_equal(len(val), 1, "TariffComponents must have at least 1 entries!")

        self.step("15")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffPeriods attribute must return a list")
            matter_asserts.assert_list_element_type(val,  "TariffPeriods attribute must contain TariffPeriodStruct elements", cluster.Structs.TariffPeriodStruct)
            for item in val:
                await self.test_checkTariffPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)
            asserts.assert_greater_equal(len(val), 1, "TariffPeriods must have at least 1 entries!")

        self.step("16")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "CurrentTariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(val,  "CurrentTariffComponents attribute must contain TariffComponentStruct elements", cluster.Structs.TariffComponentStruct)
            for item in val:
                await self.test_checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("17")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextTariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "NextTariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(val,  "NextTariffComponents attribute must contain TariffComponentStruct elements", cluster.Structs.TariffComponentStruct)
            for item in val:
                await self.test_checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("18")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.DefaultRandomizationOffset):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationOffset)
            if val is not NullValue:
                matter_asserts.assert_valid_int16(val, 'DefaultRandomizationOffset')

        self.step("19")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.DefaultRandomizationType):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationType)
            if val is not NullValue:
                matter_asserts.assert_valid_enum(val, "DefaultRandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)

    async def test_checkAuxiliaryLoadSwitchSettingsStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Clusters.CommodityTariff.Structs.AuxiliaryLoadSwitchSettingsStruct = None):
        matter_asserts.assert_valid_uint8(struct.number, 'Number')
        matter_asserts.assert_valid_enum(struct.requiredState, "RequiredState attribute must return a AuxiliaryLoadSettingEnum", cluster.Enums.AuxiliaryLoadSettingEnum)

    async def test_checkCalendarPeriodStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Clusters.CommodityTariff.Structs.CalendarPeriodStruct = None):
        if struct.startDate is not NullValue:
            matter_asserts.assert_valid_uint32(struct.startDate, 'StartDate')
            asserts.assert_greater_equal(struct.startDate, self.StartDate)
        matter_asserts.assert_list(struct.dayPatternIDs, "DayPatternIDs attribute must return a list")
        matter_asserts.assert_list_element_type(struct.dayPatternIDs,  "DayPatternIDs attribute must contain int elements", int)
        asserts.assert_greater_equal(len(struct.dayPatternIDs), 1, "DayPatternIDs must have at least 1 entries!")
        asserts.assert_less_equal(len(struct.dayPatternIDs), 7, "DayPatternIDs must have at most 7 entries!")

    async def test_checkCurrencyStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Globals.Structs.CurrencyStruct = None):
        matter_asserts.assert_valid_uint16(struct.currency, 'Currency')
        asserts.assert_less_equal(struct.currency, 999)
        matter_asserts.assert_valid_uint8(struct.decimalPoints, 'DecimalPoints')

    async def test_checkDayEntryStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Clusters.CommodityTariff.Structs.DayEntryStruct = None):
        matter_asserts.assert_valid_uint32(struct.dayEntryID, 'DayEntryID')
        matter_asserts.assert_valid_uint16(struct.startTime, 'StartTime')
        asserts.assert_less_equal(struct.startTime, 1499)
        if struct.duration is not None:
            matter_asserts.assert_valid_uint16(struct.duration, 'Duration')
            asserts.assert_less_equal(struct.duration, 1500 - struct.StartTime)
        if struct.randomizationOffset is not None:
            matter_asserts.assert_valid_int16(struct.randomizationOffset, 'RandomizationOffset')
        if struct.randomizationType is not None:
            matter_asserts.assert_valid_enum(struct.randomizationType, "RandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)

    async def test_checkDayPatternStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Clusters.CommodityTariff.Structs.DayPatternStruct = None):
        matter_asserts.assert_valid_uint32(struct.dayPatternID, 'DayPatternID')
        matter_asserts.is_valid_int_value(struct.daysOfWeek)
        # Check bitmap value less than or equal to (Sunday | Monday | Tuesday | Wednesday | Thursday | Friday | Saturday)
        asserts.assert_less_equal(struct.daysOfWeek, 127)
        matter_asserts.assert_list(struct.dayEntryIDs, "DayEntryIDs attribute must return a list")
        matter_asserts.assert_list_element_type(struct.dayEntryIDs,  "DayEntryIDs attribute must contain int elements", int)
        asserts.assert_greater_equal(len(struct.dayEntryIDs), 1, "DayEntryIDs must have at least 1 entries!")
        asserts.assert_less_equal(len(struct.dayEntryIDs), 96, "DayEntryIDs must have at most 96 entries!")

    async def test_checkDayStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Clusters.CommodityTariff.Structs.DayStruct = None):
        matter_asserts.assert_valid_uint32(struct.date, 'Date')
        matter_asserts.assert_valid_enum(struct.dayType, "DayType attribute must return a DayTypeEnum", cluster.Enums.DayTypeEnum)
        matter_asserts.assert_list(struct.dayEntryIDs, "DayEntryIDs attribute must return a list")
        matter_asserts.assert_list_element_type(struct.dayEntryIDs,  "DayEntryIDs attribute must contain int elements", int)
        asserts.assert_greater_equal(len(struct.dayEntryIDs), 1, "DayEntryIDs must have at least 1 entries!")
        asserts.assert_less_equal(len(struct.dayEntryIDs), 96, "DayEntryIDs must have at most 96 entries!")

    async def test_checkPeakPeriodStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Clusters.CommodityTariff.Structs.PeakPeriodStruct = None):
        matter_asserts.assert_valid_enum(struct.severity, "Severity attribute must return a PeakPeriodSeverityEnum", cluster.Enums.PeakPeriodSeverityEnum)
        matter_asserts.assert_valid_uint16(struct.peakPeriod, 'PeakPeriod')
        asserts.assert_greater_equal(struct.peakPeriod, 1)

    async def test_checkPowerThresholdStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Globals.Structs.PowerThresholdStruct = None):
        if struct.powerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.powerThreshold, 'PowerThreshold')
        if struct.apparentPowerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.apparentPowerThreshold, 'ApparentPowerThreshold')
        if struct.powerThresholdSource is not NullValue:
            matter_asserts.assert_valid_enum(struct.powerThresholdSource, "PowerThresholdSource attribute must return a PowerThresholdSourceEnum", Globals.Enums.PowerThresholdSourceEnum)

    async def test_checkTariffComponentStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Clusters.CommodityTariff.Structs.TariffComponentStruct = None):
        matter_asserts.assert_valid_uint32(struct.tariffComponentID, 'TariffComponentID')
        if struct.price is not NullValue and struct.price is not None:
            asserts.assert_true(isinstance(struct.price, cluster.Structs.TariffPriceStruct), f"struct.price must be of type TariffPriceStruct")
            await self.test_checkTariffPriceStruct(endpoint=endpoint, cluster=cluster, struct=struct.price)
        if struct.friendlyCredit is not None:
            matter_asserts.assert_valid_bool(struct.friendlyCredit, 'FriendlyCredit')
        if struct.auxiliaryLoad is not None:
            asserts.assert_true(isinstance(struct.auxiliaryLoad, cluster.Structs.AuxiliaryLoadSwitchSettingsStruct), f"struct.auxiliaryLoad must be of type AuxiliaryLoadSwitchSettingsStruct")
            await self.test_checkAuxiliaryLoadSwitchSettingsStruct(endpoint=endpoint, cluster=cluster, struct=struct.auxiliaryLoad)
        if struct.peakPeriod is not None:
            asserts.assert_true(isinstance(struct.peakPeriod, cluster.Structs.PeakPeriodStruct), f"struct.peakPeriod must be of type PeakPeriodStruct")
            await self.test_checkPeakPeriodStruct(endpoint=endpoint, cluster=cluster, struct=struct.peakPeriod)
        if struct.powerThreshold is not None:
            asserts.assert_true(isinstance(struct.powerThreshold, Globals.Structs.PowerThresholdStruct), f"struct.powerThreshold must be of type PowerThresholdStruct")
            await self.test_checkPowerThresholdStruct(endpoint=endpoint, cluster=cluster, struct=struct.powerThreshold)
        if struct.threshold is not NullValue:
            matter_asserts.assert_valid_uint32(struct.threshold, 'Threshold')
        if struct.label is not NullValue and struct.label is not None:
            matter_asserts.assert_is_string(struct.label, "Label must be a string")
            asserts.assert_less_equal(len(struct.label), 128, "Label must have length at most 128!")
        if struct.predicted is not None:
            matter_asserts.assert_valid_bool(struct.predicted, 'Predicted')

    async def test_checkTariffInformationStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Clusters.CommodityTariff.Structs.TariffInformationStruct = None):
        if struct.tariffLabel is not NullValue:
            matter_asserts.assert_is_string(struct.tariffLabel, "TariffLabel must be a string")
            asserts.assert_less_equal(len(struct.tariffLabel), 128, "TariffLabel must have length at most 128!")
        if struct.providerName is not NullValue:
            matter_asserts.assert_is_string(struct.providerName, "ProviderName must be a string")
            asserts.assert_less_equal(len(struct.providerName), 128, "ProviderName must have length at most 128!")
        if struct.currency is not NullValue:
            asserts.assert_true(isinstance(struct.currency, Globals.Structs.CurrencyStruct), f"struct.currency must be of type CurrencyStruct")
            await self.test_checkCurrencyStruct(endpoint=endpoint, cluster=cluster, struct=struct.currency)
        if struct.blockMode is not NullValue:
            matter_asserts.assert_valid_enum(struct.blockMode, "BlockMode attribute must return a BlockModeEnum", cluster.Enums.BlockModeEnum)

    async def test_checkTariffPeriodStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Clusters.CommodityTariff.Structs.TariffPeriodStruct = None):
        if struct.label is not NullValue:
            matter_asserts.assert_is_string(struct.label, "Label must be a string")
            asserts.assert_less_equal(len(struct.label), 128, "Label must have length at most 128!")
        matter_asserts.assert_list(struct.dayEntryIDs, "DayEntryIDs attribute must return a list")
        matter_asserts.assert_list_element_type(struct.dayEntryIDs,  "DayEntryIDs attribute must contain int elements", int)
        asserts.assert_greater_equal(len(struct.dayEntryIDs), 1, "DayEntryIDs must have at least 1 entries!")
        asserts.assert_less_equal(len(struct.dayEntryIDs), 20, "DayEntryIDs must have at most 20 entries!")
        matter_asserts.assert_list(struct.tariffComponentIDs, "TariffComponentIDs attribute must return a list")
        matter_asserts.assert_list_element_type(struct.tariffComponentIDs,  "TariffComponentIDs attribute must contain int elements", int)
        asserts.assert_greater_equal(len(struct.tariffComponentIDs), 1, "TariffComponentIDs must have at least 1 entries!")
        asserts.assert_less_equal(len(struct.tariffComponentIDs), 20, "TariffComponentIDs must have at most 20 entries!")

    async def test_checkTariffPriceStruct(self, 
                                 endpoint: int = None, 
                                 cluster: Clusters.CommodityTariff = None, 
                                 struct: Clusters.CommodityTariff.Structs.TariffPriceStruct = None):
        matter_asserts.assert_valid_enum(struct.priceType, "PriceType attribute must return a TariffPriceTypeEnum", Globals.Enums.TariffPriceTypeEnum)
        if struct.price is not None:
            matter_asserts.assert_valid_int64(struct.price, 'Price')
        if struct.priceLevel is not None:
            matter_asserts.assert_valid_int16(struct.priceLevel, 'PriceLevel')

if __name__ == "__main__":
    default_matter_test_main()
