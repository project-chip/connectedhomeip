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
#     app: ${ENERGY_GATEWAY_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 00112233445566778899aabbccddeeff
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:00112233445566778899aabbccddeeff
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_SETRF_2_1."""

import logging
import time

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.clusters.Types import NullValue, Nullable
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
import test_plan_support

from TC_SETRF_TestBase import CommodityTariffTestBaseHelper

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class TC_SETRF_2_1(MatterBaseTest, CommodityTariffTestBaseHelper):
    """Implementation of test case TC_SETRF_2_1."""

    def desc_TC_SETRF_2_1(self) -> str:
        """Returns a description of this test"""

        return "Attributes with Server as DUT"

    def pics_TC_SETRF_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_SETRF_2_1(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "Read TariffInfo attribute"),
            TestStep("3", "Read TariffUnit attribute"),
            TestStep("4", "Read StartDate attribute"),
            TestStep("5", "Read DayEntries attribute"),
            TestStep("6", "Read DayPatterns attribute"),
            TestStep("7", "Read CalendarPeriods attribute"),
            TestStep("8", "Read IndividualDays attribute"),
            TestStep("9", "Read CurrentDay attribute"),
            TestStep("10", "Read NextDay attribute"),
            TestStep("11", "Read CurrentDayEntry attribute"),
            TestStep("12", "Read CurrentDayEntryDate attribute"),
            TestStep("13", "Read NextDayEntry attribute"),
            TestStep("14", "Read NextDayEntryDate attribute"),
            TestStep("15", "Read TariffComponents attribute"),
            TestStep("16", "Read TariffPeriods attribute"),
            TestStep("17", "Read CurrentTariffComponents attribute"),
            TestStep("18", "Read NextTariffComponents attribute"),
            TestStep("19", "Read DefaultRandomizationOffset attribute"),
            TestStep("20", "Read DefaultRandomizationType attribute"),
            TestStep("21", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster"),
            TestStep("22", "TH sends TestEventTrigger command to General Diagnostics Cluster for Full Tariff Set Test Event"),
            TestStep("23", "Read TariffInfo attribute"),
            TestStep("24", "Read TariffUnit attribute"),
            TestStep("25", "Read StartDate attribute"),
            TestStep("26", "Read DayEntries attribute"),
            TestStep("27", "Read DayPatterns attribute"),
            TestStep("28", "Read CalendarPeriods attribute"),
            TestStep("29", "Read IndividualDays attribute"),
            TestStep("30", "Read CurrentDay attribute"),
            TestStep("31", "Read NextDay attribute"),
            TestStep("32", "Read CurrentDayEntry attribute"),
            TestStep("33", "Read CurrentDayEntryDate attribute"),
            TestStep("34", "Read NextDayEntry attribute"),
            TestStep("35", "Read NextDayEntryDate attribute"),
            TestStep("36", "Read TariffComponents attribute"),
            TestStep("37", "Read TariffPeriods attribute"),
            TestStep("38", "Read CurrentTariffComponents attribute"),
            TestStep("39", "Read NextTariffComponents attribute"),
            TestStep("40", "Read DefaultRandomizationOffset attribute"),
            TestStep("41", "Read DefaultRandomizationType attribute"),
        ]
        return steps

    # StartDate = None
    # StartTime = None

    # async def checkAuxiliaryLoadSwitchSettingsStruct(self,
    #                                                  endpoint: int = None,
    #                                                  cluster: Clusters.CommodityTariff = None,
    #                                                  struct: Clusters.CommodityTariff.Structs.AuxiliaryLoadSwitchSettingsStruct = None):
    #     matter_asserts.assert_valid_uint8(struct.number, 'Number')
    #     matter_asserts.assert_valid_enum(
    #         struct.requiredState, "RequiredState attribute must return a AuxiliaryLoadSettingEnum", cluster.Enums.AuxiliaryLoadSettingEnum)

    # async def checkCalendarPeriodStruct(self,
    #                                     endpoint: int = None,
    #                                     cluster: Clusters.CommodityTariff = None,
    #                                     struct: Clusters.CommodityTariff.Structs.CalendarPeriodStruct = None):
    #     if struct.startDate is not NullValue:
    #         matter_asserts.assert_valid_uint32(struct.startDate, 'StartDate')
    #         asserts.assert_greater_equal(struct.startDate, self.StartDate)
    #     matter_asserts.assert_list(struct.dayPatternIDs, "DayPatternIDs attribute must return a list")
    #     matter_asserts.assert_list_element_type(
    #         struct.dayPatternIDs, int, "DayPatternIDs attribute must contain int elements")
    #     asserts.assert_greater_equal(len(struct.dayPatternIDs), 1, "DayPatternIDs must have at least 1 entries!")
    #     asserts.assert_less_equal(len(struct.dayPatternIDs), 7, "DayPatternIDs must have at most 7 entries!")

    # async def checkCurrencyStruct(self,
    #                               endpoint: int = None,
    #                               cluster: Clusters.CommodityTariff = None,
    #                               struct: Globals.Structs.CurrencyStruct = None):
    #     matter_asserts.assert_valid_uint16(struct.currency, 'Currency')
    #     asserts.assert_less_equal(struct.currency, 999)
    #     matter_asserts.assert_valid_uint8(struct.decimalPoints, 'DecimalPoints')

    # async def checkDayEntryStruct(self,
    #                               endpoint: int = None,
    #                               cluster: Clusters.CommodityTariff = None,
    #                               struct: Clusters.CommodityTariff.Structs.DayEntryStruct = None):
    #     matter_asserts.assert_valid_uint32(struct.dayEntryID, 'DayEntryID')
    #     matter_asserts.assert_valid_uint16(struct.startTime, 'StartTime')
    #     asserts.assert_less_equal(struct.startTime, 1499)
    #     if struct.duration is not None:
    #         matter_asserts.assert_valid_uint16(struct.duration, 'Duration')
    #         asserts.assert_less_equal(struct.duration, 1500 - struct.startTime)
    #     if self.check_pics("SETRF.S.F05"):
    #         matter_asserts.assert_valid_int16(struct.randomizationOffset, 'RandomizationOffset')
    #         matter_asserts.assert_valid_enum(
    #             struct.randomizationType, "RandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)
    #     else:
    #         asserts.assert_is_none(struct.randomizationOffset, "RandomizationOffset must be None")
    #         asserts.assert_is_none(struct.randomizationType, "RandomizationType must be None")

    # async def checkDayPatternStruct(self,
    #                                 endpoint: int = None,
    #                                 cluster: Clusters.CommodityTariff = None,
    #                                 struct: Clusters.CommodityTariff.Structs.DayPatternStruct = None):
    #     matter_asserts.assert_valid_uint32(struct.dayPatternID, 'DayPatternID')
    #     matter_asserts.is_valid_int_value(struct.daysOfWeek)
    #     # Check bitmap value less than or equal to (Sunday | Monday | Tuesday | Wednesday | Thursday | Friday | Saturday)
    #     asserts.assert_less_equal(struct.daysOfWeek, 127)
    #     matter_asserts.assert_list(struct.dayEntryIDs, "DayEntryIDs attribute must return a list")
    #     matter_asserts.assert_list_element_type(
    #         struct.dayEntryIDs, int, "DayEntryIDs attribute must contain int elements")
    #     asserts.assert_greater_equal(len(struct.dayEntryIDs), 1, "DayEntryIDs must have at least 1 entries!")
    #     asserts.assert_less_equal(len(struct.dayEntryIDs), 96, "DayEntryIDs must have at most 96 entries!")

    # async def checkDayStruct(self,
    #                          endpoint: int = None,
    #                          cluster: Clusters.CommodityTariff = None,
    #                          struct: Clusters.CommodityTariff.Structs.DayStruct = None):
    #     matter_asserts.assert_valid_uint32(struct.date, 'Date')
    #     matter_asserts.assert_valid_enum(
    #         struct.dayType, "DayType attribute must return a DayTypeEnum", cluster.Enums.DayTypeEnum)
    #     matter_asserts.assert_list(struct.dayEntryIDs, "DayEntryIDs attribute must return a list")
    #     matter_asserts.assert_list_element_type(
    #         struct.dayEntryIDs, int, "DayEntryIDs attribute must contain int elements")
    #     asserts.assert_greater_equal(len(struct.dayEntryIDs), 1, "DayEntryIDs must have at least 1 entries!")
    #     asserts.assert_less_equal(len(struct.dayEntryIDs), 96, "DayEntryIDs must have at most 96 entries!")

    # async def checkPeakPeriodStruct(self,
    #                                 endpoint: int = None,
    #                                 cluster: Clusters.CommodityTariff = None,
    #                                 struct: Clusters.CommodityTariff.Structs.PeakPeriodStruct = None):
    #     matter_asserts.assert_valid_enum(
    #         struct.severity, "Severity attribute must return a PeakPeriodSeverityEnum", cluster.Enums.PeakPeriodSeverityEnum)
    #     matter_asserts.assert_valid_uint16(struct.peakPeriod, 'PeakPeriod')
    #     asserts.assert_greater_equal(struct.peakPeriod, 1)

    # async def checkPowerThresholdStruct(self,
    #                                     endpoint: int = None,
    #                                     cluster: Clusters.CommodityTariff = None,
    #                                     struct: Globals.Structs.PowerThresholdStruct = None):
    #     if struct.powerThreshold is not None:
    #         matter_asserts.assert_valid_int64(struct.powerThreshold, 'PowerThreshold')
    #     if struct.apparentPowerThreshold is not None:
    #         matter_asserts.assert_valid_int64(struct.apparentPowerThreshold, 'ApparentPowerThreshold')
    #     if struct.powerThresholdSource is not NullValue:
    #         matter_asserts.assert_valid_enum(
    #             struct.powerThresholdSource, "PowerThresholdSource attribute must return a PowerThresholdSourceEnum", Globals.Enums.PowerThresholdSourceEnum)

    # async def checkTariffComponentStruct(self,
    #                                      endpoint: int = None,
    #                                      cluster: Clusters.CommodityTariff = None,
    #                                      struct: Clusters.CommodityTariff.Structs.TariffComponentStruct = None):
    #     matter_asserts.assert_valid_uint32(struct.tariffComponentID, 'TariffComponentID')
    #     if self.check_pics("SETRF.S.F00"):
    #         if struct.price is not NullValue:
    #             asserts.assert_true(isinstance(
    #                 struct.price, cluster.Structs.TariffPriceStruct), "struct.price must be of type TariffPriceStruct")
    #             await self.checkTariffPriceStruct(endpoint=endpoint, cluster=cluster, struct=struct.price)
    #     else:
    #         asserts.assert_is_none(struct.price, "Price must be None")
    #     if self.check_pics("SETRF.S.F01"):
    #         matter_asserts.assert_valid_bool(struct.friendlyCredit, 'FriendlyCredit')
    #     else:
    #         asserts.assert_is_none(struct.friendlyCredit, "FriendlyCredit must be None")
    #     if self.check_pics("SETRF.S.F02"):
    #         asserts.assert_true(isinstance(
    #             struct.auxiliaryLoad, cluster.Structs.AuxiliaryLoadSwitchSettingsStruct), "struct.auxiliaryLoad must be of type AuxiliaryLoadSwitchSettingsStruct")
    #         await self.checkAuxiliaryLoadSwitchSettingsStruct(endpoint=endpoint, cluster=cluster, struct=struct.auxiliaryLoad)
    #     else:
    #         asserts.assert_is_none(struct.auxiliaryLoad, "AuxiliaryLoad must be None")
    #     if self.check_pics("SETRF.S.F03"):
    #         asserts.assert_true(isinstance(
    #             struct.peakPeriod, cluster.Structs.PeakPeriodStruct), "struct.peakPeriod must be of type PeakPeriodStruct")
    #         await self.checkPeakPeriodStruct(endpoint=endpoint, cluster=cluster, struct=struct.peakPeriod)
    #     else:
    #         asserts.assert_is_none(struct.peakPeriod, "PeakPeriod must be None")
    #     if self.check_pics("SETRF.S.F04"):
    #         asserts.assert_true(isinstance(
    #             struct.powerThreshold, Globals.Structs.PowerThresholdStruct), "struct.powerThreshold must be of type PowerThresholdStruct")
    #         await self.checkPowerThresholdStruct(endpoint=endpoint, cluster=cluster, struct=struct.powerThreshold)
    #     else:
    #         asserts.assert_is_none(struct.powerThreshold, "PowerThreshold must be None")
    #     if struct.threshold is not NullValue:
    #         matter_asserts.assert_valid_uint32(struct.threshold, 'Threshold')
    #     if struct.label is not NullValue and struct.label is not None:
    #         matter_asserts.assert_is_string(struct.label, "Label must be a string")
    #         asserts.assert_less_equal(len(struct.label), 128, "Label must have length at most 128!")
    #     if struct.predicted is not None:
    #         matter_asserts.assert_valid_bool(struct.predicted, 'Predicted')

    # async def checkTariffInformationStruct(self,
    #                                        endpoint: int = None,
    #                                        cluster: Clusters.CommodityTariff = None,
    #                                        struct: Clusters.CommodityTariff.Structs.TariffInformationStruct = None):
    #     if struct.tariffLabel is not NullValue:
    #         matter_asserts.assert_is_string(struct.tariffLabel, "TariffLabel must be a string")
    #         asserts.assert_less_equal(len(struct.tariffLabel), 128, "TariffLabel must have length at most 128!")
    #     if struct.providerName is not NullValue:
    #         matter_asserts.assert_is_string(struct.providerName, "ProviderName must be a string")
    #         asserts.assert_less_equal(len(struct.providerName), 128, "ProviderName must have length at most 128!")
    #     if struct.currency is not NullValue:
    #         asserts.assert_true(isinstance(
    #             struct.currency, Globals.Structs.CurrencyStruct), "struct.currency must be of type CurrencyStruct")
    #         await self.checkCurrencyStruct(endpoint=endpoint, cluster=cluster, struct=struct.currency)
    #     if struct.blockMode is not NullValue:
    #         matter_asserts.assert_valid_enum(
    #             struct.blockMode, "BlockMode attribute must return a BlockModeEnum", cluster.Enums.BlockModeEnum)

    # async def checkTariffPeriodStruct(self,
    #                                   endpoint: int = None,
    #                                   cluster: Clusters.CommodityTariff = None,
    #                                   struct: Clusters.CommodityTariff.Structs.TariffPeriodStruct = None):
    #     if struct.label is not NullValue:
    #         matter_asserts.assert_is_string(struct.label, "Label must be a string")
    #         asserts.assert_less_equal(len(struct.label), 128, "Label must have length at most 128!")
    #     matter_asserts.assert_list(struct.dayEntryIDs, "DayEntryIDs attribute must return a list")
    #     matter_asserts.assert_list_element_type(
    #         struct.dayEntryIDs, int, "DayEntryIDs attribute must contain int elements")
    #     asserts.assert_greater_equal(len(struct.dayEntryIDs), 1, "DayEntryIDs must have at least 1 entries!")
    #     asserts.assert_less_equal(len(struct.dayEntryIDs), 20, "DayEntryIDs must have at most 20 entries!")
    #     matter_asserts.assert_list(struct.tariffComponentIDs, "TariffComponentIDs attribute must return a list")
    #     matter_asserts.assert_list_element_type(
    #         struct.tariffComponentIDs, int, "TariffComponentIDs attribute must contain int elements")
    #     asserts.assert_greater_equal(len(struct.tariffComponentIDs), 1, "TariffComponentIDs must have at least 1 entries!")
    #     asserts.assert_less_equal(len(struct.tariffComponentIDs), 20, "TariffComponentIDs must have at most 20 entries!")

    # async def checkTariffPriceStruct(self,
    #                                  endpoint: int = None,
    #                                  cluster: Clusters.CommodityTariff = None,
    #                                  struct: Clusters.CommodityTariff.Structs.TariffPriceStruct = None):
    #     matter_asserts.assert_valid_enum(
    #         struct.priceType, "PriceType attribute must return a TariffPriceTypeEnum", Globals.Enums.TariffPriceTypeEnum)
    #     if struct.price is not None:
    #         matter_asserts.assert_valid_int64(struct.price, 'Price')
    #     if struct.priceLevel is not None:
    #         matter_asserts.assert_valid_int16(struct.priceLevel, 'PriceLevel')

    @async_test_body
    async def test_TC_SETRF_2_1(self):

        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics("DGGEN.S") or not self.check_pics("DGGEN.S.A0008") or not self.check_pics("DGGEN.S.C00.Rsp"):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")

        self.step("2")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo
        )
        asserts.assert_true(val is NullValue, "TariffInfo attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "TariffInfo attribute must be a Nullable")

        self.step("3")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit
        )
        asserts.assert_true(val is NullValue, "TariffUnit attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "TariffUnit attribute must be a Nullable")

        self.step("4")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate
        )
        asserts.assert_true(val is NullValue, "StartDate attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "StartDate attribute must be a Nullable")

        self.step("5")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries
        )
        asserts.assert_true(val is NullValue, "DayEntries attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "DayEntries attribute must be a Nullable")

        self.step("6")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns
        )
        asserts.assert_true(val is NullValue, "DayPatterns attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "DayPatterns attribute must be a Nullable")

        self.step("7")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods
        )
        asserts.assert_true(val is NullValue, "CalendarPeriods attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "CalendarPeriods attribute must be a Nullable")

        self.step("8")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays
        )
        asserts.assert_true(val is NullValue, "IndividualDays attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "IndividualDays attribute must be a Nullable")

        self.step("9")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay
        )
        asserts.assert_true(val is NullValue, "CurrentDay attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "CurrentDay attribute must be a Nullable")

        self.step("10")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay
        )
        asserts.assert_true(val is NullValue, "NextDay attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "NextDay attribute must be a Nullable")

        self.step("11")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntry
        )
        asserts.assert_true(val is NullValue, "CurrentDayEntry attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "CurrentDayEntry attribute must be a Nullable")

        self.step("12")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate
        )
        asserts.assert_true(val is NullValue, "CurrentDayEntryDate attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "CurrentDayEntryDate attribute must be a Nullable")

        self.step("13")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntry
        )
        asserts.assert_true(val is NullValue, "NextDayEntry attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "NextDayEntry attribute must be a Nullable")

        self.step("14")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate
        )
        asserts.assert_true(val is NullValue, "NextDayEntryDate attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "NextDayEntryDate attribute must be a Nullable")

        self.step("15")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents
        )
        asserts.assert_true(val is NullValue, "TariffComponents attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "TariffComponents attribute must be a Nullable")

        self.step("16")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods
        )
        asserts.assert_true(val is NullValue, "TariffPeriods attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "TariffPeriods attribute must be a Nullable")

        self.step("17")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents
        )
        asserts.assert_true(val is NullValue, "CurrentTariffComponents attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "CurrentTariffComponents attribute must be a Nullable")

        self.step("18")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextTariffComponents
        )
        asserts.assert_true(val is NullValue, "NextTariffComponents attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "NextTariffComponents attribute must be a Nullable")

        self.step("19")
        if not self.check_pics("SETRF.S.A0011"):
            logger.info("PICS SETRF.S.A0011 is not True")
            self.mark_current_step_skipped()

        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationOffset
        )
        asserts.assert_true(val is NullValue, "DefaultRandomizationOffset attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "DefaultRandomizationOffset attribute must be a Nullable")

        self.step("20")
        if not self.check_pics("SETRF.S.A0012"):
            logger.info("PICS SETRF.S.A0012 is not True")
            self.mark_current_step_skipped()

        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationType
        )
        asserts.assert_true(val is NullValue, "DefaultRandomizationType attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "DefaultRandomizationType attribute must be a Nullable")

        self.step("21")
        await self.check_test_event_triggers_enabled()

        self.step("22")
        await self.send_test_event_triggers(eventTrigger=0x0700000000000000)
        time.sleep(3)

        self.step("23")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.TariffInformationStruct), "val must be of type TariffInformationStruct")
            await self.checkTariffInformationStruct(endpoint=endpoint, cluster=cluster, struct=val)
            if self.check_pics("SETRF.S.F00"):
                asserts.assert_true(val.currency is not None, "Currency must have real value or can be Null")

        self.step("24")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit)
        if val is not NullValue:
            matter_asserts.assert_valid_enum(
                val, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)
            asserts.assert_true(val >= 0 and val <= 8, "TariffUnit must be in range 0 - 8")

        self.step("25")
        self.StartDate = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate)
        if self.StartDate is not NullValue:
            matter_asserts.assert_valid_uint32(self.StartDate, 'StartDate')

        self.step("26")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayEntries attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayEntryStruct, "DayEntries attribute must contain DayEntryStruct elements")
            for item in val:
                await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("27")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayPatterns attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayPatternStruct, "DayPatterns attribute must contain DayPatternStruct elements")
            for item in val:
                await self.checkDayPatternStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("28")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods)
        if val is not NullValue:
            matter_asserts.assert_list(val, "CalendarPeriods attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.CalendarPeriodStruct, "CalendarPeriods attribute must contain CalendarPeriodStruct elements")
            for item in val:
                await self.checkCalendarPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)
            asserts.assert_greater_equal(len(val), 1, "CalendarPeriods must have at least 1 entries!")
            asserts.assert_less_equal(len(val), 4, "CalendarPeriods must have at most 4 entries!")
            for item in range(len(val) - 1):
                asserts.assert_less(val[item].startDate, val[item + 1].startDate,
                                    "CalendarPeriods must be sorted by Date in increasing order!")
            startDate_attr = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate)
            if startDate_attr is NullValue:
                asserts.assert_true(val[0].startDate is NullValue,
                                    "If StartDate is Null, the first CalendarPeriod item Start Date field must also be Null")

        self.step("29")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays)
        if val is not NullValue:
            matter_asserts.assert_list(val, "IndividualDays attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayStruct, "IndividualDays attribute must contain DayStruct elements")
            for item in val:
                await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=item)
            asserts.assert_less_equal(len(val), 50, "IndividualDays must have at most 50 entries!")
            for item in range(len(val) - 1):
                asserts.assert_less(val[item].date, val[item + 1].date,
                                    "IndividualDays must be sorted by Date in increasing order!")

        self.step("30")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayStruct), "val must be of type DayStruct")
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("31")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayStruct), "val must be of type DayStruct")
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("32")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntry)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayEntryStruct), "val must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("33")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'CurrentDayEntryDate')

        self.step("34")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntry)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayEntryStruct), "val must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("35")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'NextDayEntryDate')

        self.step("36")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "TariffComponents attribute must contain TariffComponentStruct elements")
            for item in val:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)
            asserts.assert_greater_equal(len(val), 1, "TariffComponents must have at least 1 entries!")

        self.step("37")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffPeriods attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffPeriodStruct, "TariffPeriods attribute must contain TariffPeriodStruct elements")
            for item in val:
                await self.checkTariffPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)
            asserts.assert_greater_equal(len(val), 1, "TariffPeriods must have at least 1 entries!")

        self.step("38")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "CurrentTariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "CurrentTariffComponents attribute must contain TariffComponentStruct elements")
            for item in val:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("39")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextTariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "NextTariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "NextTariffComponents attribute must contain TariffComponentStruct elements")
            for item in val:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("40")
        if not self.check_pics("SETRF.S.A0011"):
            logger.info("PICS SETRF.S.A0011 is not True")
            self.mark_current_step_skipped()

        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.DefaultRandomizationOffset):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationOffset)
            if val is not NullValue:
                matter_asserts.assert_valid_int16(val, 'DefaultRandomizationOffset')

        self.step("41")
        if not self.check_pics("SETRF.S.A0012"):
            logger.info("PICS SETRF.S.A0012 is not True")
            self.mark_current_step_skipped()

        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.DefaultRandomizationType):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationType)
            if val is not NullValue:
                matter_asserts.assert_valid_enum(
                    val, "DefaultRandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)


if __name__ == "__main__":
    default_matter_test_main()
