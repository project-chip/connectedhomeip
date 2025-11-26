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

import datetime
import logging
import random
from typing import Any, List, Optional

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects, Globals
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.matter_testing import AttributeMatcher, AttributeValue, MatterBaseTest

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class CommodityTariffTestBaseHelper(MatterBaseTest):
    """This class contains supporting methods for the CommodityTariff test cases."""

    # Test event trigger IDs
    EventTriggerFakeData = 0x0700000000000000
    EventTriggerClear = 0x0700000000000001
    EventTriggerChangeDay = 0x0700000000000002
    EventTriggerChangeTime = 0x0700000000000003

    # Variables to store values between test steps
    # Attributes
    startDateAttributeValue: int = None
    defaultRandomizationType: cluster.Attributes.DefaultRandomizationType = None
    defaultRandomizationOffset: int = None
    tariffInfoValue: cluster.Structs.TariffInformationStruct = None
    tariffComponentsValue: List[cluster.Structs.TariffComponentStruct] = None
    currentTariffComponentsValue: List[cluster.Structs.TariffComponentStruct] = None
    nextTariffComponentsValue: List[cluster.Structs.TariffComponentStruct] = None
    tariffPeriodsValue: List[cluster.Structs.TariffPeriodStruct] = None
    dayEntriesValue: List[cluster.Structs.DayEntryStruct] = None
    dayPatternsValue: List[cluster.Structs.DayPatternStruct] = None
    calendarPeriodsValue: List[cluster.Structs.CalendarPeriodStruct] = None
    currentDayValue: cluster.Structs.DayStruct = None
    nextDayValue: cluster.Structs.DayStruct = None
    individualDaysValue: cluster.Structs.DayStruct = None
    currentDayEntryDateValue: int = None
    nextDayEntryDateValue: int = None

    # Fields
    blockModeValue: cluster.Enums.BlockModeEnum = None

    # Other
    dayEntryIDsEvents: List[int] = []

    async def check_list_elements_uniqueness(self, list_to_check: list[Any], object_name: str = "Elements") -> None:
        """
        Checks that all elements in the list are unique.
        """
        asserts.assert_equal(len(list_to_check), len(set(list_to_check)), f"{object_name} in the list must be unique")

    async def check_randomization_offset(self, randomization_type: cluster.Enums.DayEntryRandomizationTypeEnum, randomization_offset: int) -> None:
        """Checks RandomizationOffset field of DayEntryStruct or defaultRandomizationOffset attribute depending on the value of RandomizationType.

        Args:
            randomization_type (DayEntryRandomizationTypeEnum): Value of RandomizationType field of DayEntryStruct or defaultRandomizationType attribute.
            randomization_offset (int): Value of RandomizationOffset field of DayEntryStruct or defaultRandomizationOffset attribute.
        """

        if randomization_type == cluster.Enums.DayEntryRandomizationTypeEnum.kFixed:
            matter_asserts.assert_valid_int16(randomization_offset, 'RandomizationOffset must be int16 type value.')
        elif randomization_type == cluster.Enums.DayEntryRandomizationTypeEnum.kRandomNegative:
            matter_asserts.assert_valid_int16(randomization_offset, 'RandomizationOffset must be int16 type value.')
            asserts.assert_less_equal(randomization_offset, 0, "RandomizationOffset must be less than 0.")
        else:
            matter_asserts.assert_valid_int16(randomization_offset, 'RandomizationOffset must be int16 type value.')
            asserts.assert_greater_equal(randomization_offset, 0, "RandomizationOffset must be greater than 0.")

    async def checkAuxiliaryLoadSwitchSettingsStruct(self,
                                                     cluster: Clusters.CommodityTariff = None,
                                                     struct: Clusters.CommodityTariff.Structs.AuxiliaryLoadSwitchSettingsStruct = None) -> None:
        """
        Checks the correctness of the AuxiliaryLoadSwitchSettingsStruct data type entries.

        Args:
            cluster (Clusters.CommodityTariff): CommodityTariff cluster;
            struct (AuxiliaryLoadSwitchSettingsStruct): AuxiliaryLoadSwitchSettingsStruct data type entity.
        """

        # checks Number field, must be uint8 type value
        matter_asserts.assert_valid_uint8(struct.number, 'Number must be uint8 type value.')

        # checks RequiredState field, must be AuxiliaryLoadSettingEnum type value and in range 0 - 2
        matter_asserts.assert_valid_enum(
            struct.requiredState, "RequiredState attribute must return a AuxiliaryLoadSettingEnum", cluster.Enums.AuxiliaryLoadSettingEnum)
        asserts.assert_greater_equal(struct.requiredState, 0)
        asserts.assert_less_equal(struct.requiredState, 2)

    async def checkCalendarPeriodStruct(self,
                                        struct: Clusters.CommodityTariff.Structs.CalendarPeriodStruct = None,
                                        start_date_attribute: int = None) -> None:
        """
        Checks the correctness of the CalendarPeriodStruct data type entries.

        Args:
            struct (CalendarPeriodStruct): CalendarPeriodStruct data type entity;
            start_date_attribute (int): StartDate attribute value.
        """

        # checks StartDate field
        if struct.startDate is not NullValue:
            matter_asserts.assert_valid_uint32(struct.startDate, 'StartDate')
            asserts.assert_greater_equal(struct.startDate, start_date_attribute)

        # checks DayPatternIDs field
        matter_asserts.assert_list(
            struct.dayPatternIDs, "DayPatternIDs attribute must return a list with length in range 1 - 7", min_length=1, max_length=7)
        for dayPatternID in struct.dayPatternIDs:
            matter_asserts.assert_valid_uint32(dayPatternID, 'DayPatternID must has uint32 type.')

    async def checkCurrencyStruct(self,
                                  struct: Globals.Structs.CurrencyStruct = None) -> None:
        """
        Checks the correctness of the CurrencyStruct data type entries.

        Args:
            struct (CurrencyStruct): CurrencyStruct data type entity.
        """

        # checks Currency field must be uint16 type value and in range 0 - 999
        matter_asserts.assert_valid_uint16(struct.currency, 'Currency')
        asserts.assert_less_equal(struct.currency, 999)

        # checks DecimalPoints field must be uint8 type value
        matter_asserts.assert_valid_uint8(struct.decimalPoints, 'DecimalPoints')

    async def checkDayEntryStruct(self,
                                  endpoint: int,
                                  cluster: Clusters.CommodityTariff = None,
                                  struct: Clusters.CommodityTariff.Structs.DayEntryStruct = None) -> None:
        """
        Checks the correctness of the DayEntryStruct data type entries.

        Args:
            endpoint (int): Endpoint ID;
            cluster (Clusters.CommodityTariff): CommodityTariff cluster;
            struct (DayEntryStruct): DayEntryStruct data type entity.
        """

        # checks DayEntryID field must be uint32 type value
        matter_asserts.assert_valid_uint32(struct.dayEntryID, 'DayEntryID must has uint32 type.')

        # check StartTime field must be uint16 type value and in range 0 - 1499
        matter_asserts.assert_valid_uint16(struct.startTime, 'StartTime must has uint16 type.')
        asserts.assert_less_equal(struct.startTime, 1499)

        # checking Duration field only for days with DayType: Event
        # Duration must be uint16 type value and in range 1500 - StartTime
        if struct.dayEntryID in self.dayEntryIDsEvents:
            if struct.duration is not None:
                matter_asserts.assert_valid_uint16(struct.duration, 'Duration must has uint16 type.')
                asserts.assert_less_equal(struct.duration, 1500 - struct.startTime)

        # if SETRF.S.F05(RNDM) feature is enabled
        # checks RandomizationOffset and RandomizationType fields
        if await self.feature_guard(cluster=cluster, endpoint=endpoint, feature_int=cluster.Bitmaps.Feature.kRandomization):
            if struct.randomizationType is not None:
                # checks RandomizationType field must be DayEntryRandomizationTypeEnum type value and in range 0 - 4
                matter_asserts.assert_valid_enum(
                    struct.randomizationType, "RandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)
                asserts.assert_greater_equal(struct.randomizationType, 0)
                asserts.assert_less_equal(struct.randomizationType, 4)

            # checks RandomizationOffset field depending on the value of RandomizationType
            if struct.randomizationOffset is not None:
                await self.check_randomization_offset(struct.randomizationType, struct.randomizationOffset)
        else:  # if SETRF.S.F05(RNDM) feature is disabled
            asserts.assert_is_none(struct.randomizationOffset, "RandomizationOffset must be None")
            asserts.assert_is_none(struct.randomizationType, "RandomizationType must be None")

    async def checkDayPatternStruct(self,
                                    struct: Clusters.CommodityTariff.Structs.DayPatternStruct = None) -> None:
        """
        Checks the correctness of the DayPatternStruct data type entries.

        Args:
            struct (DayPatternStruct): DayPatternStruct data type entity.

        """

        # checks DayPatternID field must be uint32 type value
        matter_asserts.assert_valid_uint32(struct.dayPatternID, 'DayPatternID')

        # checks DaysOfWeek field must be uint8 type value
        matter_asserts.is_valid_int_value(struct.daysOfWeek)
        # Check bitmap value less than or equal to (Sunday | Monday | Tuesday | Wednesday | Thursday | Friday | Saturday)
        asserts.assert_less_equal(struct.daysOfWeek, 127)

        # checks DayEntryIDs field must be list with length in range 1 - 96
        matter_asserts.assert_list(struct.dayEntryIDs, "DayEntryIDs attribute must return a list", min_length=1, max_length=96)
        await self.check_list_elements_uniqueness(struct.dayEntryIDs, "DayEntryIDs")
        for dayEntryID in struct.dayEntryIDs:
            matter_asserts.assert_valid_uint32(dayEntryID, 'DayEntryID must has uint32 type.')

    async def checkDayStruct(self,
                             cluster: Clusters.CommodityTariff = None,
                             struct: Clusters.CommodityTariff.Structs.DayStruct = None) -> None:
        """
        Checks the correctness of the DayStruct data type entries.

        Args:
            cluster (Clusters.CommodityTariff): CommodityTariff cluster;
            struct (DayStruct): DayStruct data type entity.
        """

        # checks Date field must be uint32 type value
        matter_asserts.assert_valid_uint32(struct.date, 'Date field must has epoch-s type.')

        # checks DayType field must be DayTypeEnum type value and in range 0 - 3
        matter_asserts.assert_valid_enum(
            struct.dayType, "DayType field must has a DayTypeEnum", cluster.Enums.DayTypeEnum)
        asserts.assert_greater_equal(struct.dayType, 0, "DayType must be greater than 0.")
        asserts.assert_less_equal(struct.dayType, 3, "DayType must be less than 3.")

        # checks DayEntryIDs field must be list with length in range 1 - 96
        matter_asserts.assert_list(
            struct.dayEntryIDs, "DayEntryIDs attribute must return a list with length in range 1 - 96", min_length=1, max_length=96)
        for dayEntryID in struct.dayEntryIDs:
            matter_asserts.assert_valid_uint32(dayEntryID, 'DayEntryID must has uint32 type.')

        await self.check_list_elements_uniqueness(struct.dayEntryIDs, "DayEntryIDs")

        # Collect all DayEntryIDs for DayType: Event
        if struct.dayType == 3:
            self.dayEntryIDsEvents += struct.dayEntryIDs

    async def checkPeakPeriodStruct(self,
                                    cluster: Clusters.CommodityTariff = None,
                                    struct: Clusters.CommodityTariff.Structs.PeakPeriodStruct = None) -> None:
        """
        Checks the correctness of the PeakPeriodStruct data type entries.

        Args:
            cluster (Clusters.CommodityTariff): CommodityTariff cluster;
            struct (PeakPeriodStruct): PeakPeriodStruct data type entity.
        """

        # checks Severity field must be PeakPeriodSeverityEnum type value and in range 0 - 3
        matter_asserts.assert_valid_enum(
            struct.severity, "Severity attribute must return a PeakPeriodSeverityEnum", cluster.Enums.PeakPeriodSeverityEnum)
        asserts.assert_greater_equal(struct.severity, 0, "Severity must be greater than 0.")
        asserts.assert_less_equal(struct.severity, 3, "Severity must be less than 3.")

        # checks PeakPeriod field must be uint16 type value
        matter_asserts.assert_valid_uint16(struct.peakPeriod, 'PeakPeriod must has uint16 type.')
        asserts.assert_greater_equal(struct.peakPeriod, 1, "PeakPeriod must be greater or equal than 1.")

    async def checkPowerThresholdStruct(self,
                                        struct: Globals.Structs.PowerThresholdStruct = None) -> None:
        """
        Checks the correctness of the PowerThresholdStruct data type entries.

        Args:
            struct (PowerThresholdStruct): PowerThresholdStruct data type entity.
        """

        # checks PowerThreshold field must be int64 type
        if struct.powerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.powerThreshold, 'PowerThreshold must has power-mW type.')

        # checks ApparentPowerThreshold field must be int64 type
        if struct.apparentPowerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.apparentPowerThreshold, 'ApparentPowerThreshold must has power-mVA type.')

        # checks PowerThresholdSource field must be PowerThresholdSourceEnum type value and in range 0 - 2
        if struct.powerThresholdSource is not NullValue:
            matter_asserts.assert_valid_enum(
                struct.powerThresholdSource, "PowerThresholdSource attribute must return a PowerThresholdSourceEnum", Globals.Enums.PowerThresholdSourceEnum)
            asserts.assert_greater_equal(struct.powerThresholdSource, 0, "PowerThresholdSource must be greater than 0.")
            asserts.assert_less_equal(struct.powerThresholdSource, 2, "PowerThresholdSource must be less than 2.")

    async def checkTariffComponentStruct(self,
                                         endpoint: int = None,
                                         cluster: Clusters.CommodityTariff = None,
                                         struct: Clusters.CommodityTariff.Structs.TariffComponentStruct = None) -> None:
        """
        Checks the correctness of the TariffComponentStruct data type entries.

        Args:
            endpoint (int): Endpoint number;
            cluster (Clusters.CommodityTariff): CommodityTariff cluster;
            struct (TariffComponentStruct): TariffComponentStruct data type entity.
        """

        # checks TariffComponentID field must be uint32 type
        matter_asserts.assert_valid_uint32(struct.tariffComponentID, 'TariffComponentID must has uint32 type.')

        # checks that at least one field from price, friendlyCredit, auxiliaryLoad, peakPeriod, powerThreshold is set
        asserts.assert_true(any((struct.price, struct.friendlyCredit, struct.auxiliaryLoad, struct.peakPeriod, struct.powerThreshold)),
                            'At least one field from price, friendlyCredit, auxiliaryLoad, peakPeriod, powerThreshold must be set')

        # if SETRF.S.F00(PRICE) feature is enabled
        if await self.feature_guard(cluster=cluster, endpoint=endpoint, feature_int=cluster.Bitmaps.Feature.kPricing):
            # checks Price field must be of type TariffPriceStruct
            if struct.price is not None and struct.price is not NullValue:
                asserts.assert_true(isinstance(
                    struct.price, cluster.Structs.TariffPriceStruct), "Price field must be of type TariffPriceStruct")
                await self.checkTariffPriceStruct(struct=struct.price)
        else:  # if SETRF.S.F00(PRICE) feature is disabled
            asserts.assert_is_none(struct.price, "Price must be None")

        # if SETRF.S.F01(FCRED) feature is enabled
        if await self.feature_guard(cluster=cluster, endpoint=endpoint, feature_int=cluster.Bitmaps.Feature.kFriendlyCredit):
            if struct.friendlyCredit is not None:
                # checks FriendlyCredit field must be bool
                matter_asserts.assert_valid_bool(struct.friendlyCredit, 'FriendlyCredit')
        else:  # if SETRF.S.F01(FCRED) feature is disabled
            asserts.assert_is_none(struct.friendlyCredit, "FriendlyCredit must be None")

        # if SETRF.S.F02(AUXLD) feature is enabled
        if await self.feature_guard(cluster=cluster, endpoint=endpoint, feature_int=cluster.Bitmaps.Feature.kAuxiliaryLoad):
            if struct.auxiliaryLoad is not None:
                # checks AuxiliaryLoad field must be of type AuxiliaryLoadSwitchSettingsStruct
                asserts.assert_true(isinstance(
                    struct.auxiliaryLoad, cluster.Structs.AuxiliaryLoadSwitchSettingsStruct), "AuxiliaryLoad must be of type AuxiliaryLoadSwitchSettingsStruct")
                await self.checkAuxiliaryLoadSwitchSettingsStruct(cluster=cluster, struct=struct.auxiliaryLoad)
        else:  # if SETRF.S.F02(AUXLD) feature is disabled
            asserts.assert_is_none(struct.auxiliaryLoad, "AuxiliaryLoad must be None")

        # if SETRF.S.F03(PEAKP) feature is enabled
        if await self.feature_guard(cluster=cluster, endpoint=endpoint, feature_int=cluster.Bitmaps.Feature.kPeakPeriod):
            if struct.peakPeriod is not None:
                # checks PeakPeriod field must be of type PeakPeriodStruct
                asserts.assert_true(isinstance(
                    struct.peakPeriod, cluster.Structs.PeakPeriodStruct), "PeakPeriod must be of type PeakPeriodStruct")
                await self.checkPeakPeriodStruct(cluster=cluster, struct=struct.peakPeriod)
        else:  # if SETRF.S.F03(PEAKP) feature is disabled
            asserts.assert_is_none(struct.peakPeriod, "PeakPeriod must be None")

        # if SETRF.S.F04(PWRTHLD) feature is enabled
        if await self.feature_guard(cluster=cluster, endpoint=endpoint, feature_int=cluster.Bitmaps.Feature.kPowerThreshold):
            if struct.powerThreshold is not None:
                # checks PowerThreshold field must be of type PowerThresholdStruct
                asserts.assert_true(isinstance(
                    struct.powerThreshold, Globals.Structs.PowerThresholdStruct), "PowerThreshold must be of type PowerThresholdStruct")
                await self.checkPowerThresholdStruct(struct=struct.powerThreshold)
        else:  # if SETRF.S.F04(PWRTHLD) feature is disabled
            asserts.assert_is_none(struct.powerThreshold, "PowerThreshold must be None")

        # checks Threshold field must be int64 or null
        # All Threshold fields on TariffComponentStruct in a tariff whose BlockMode field is set to NoBlock SHALL be null.
        if self.blockModeValue == 0:
            asserts.assert_equal(struct.threshold, NullValue, "Threshold field must be Null if BlockMode is 0.")
        else:
            if struct.threshold is not NullValue:
                matter_asserts.assert_valid_int64(struct.threshold, 'Threshold field must has int64 type.')

        # checks Label field must be string
        if struct.label is not None and struct.label is not NullValue:
            matter_asserts.assert_is_string(struct.label, "Label must be a string")
            asserts.assert_less_equal(len(struct.label), 128, "Label must have length at most 128!")

        # checks Predicted field must be bool
        if struct.predicted is not None:
            matter_asserts.assert_valid_bool(struct.predicted, 'Predicted must has bool type.')

    async def checkTariffInformationStruct(self,
                                           endpoint: int = None,
                                           cluster: Clusters.CommodityTariff = None,
                                           struct: Clusters.CommodityTariff.Structs.TariffInformationStruct = None) -> None:
        """
        Checks the correctness of the TariffInformationStruct data type entries.

        Args:
            endpoint (int): Endpoint number;
            cluster (Clusters.CommodityTariff): CommodityTariff cluster;
            struct (TariffInformationStruct): TariffInformationStruct data type entity.
        """

        # checks TariffLabel field must be string
        if struct.tariffLabel is not NullValue:
            matter_asserts.assert_is_string(struct.tariffLabel, "TariffLabel must be a string")
            asserts.assert_less_equal(len(struct.tariffLabel), 128, "TariffLabel must have length at most 128!")

        # checks ProviderName field must be string
        if struct.providerName is not NullValue:
            matter_asserts.assert_is_string(struct.providerName, "ProviderName must be a string")
            asserts.assert_less_equal(len(struct.providerName), 128, "ProviderName must have length at most 128!")

        # if SETRF.S.F00(PRICE) feature is enabled
        if await self.feature_guard(cluster=cluster, endpoint=endpoint, feature_int=cluster.Bitmaps.Feature.kPricing):
            # checks Currency field must be CurrencyStruct
            asserts.assert_true(struct.currency is not None, "Currency must have real value or can be Null")
            if struct.currency is not NullValue:
                asserts.assert_true(isinstance(
                    struct.currency, Globals.Structs.CurrencyStruct), "Currency must be of type CurrencyStruct")
                await self.checkCurrencyStruct(struct=struct.currency)
        else:  # if SETRF.S.F00(PRICE) feature is disabled
            asserts.assert_is_none(struct.currency, "Currency must be None")

        # checks BlockMode field must be BlockModeEnum
        if struct.blockMode is not NullValue:
            matter_asserts.assert_valid_enum(
                struct.blockMode, "BlockMode attribute must return a BlockModeEnum", cluster.Enums.BlockModeEnum)
            self.blockModeValue = struct.blockMode

    async def checkTariffPeriodStruct(self,
                                      struct: Clusters.CommodityTariff.Structs.TariffPeriodStruct = None) -> None:
        """
        Checks the correctness of the TariffPeriodStruct data type entries.

        Args:
            struct (TariffPeriodStruct): TariffPeriodStruct data type entity.
        """

        # checks Label field must be string
        if struct.label is not NullValue:
            matter_asserts.assert_is_string(struct.label, "Label must be a string")
            asserts.assert_less_equal(len(struct.label), 128, "Label must have length at most 128!")

        # checks DayEntryIDs field must be list with length in range 1 - 20
        matter_asserts.assert_list(
            struct.dayEntryIDs, "DayEntryIDs attribute must return a list with length in range 1 - 20", min_length=1, max_length=20)
        for dayEntryID in struct.dayEntryIDs:
            matter_asserts.assert_valid_uint32(dayEntryID, 'DayEntryID must has uint32 type.')
        await self.check_list_elements_uniqueness(struct.dayEntryIDs, "DayEntryIDs")

        # checks TariffComponentIDs field must be list with length in range 1 - 20
        matter_asserts.assert_list(
            struct.tariffComponentIDs, "TariffComponentIDs attribute must return a list with length in range 1 - 20", min_length=1, max_length=20)
        for tariffComponentID in struct.tariffComponentIDs:
            matter_asserts.assert_valid_uint32(tariffComponentID, 'TariffComponentID must has uint32 type.')

    async def checkTariffPriceStruct(self,
                                     struct: Clusters.CommodityTariff.Structs.TariffPriceStruct = None) -> None:
        """
        Checks the correctness of the TariffPriceStruct data type entries.

        Args:
            struct (TariffPriceStruct): TariffPriceStruct data type entity.
        """

        # checks PriceType field must be TariffPriceTypeEnum and in range 0 - 4
        matter_asserts.assert_valid_enum(
            struct.priceType, "PriceType attribute must return a TariffPriceTypeEnum", Globals.Enums.TariffPriceTypeEnum)
        asserts.assert_greater_equal(struct.priceType, 0, "PriceType field of TariffPriceStruct must be greater than 0.")
        asserts.assert_less_equal(struct.priceType, 4, "PriceType field of TariffPriceStruct must be less than 4.")

        # checks Price field must be int64
        if struct.price is not None:
            matter_asserts.assert_valid_int64(struct.price, 'Price field of TariffPriceStruct must be money')

        # checks PriceLevel field must be int16
        if struct.priceLevel is not None:
            matter_asserts.assert_valid_int16(struct.priceLevel, 'PriceLevel field of TariffPriceStruct must be int16')

    async def send_test_event_trigger_for_attributes_value_set(self):
        """Simulate updating of values for all cluster attributes with valid data not equal to the pre-test state."""

        await self.send_test_event_triggers(eventTrigger=self.EventTriggerFakeData)

    async def send_test_event_trigger_clear(self):
        """Return the device to pre-test state."""

        await self.send_test_event_triggers(eventTrigger=self.EventTriggerClear)

    async def send_test_event_trigger_change_day(self):
        """This test event trigger ensure time shifting on 24h to simulate a day change."""

        await self.send_test_event_triggers(eventTrigger=self.EventTriggerChangeDay)

    async def send_test_event_trigger_change_time(self):
        """This test event trigger ensure time shifting on 4h to simulate a time change."""

        await self.send_test_event_triggers(eventTrigger=self.EventTriggerChangeTime)

    async def check_tariff_info_attribute(self, endpoint: int, attribute_value: Optional[cluster.Structs.TariffInformationStruct] = None) -> None:
        """Validate TariffInfo attribute.

        Args:
            endpoint (int): endpoint
            attribute_value (cluster.Structs.TariffInformationStruct, optional): TariffInfo attribute value. Defaults to None.
        """

        self.tariffInfoValue = attribute_value
        # if attribute value is not set, read it
        if not self.tariffInfoValue:
            self.tariffInfoValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo)

        logger.info(f"TariffInfo attribute value is: {self.tariffInfoValue}")

        # if attribute value is not null it must be of type TariffInformationStruct
        if self.tariffInfoValue is not NullValue:
            asserts.assert_true(isinstance(
                self.tariffInfoValue, cluster.Structs.TariffInformationStruct), "TariffInfo must be of type TariffInformationStruct")
            await self.checkTariffInformationStruct(endpoint=endpoint, cluster=cluster, struct=self.tariffInfoValue)

    async def check_tariff_unit_attribute(self, endpoint: int, attribute_value: Optional[Globals.Enums.TariffUnitEnum] = None) -> None:
        """Validate TariffUnit attribute.

        Args:
            endpoint (int): endpoint
            attribute_value (Optional[Globals.Enums.TariffUnitEnum], optional): TariffUnit attribute value. Defaults to None.
        """

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit)

        logger.info(f"TariffUnit attribute value is: {attribute_value}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(attribute_value, NullValue, "TariffUnit must be Null when TariffInfo is Null")

        # if attribute value is not null it must be of type TariffUnitEnum and in range 0 - 1
        if attribute_value is not NullValue:
            matter_asserts.assert_valid_enum(
                attribute_value, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)
            asserts.assert_true(attribute_value >= 0 and attribute_value <= 1, "TariffUnit must be in range 0 - 1")

    async def check_start_date_attribute(self, endpoint: int, attribute_value: Optional[int] = None) -> None:
        """Validate StartDate attribute.

        Args:
            endpoint (int): endpoint
            attribute_value (Optional[int], optional): StartDate attribute value. Defaults to None.
        """

        self.startDateAttributeValue = attribute_value
        if not self.startDateAttributeValue:
            self.startDateAttributeValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate)

        logger.info(f"StartDate attribute value is: {self.startDateAttributeValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.startDateAttributeValue, NullValue, "StartDate must be Null when TariffInfo is Null")

        # if attribute value is not null it must be of type uint32
        if self.startDateAttributeValue is not NullValue:
            matter_asserts.assert_valid_uint32(self.startDateAttributeValue, 'StartDate attribute must has uint32 type.')

    async def check_day_entries_attribute(self, endpoint: int, attribute_value: Optional[List[cluster.Structs.DayEntryStruct]] = None) -> None:
        """Validate DayEntries attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[List[cluster.Structs.DayEntryStruct]], optional): DayEntries attribute value. Defaults to None.
        """

        self.dayEntriesValue = attribute_value
        if not self.dayEntriesValue:
            self.dayEntriesValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries)

        logger.info(f"DayEntries attribute value is: {self.dayEntriesValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.dayEntriesValue, NullValue, "DayEntries must be Null when TariffInfo is Null")

        # if attribute value is not null it must be list of DayEntryStruct
        if self.dayEntriesValue is not NullValue:
            matter_asserts.assert_list(
                self.dayEntriesValue, "DayEntries attribute must return a list with length less or equal 672", max_length=672)
            matter_asserts.assert_list_element_type(
                self.dayEntriesValue, cluster.Structs.DayEntryStruct, "DayEntries attribute must contain DayEntryStruct elements")

            # is used to check DayEntryID uniqueness below
            dayEntryIDs_from_day_entries_attribute = []

            # check each DayEntryStruct
            for item in self.dayEntriesValue:
                await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=item)
                dayEntryIDs_from_day_entries_attribute.append(item.dayEntryID)

            # check DayEntryID uniqueness
            await self.check_list_elements_uniqueness(dayEntryIDs_from_day_entries_attribute, "DayEntryIDs")

    async def check_day_patterns_attribute(self, endpoint: int, attribute_value: Optional[List[cluster.Structs.DayPatternStruct]] = None) -> None:
        """Validate DayPatterns attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[List[cluster.Structs.DayPatternStruct]], optional): DayPatterns attribute value. Defaults to None.
        """

        self.dayPatternsValue = attribute_value
        if not self.dayPatternsValue:
            self.dayPatternsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)

        logger.info(f"DayPatterns attribute value is: {self.dayPatternsValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.dayPatternsValue, NullValue, "DayPatterns must be Null when TariffInfo is Null")

        # if attribute value is not null it must be list of DayPatternStruct with length in range 0 - 28
        if self.dayPatternsValue is not NullValue:
            matter_asserts.assert_list(
                self.dayPatternsValue, "DayPatterns attribute must return a list with length less or equal 28", min_length=0, max_length=28)
            if self.dayPatternsValue:  # if list is not empty
                matter_asserts.assert_list_element_type(
                    self.dayPatternsValue, cluster.Structs.DayPatternStruct, "DayPatterns attribute must contain DayPatternStruct elements")
                for item in self.dayPatternsValue:  # check each DayPatternStruct
                    await self.checkDayPatternStruct(struct=item)

    async def check_calendar_periods_attribute(self, endpoint: int, attribute_value: Optional[List[cluster.Structs.CalendarPeriodStruct]] = None) -> None:
        """Validate CalendarPeriods attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[List[cluster.Structs.CalendarPeriodStruct]], optional): CalendarPeriods attribute value. Defaults to None.
        """

        self.calendarPeriodsValue = attribute_value
        if not self.calendarPeriodsValue:
            self.calendarPeriodsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods)

        if self.startDateAttributeValue is None:
            self.startDateAttributeValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate)
            self.check_start_date_attribute(endpoint, self.startDateAttributeValue)

        logger.info(f"CalendarPeriods attribute value is: {self.calendarPeriodsValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.calendarPeriodsValue, NullValue, "CalendarPeriods must be Null when TariffInfo is Null")

        # if attribute value is not null it must be list of CalendarPeriodStruct with length in range 1 - 4
        if self.calendarPeriodsValue is not NullValue:
            matter_asserts.assert_list(
                self.calendarPeriodsValue, "CalendarPeriods attribute must return a list with length in range 1 - 4", min_length=1, max_length=4)
            matter_asserts.assert_list_element_type(
                self.calendarPeriodsValue, cluster.Structs.CalendarPeriodStruct, "CalendarPeriods attribute must contain CalendarPeriodStruct elements")

            # check each CalendarPeriodStruct
            for item in self.calendarPeriodsValue:
                await self.checkCalendarPeriodStruct(struct=item, start_date_attribute=self.startDateAttributeValue)

            # check CalendarPeriods order
            for item in range(len(self.calendarPeriodsValue) - 1):
                asserts.assert_less(self.calendarPeriodsValue[item].startDate, self.calendarPeriodsValue[item + 1].startDate,
                                    "CalendarPeriods must be sorted by Date in increasing order!")

            # If StartDate is Null, the first CalendarPeriod item Start Date field must also be Null
            if self.startDateAttributeValue is NullValue:
                asserts.assert_true(self.calendarPeriodsValue[0].startDate is NullValue,
                                    "If StartDate is Null, the first CalendarPeriod item Start Date field must also be Null")

                # If StartDate is Null only first CalendarPeriod item Start Date field must be Null, the other CalendarPeriod items Start Date field must not be Null
                if len(self.calendarPeriodsValue) > 1:
                    for item in range(1, len(self.calendarPeriodsValue)):
                        asserts.assert_true(self.calendarPeriodsValue[item].startDate is not NullValue,
                                            "If StartDate is Null only first CalendarPeriod item Start Date field must be Null, the other CalendarPeriod items Start Date field must not be Null")

    async def check_individual_days_attribute(self, endpoint: int, attribute_value: Optional[List[cluster.Structs.DayStruct]] = None) -> None:
        """Validate IndividualDays attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[List[cluster.Structs.DayStruct]], optional): IndividualDays attribute value. Defaults to None.
        """

        self.individualDaysValue = attribute_value
        if not self.individualDaysValue:
            self.individualDaysValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays)

        logger.info(f"IndividualDays attribute value is: {self.individualDaysValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.individualDaysValue, NullValue, "IndividualDays must be Null when TariffInfo is Null")

        # if attribute value is not null it must be list of DayStruct with length in range 0 - 50
        if self.individualDaysValue is not NullValue:
            matter_asserts.assert_list(
                self.individualDaysValue, "IndividualDays attribute must return a list with length less or equal 50", max_length=50)
            matter_asserts.assert_list_element_type(
                self.individualDaysValue, cluster.Structs.DayStruct, "IndividualDays attribute must contain DayStruct elements")

            # check each DayStruct
            for item in self.individualDaysValue:
                await self.checkDayStruct(cluster=cluster, struct=item)

            # check IndividualDays order
            for item in range(len(self.individualDaysValue) - 1):
                asserts.assert_less(self.individualDaysValue[item].date, self.individualDaysValue[item + 1].date,
                                    "IndividualDays must be sorted by Date in increasing order!")

    async def check_current_day_attribute(self, endpoint: int, attribute_value: Optional[cluster.Structs.DayStruct] = None) -> None:
        """Validate CurrentDay attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[cluster.Structs.DayStruct], optional): CurrentDay attribute value. Defaults to None.
        """

        self.currentDayValue = attribute_value
        if not self.currentDayValue:
            self.currentDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)

        logger.info(f"CurrentDay attribute value is: {self.currentDayValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.currentDayValue, NullValue, "CurrentDay must be Null when TariffInfo is Null")

        # if attribute value is not null it must be DayStruct
        if self.currentDayValue is not NullValue:
            asserts.assert_true(isinstance(
                self.currentDayValue, cluster.Structs.DayStruct), "CurrentDay must be of type DayStruct")
            await self.checkDayStruct(cluster=cluster, struct=self.currentDayValue)

    async def check_next_day_attribute(self, endpoint: int, attribute_value: Optional[cluster.Structs.DayStruct] = None) -> None:
        """Validate NextDay attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[cluster.Structs.DayStruct], optional): NextDay attribute value. Defaults to None.
        """

        self.nextDayValue = attribute_value
        if not self.nextDayValue:
            self.nextDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)

        logger.info(f"NextDay attribute value is: {self.nextDayValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.nextDayValue, NullValue, "NextDay must be Null when TariffInfo is Null")

        # if attribute value is not null it must be DayStruct
        if self.nextDayValue is not NullValue:
            asserts.assert_true(isinstance(
                self.nextDayValue, cluster.Structs.DayStruct), "NextDay must be of type DayStruct")
            await self.checkDayStruct(cluster=cluster, struct=self.nextDayValue)

    async def check_current_day_entry_attribute(self, endpoint: int, attribute_value: Optional[cluster.Structs.DayEntryStruct] = None) -> None:
        """Validate CurrentDayEntry attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[cluster.Structs.DayEntryStruct], optional): CurrentDayEntry attribute value. Defaults to None.
        """

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntry)

        logger.info(f"CurrentDayEntry attribute value is: {attribute_value}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(attribute_value, NullValue, "CurrentDayEntry must be Null when TariffInfo is Null")

        # if attribute value is not null it must be DayEntryStruct
        if attribute_value is not NullValue:
            asserts.assert_true(isinstance(
                attribute_value, cluster.Structs.DayEntryStruct), "CurrentDayEntry must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=attribute_value)

    async def check_current_day_entry_date_attribute(self, endpoint: int, attribute_value: Optional[int] = None) -> None:
        """Validate CurrentDayEntryDate attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[int], optional): CurrentDayEntryDate attribute value. Defaults to None.
        """
        self.currentDayEntryDateValue = attribute_value
        if not self.currentDayEntryDateValue:
            self.currentDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)

        logger.info(f"CurrentDayEntryDate attribute value is: {self.currentDayEntryDateValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.currentDayEntryDateValue, NullValue,
                                 "CurrentDayEntryDate must be Null when TariffInfo is Null")

        # if attribute value is not null it must be uint32
        if self.currentDayEntryDateValue is not NullValue:
            matter_asserts.assert_valid_uint32(self.currentDayEntryDateValue, 'CurrentDayEntryDate must be of type uint32')

    async def check_next_day_entry_attribute(self, endpoint: int, attribute_value: Optional[cluster.Structs.DayEntryStruct] = None) -> None:
        """Validate NextDayEntry attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[cluster.Structs.DayEntryStruct], optional): NextDayEntry attribute value. Defaults to None.
        """

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntry)

        logger.info(f"NextDayEntry attribute value is: {attribute_value}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(attribute_value, NullValue, "NextDayEntry must be Null when TariffInfo is Null")

        # if attribute value is not null it must be DayEntryStruct
        if attribute_value is not NullValue:
            asserts.assert_true(isinstance(
                attribute_value, cluster.Structs.DayEntryStruct), "NextDayEntry must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=attribute_value)

    async def check_next_day_entry_date_attribute(self, endpoint: int, attribute_value: Optional[int] = None) -> None:
        """Validate NextDayEntryDate attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[int], optional): NextDayEntryDate attribute value. Defaults to None.
        """

        self.nextDayEntryDateValue = attribute_value
        if not self.nextDayEntryDateValue:
            self.nextDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)

        logger.info(f"NextDayEntryDate attribute value is: {self.nextDayEntryDateValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.nextDayEntryDateValue, NullValue, "NextDayEntryDate must be Null when TariffInfo is Null")

        # if attribute value is not null it must be uint32 and greater than CurrentDayEntryDate
        if self.nextDayEntryDateValue is not NullValue:
            matter_asserts.assert_valid_uint32(self.nextDayEntryDateValue, 'NextDayEntryDate must be of type uint32')
            asserts.assert_greater(self.nextDayEntryDateValue, self.currentDayEntryDateValue,
                                   "NextDayEntryDate must be greater than CurrentDayEntryDate")

    async def check_tariff_components_attribute(self, endpoint: int, attribute_value: Optional[List[cluster.Structs.TariffComponentStruct]] = None) -> None:
        """Validate TariffComponents attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[List[cluster.Structs.TariffComponentStruct]], optional): TariffComponents attribute value. Defaults to None.
        """

        self.tariffComponentsValue = attribute_value
        if not self.tariffComponentsValue:
            self.tariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)

        logger.info(f"TariffComponents attribute value is: {self.tariffComponentsValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.tariffComponentsValue, NullValue, "TariffComponents must be empty when TariffInfo is Null")

        # if attribute value is not null it must be list of TariffComponentStruct
        if self.tariffComponentsValue is not NullValue:
            matter_asserts.assert_list(
                self.tariffComponentsValue, "TariffComponents attribute must return a list with length greater or equal 1", min_length=1, max_length=672)
            matter_asserts.assert_list_element_type(
                self.tariffComponentsValue, cluster.Structs.TariffComponentStruct, "TariffComponents attribute must contain TariffComponentStruct elements")

            # check each TariffComponentStruct
            for item in self.tariffComponentsValue:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

    async def check_tariff_periods_attribute(self, endpoint: int, attribute_value: Optional[List[cluster.Structs.TariffPeriodStruct]] = None) -> None:
        """Validate TariffPeriods attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[List[cluster.Structs.TariffPeriodStruct]], optional): TariffPeriods attribute value. Defaults to None.
        """
        self.tariffPeriodsValue = attribute_value
        if not self.tariffPeriodsValue:
            self.tariffPeriodsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)

        logger.info(f"TariffPeriods attribute value is: {self.tariffPeriodsValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.tariffPeriodsValue, NullValue, "TariffPeriods must be empty when TariffInfo is Null")

        # if attribute value is not null it must be list of TariffPeriodStruct
        if self.tariffPeriodsValue is not NullValue:
            matter_asserts.assert_list(
                self.tariffPeriodsValue, "TariffPeriods attribute must return a list with length greater or equal 1", min_length=1, max_length=672)
            matter_asserts.assert_list_element_type(
                self.tariffPeriodsValue, cluster.Structs.TariffPeriodStruct, "TariffPeriods attribute must contain TariffPeriodStruct elements")

            # check each TariffPeriodStruct
            for tariff_period in self.tariffPeriodsValue:

                await self.checkTariffPeriodStruct(struct=tariff_period)

                await self.validate_tariff_component_ID_uniqueness_for_features(await self.get_tariff_components_by_its_IDs(tariff_period.tariffComponentIDs))

    async def check_current_tariff_components_attribute(self, endpoint: int, attribute_value: Optional[List[cluster.Structs.TariffComponentStruct]] = None) -> None:
        """Validate CurrentTariffComponents attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[List[cluster.Structs.TariffComponentStruct]], optional): CurrentTariffComponents attribute value. Defaults to None.
        """

        self.currentTariffComponentsValue = attribute_value
        if not self.currentTariffComponentsValue:
            self.currentTariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents)

        logger.info(f"CurrentTariffComponents attribute value is: {self.currentTariffComponentsValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.currentTariffComponentsValue, NullValue,
                                 "CurrentTariffComponents must be Null when TariffInfo is Null")

        # if attribute value is not null it must be list of TariffComponentStruct with length in range 0 - 20
        if self.currentTariffComponentsValue is not NullValue:
            matter_asserts.assert_list(
                self.currentTariffComponentsValue, "CurrentTariffComponents attribute must return a list with length less or equal 20", max_length=20)
            matter_asserts.assert_list_element_type(
                self.currentTariffComponentsValue, cluster.Structs.TariffComponentStruct, "CurrentTariffComponents attribute must contain TariffComponentStruct elements")

            # check each TariffComponentStruct
            for item in self.currentTariffComponentsValue:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

    async def check_next_tariff_components_attribute(self, endpoint: int, attribute_value: Optional[List[cluster.Structs.TariffComponentStruct]] = None) -> None:
        """Validate NextTariffComponents attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[List[cluster.Structs.TariffComponentStruct]], optional): NextTariffComponents attribute value. Defaults to None.
        """

        self.nextTariffComponentsValue = attribute_value
        if not self.nextTariffComponentsValue:
            self.nextTariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextTariffComponents)

        logger.info(f"NextTariffComponents attribute value is: {self.nextTariffComponentsValue}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.nextTariffComponentsValue, NullValue,
                                 "NextTariffComponents must be Null when TariffInfo is Null")

        # if attribute value is not null it must be list of TariffComponentStruct with length in range 0 - 20
        if self.nextTariffComponentsValue is not NullValue:
            matter_asserts.assert_list(
                self.nextTariffComponentsValue, "NextTariffComponents attribute must return a list with length less or equal 20", max_length=20)
            matter_asserts.assert_list_element_type(
                self.nextTariffComponentsValue, cluster.Structs.TariffComponentStruct, "NextTariffComponents attribute must contain TariffComponentStruct elements")

            # check TariffComponentStruct
            for item in self.nextTariffComponentsValue:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

    async def check_default_randomization_offset_attribute(self, endpoint: int, attribute_value: Optional[int] = None) -> None:
        """Validate DefaultRandomizationOffset attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[int], optional): DefaultRandomizationOffset attribute value. Defaults to None.
        """

        self.defaultRandomizationOffset = attribute_value
        if self.defaultRandomizationOffset is None:
            if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DefaultRandomizationOffset):
                self.defaultRandomizationOffset = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationOffset)

        logger.info(f"DefaultRandomizationOffset attribute value is: {self.defaultRandomizationOffset}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.defaultRandomizationOffset, NullValue,
                                 "DefaultRandomizationOffset must be Null when TariffInfo is Null")

        # check if RNDM feature and DefaultRandomizationOffset attribute are enabled
        if (await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DefaultRandomizationOffset) and
                await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kRandomization)):
            # if feature is enabled, DefaultRandomizationOffset attribute must not be None
            asserts.assert_is_not_none(
                self.defaultRandomizationOffset, "DefaultRandomizationOffset attribute must not be None if RNDM feature is enabled.")

            # check that DefaultRandomizationOffset attribute is NullValue if DefaultRandomizationType attribute is NullValue
            if self.defaultRandomizationType is NullValue:
                asserts.assert_equal(self.defaultRandomizationOffset, NullValue,
                                     "DefaultRandomizationOffset attribute must be NullValue if DefaultRandomizationType is NullValue.")

            # check DefaultRandomizationOffset value depending on DefaultRandomizationType
            if self.defaultRandomizationOffset is not None and self.defaultRandomizationOffset is not NullValue:
                await self.check_randomization_offset(self.defaultRandomizationType, self.defaultRandomizationOffset)
        else:  # if feature is disabled, DefaultRandomizationOffset attribute must be None
            asserts.assert_is_none(
                self.defaultRandomizationOffset, "DefaultRandomizationOffset attribute must be None if RNDM feature is disabled.")

    async def check_default_randomization_type_attribute(self, endpoint: int, attribute_value: Optional[cluster.Enums.DayEntryRandomizationTypeEnum] = None) -> None:

        self.defaultRandomizationType = attribute_value
        if self.defaultRandomizationType is None:
            if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DefaultRandomizationType):
                self.defaultRandomizationType = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationType)

        logger.info(f"DefaultRandomizationType attribute value is: {self.defaultRandomizationType}")

        if self.tariffInfoValue is not None and self.tariffInfoValue is NullValue:
            asserts.assert_equal(self.defaultRandomizationType, NullValue,
                                 "DefaultRandomizationType must be Null when TariffInfo is Null")

        # check if RNDM feature is enabled
        if (await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DefaultRandomizationType) and
                await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kRandomization)):
            # if feature is enabled, DefaultRandomizationType attribute must not be None
            asserts.assert_is_not_none(
                self.defaultRandomizationType, "DefaultRandomizationType attribute must not be None if RNDM feature is enabled.")

            # check DefaultRandomizationType must be of type DayEntryRandomizationTypeEnum and in range 0 - 4
            if self.defaultRandomizationType is not NullValue:
                matter_asserts.assert_valid_enum(
                    self.defaultRandomizationType, "DefaultRandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)
                asserts.assert_greater_equal(self.defaultRandomizationType, 0,
                                             "DefaultRandomizationType must be greater or equal than 0.")
                asserts.assert_less_equal(self.defaultRandomizationType, 4,
                                          "DefaultRandomizationType must be less or equal than 4.")
        else:  # if feature is disabled, DefaultRandomizationType attribute must be None
            asserts.assert_is_none(
                self.defaultRandomizationType, "DefaultRandomizationType attribute must be None if RNDM feature is disabled.")

    async def convert_matter_time_to_posix_epoch_time(self, matter_time: int) -> int:

        MATTER_UNIX_EPOCH_OFFSET = 10957 * 24 * 60 * 60

        return matter_time + MATTER_UNIX_EPOCH_OFFSET

    async def get_day_pattern_IDs_for_active_calendar_period(self, next: bool = False) -> List[int]:
        """Define active calendar period based on CurrentDayEntryDateValue or NextDayEntryDateValue and extract DayPatternIDs field.

        Args:
            next (bool, optional): If True, use NextDayEntryDateValue. Defaults to False (Use CurrentDayEntryDateValue).

        Returns:
            List[int]: DayPatternIDs field of active calendar period.
        """

        day_patterns_IDs = []

        if not next:
            date = self.currentDayEntryDateValue
        else:
            date = self.nextDayEntryDateValue

        for period in self.calendarPeriodsValue[::-1]:
            if period.startDate < date:
                day_patterns_IDs = period.dayPatternIDs

        return day_patterns_IDs

    async def get_day_of_week_from_day_entry_date(self, day_entry_date: int) -> int:

        weekDays = {
            0: cluster.Bitmaps.DayPatternDayOfWeekBitmap.kMonday,
            1: cluster.Bitmaps.DayPatternDayOfWeekBitmap.kTuesday,
            2: cluster.Bitmaps.DayPatternDayOfWeekBitmap.kWednesday,
            3: cluster.Bitmaps.DayPatternDayOfWeekBitmap.kThursday,
            4: cluster.Bitmaps.DayPatternDayOfWeekBitmap.kFriday,
            5: cluster.Bitmaps.DayPatternDayOfWeekBitmap.kSaturday,
            6: cluster.Bitmaps.DayPatternDayOfWeekBitmap.kSunday
        }

        day_entry_date_epoch = await self.convert_matter_time_to_posix_epoch_time(day_entry_date)
        dayOfWeek = datetime.datetime.fromtimestamp(day_entry_date_epoch).weekday()

        return weekDays[dayOfWeek]

    async def get_day_entry_IDs_from_tariff_periods_for_particular_tariff_component(self, tariff_componentID: int) -> List[int]:
        """Extracts DayEntryIDs field values from the list of TariffPeriodStruct where particular TariffComponentID is present.

        Args:
            tariff_componentID (int): TariffComponentID to search for in TariffComponentIDs field.

        Returns:
            List[int]: List of all DayEntryIDs from the list of TariffPeriodStruct where particular TariffComponentID is present.
        """

        return sorted({dayEntryID for tariff_period in self.tariffPeriodsValue if tariff_componentID in tariff_period.tariffComponentIDs for dayEntryID in tariff_period.dayEntryIDs})

    async def get_tariff_period_label_for_particular_tariff_component(self, tariff_componentID: int) -> List[int]:
        """Gets Label field from the TariffPeriodStruct where particular TariffComponentID is present.

        Args:
            tariff_componentID (int): TariffComponentID to search for in TariffComponentIDs field.

        Returns:
            List[int]: Label field from the TariffPeriodStruct where particular TariffComponentID is present.
        """

        for tariff_period in self.tariffPeriodsValue:
            if tariff_componentID in tariff_period.tariffComponentIDs:
                return tariff_period.label
        return None

    async def get_tariff_components_IDs_from_tariff_components_attribute(self, tariff_components: List[cluster.Structs.TariffComponentStruct]) -> List[int]:
        """Extracts TariffComponentIDs from the list of TariffComponentStruct entities.

        Args:
            tariff_components (List[cluster.Structs.TariffComponentStruct]): List of TariffComponentStruct entities.

        Returns:
            List[int]: List of TariffComponentIDs only.
        """

        return [tariff_component.tariffComponentID for tariff_component in tariff_components]

    async def get_tariff_components_by_its_IDs(self, tariff_components_IDs: List[int]) -> List[cluster.Structs.TariffComponentStruct]:
        """Extracts TariffComponentStruct entities by TariffComponentIDs.

        Args:
            tariff_components_IDs (List[int]): List of TariffComponentIDs.

        Returns:
            List[cluster.Structs.TariffComponentStruct]: List of TariffComponentStruct entities with specified TariffComponentIDs.
        """

        if self.tariffComponentsValue is None:
            self.tariffComponentsValue = await self.read_single_attribute_check_success(endpoint=self.get_endpoint(), cluster=cluster, attribute=cluster.Attributes.TariffComponents)

        return [tariff_component for tariff_component in self.tariffComponentsValue if tariff_component.tariffComponentID in tariff_components_IDs]

    async def get_day_entry_IDs_from_day_entries_attribute(self, tariff_components: List[cluster.Structs.DayEntryStruct]) -> List[int]:
        """Extracts DayEntryIDs from the list of DayEntryStruct entities.

        Args:
            tariff_components (List[cluster.Structs.DayEntryStruct]): List of DayEntryStruct entities.

        Returns:
            List[int]: List of DayEntryIDs only.
        """

        return [tariff_component.dayEntryID for tariff_component in tariff_components]

    async def generate_unique_uint32_for_IDs(self, list_of_IDs: List[int]) -> int:
        """This function generates random ID (uint32) that is not in the list of given IDs.
        Is intended to be used in TC_SETRF_2_2.

        Args:
            list_of_IDs (List[int]): List of IDs (for example, DayEntryIDs or TariffComponentIDs).

        Returns:
            int: ID (uint32) that is not in the list of given IDs
        """

        IDs_set = set(list_of_IDs)

        while True:
            new_ID = random.randint(0, 2**32 - 1)
            if new_ID not in IDs_set:
                return new_ID

    async def verify_reporting(self, reports: dict, attribute: ClusterObjects.ClusterAttributeDescriptor, attribute_name: str, saved_value) -> bool:
        """This function verifies that the reported value is different from the saved value.

        Args:
            reports (dict): All reports stored in attribute_reports attribute in subscription handler object;
            attribute (ClusterObjects.ClusterAttributeDescriptor): Attribute to check;
            attribute_name (str): String with attribute name for error message;
            saved_value: Value that has been saved before TestEventTrigger sending;

        Returns:
            bool: - True if report is presented;
                  - False if there are no reports at all;
        """

        try:
            asserts.assert_not_equal(reports[attribute][0].value, saved_value,
                                     f"""Reported '{attribute_name}' value should be different from saved value.
                                     Subscriptions should only report when values have changed.""")
            return True
        except (KeyError, IndexError) as err:
            asserts.fail(f"There are no reports for attribute {attribute_name}:\n{err}")
            return False

    @staticmethod
    def _tariff_info_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.TariffInfo:
                return True
            return False
        return AttributeMatcher.from_callable(description="TariffInfo", matcher=predicate)

    @staticmethod
    def _tariff_unit_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.TariffUnit:
                return True
            return False
        return AttributeMatcher.from_callable(description="TariffUnit", matcher=predicate)

    @staticmethod
    def _start_date_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.StartDate:
                return True
            return False
        return AttributeMatcher.from_callable(description="StartDate", matcher=predicate)

    @staticmethod
    def _day_entries_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.DayEntries:
                return True
            return False
        return AttributeMatcher.from_callable(description="DayEntries", matcher=predicate)

    @staticmethod
    def _day_patterns_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.DayPatterns:
                return True
            return False
        return AttributeMatcher.from_callable(description="DayPatterns", matcher=predicate)

    @staticmethod
    def _calendar_periods_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.CalendarPeriods:
                return True
            return False
        return AttributeMatcher.from_callable(description="CalendarPeriods", matcher=predicate)

    @staticmethod
    def _individual_days_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.IndividualDays:
                return True
            return False
        return AttributeMatcher.from_callable(description="IndividualDays", matcher=predicate)

    @staticmethod
    def _current_day_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.CurrentDay:
                return True
            return False
        return AttributeMatcher.from_callable(description="CurrentDay", matcher=predicate)

    @staticmethod
    def _next_day_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.NextDay:
                return True
            return False
        return AttributeMatcher.from_callable(description="NextDay", matcher=predicate)

    @staticmethod
    def _current_day_entry_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.CurrentDayEntry:
                return True
            return False
        return AttributeMatcher.from_callable(description="CurrentDayEntry", matcher=predicate)

    @staticmethod
    def _current_day_entry_date_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.CurrentDayEntryDate:
                return True
            return False
        return AttributeMatcher.from_callable(description="CurrentDayEntryDate", matcher=predicate)

    @staticmethod
    def _next_day_entry_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.NextDayEntry:
                return True
            return False
        return AttributeMatcher.from_callable(description="NextDayEntry", matcher=predicate)

    @staticmethod
    def _next_day_entry_date_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.NextDayEntryDate:
                return True
            return False
        return AttributeMatcher.from_callable(description="NextDayEntryDate", matcher=predicate)

    @staticmethod
    def _tariff_components_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.TariffComponents:
                return True
            return False
        return AttributeMatcher.from_callable(description="TariffComponents", matcher=predicate)

    @staticmethod
    def _tariff_periods_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.TariffPeriods:
                return True
            return False
        return AttributeMatcher.from_callable(description="TariffPeriods", matcher=predicate)

    @staticmethod
    def _current_tariff_component_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.CurrentTariffComponents:
                return True
            return False
        return AttributeMatcher.from_callable(description="CurrentTariffComponents", matcher=predicate)

    @staticmethod
    def _next_tariff_component_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.NextTariffComponents:
                return True
            return False
        return AttributeMatcher.from_callable(description="NextTariffComponents", matcher=predicate)

    @staticmethod
    def _default_randomization_offset_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.DefaultRandomizationOffset:
                return True
            return False
        return AttributeMatcher.from_callable(description="DefaultRandomizationOffset", matcher=predicate)

    @staticmethod
    def _default_randomization_type_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            if report.attribute == cluster.Attributes.DefaultRandomizationType:
                return True
            return False
        return AttributeMatcher.from_callable(description="DefaultRandomizationType", matcher=predicate)

    def get_mandatory_matchers(self) -> List[AttributeMatcher]:

        return [
            self._tariff_info_matcher(),
            self._tariff_unit_matcher(),
            self._start_date_matcher(),
            self._day_entries_matcher(),
            self._day_patterns_matcher(),
            self._calendar_periods_matcher(),
            self._individual_days_matcher(),
            self._current_day_matcher(),
            self._next_day_matcher(),
            self._current_day_entry_matcher(),
            self._current_day_entry_date_matcher(),
            self._next_day_entry_matcher(),
            self._next_day_entry_date_matcher(),
            self._tariff_components_matcher(),
            self._tariff_periods_matcher(),
            self._current_tariff_component_matcher(),
            self._next_tariff_component_matcher()
        ]

    async def search_day_pattern_for_given_day_of_week_check_day_entry_IDs(self, list_of_day_pattern_IDs: List[int],
                                                                           day_of_week: int,
                                                                           list_of_day_entry_IDs: List[int],
                                                                           current_or_next: str) -> None:
        """1. Searches DayPattern in currently active CalendarPeriod that contains the given day of week.
        2. Checks that DayEntryIDs from DayPattern are equal to DayEntryIDs from CurrentDay/NextDay attribute.

        Args:
            list_of_day_pattern_IDs (List[int]): List of DayPatternIDs from defined active CalendarPeriodStruct.
            day_of_week (int): Day of Week defined based on CurrentDayEntryDate/NextDayEntryDate attribute value.
            list_of_day_entry_IDs (List[int]): List of DayEntryIDs from CurrentDay/NextDay attribute.
            current_or_next (str): "Current" or "Next".
        """

        day_pattern_found = False

        for dayPattern in self.dayPatternsValue:
            if dayPattern.dayPatternID in list_of_day_pattern_IDs:
                if (dayPattern.daysOfWeek & day_of_week) == day_of_week:
                    asserts.assert_equal(sorted(dayPattern.dayEntryIDs), sorted(list_of_day_entry_IDs),
                                         f"DayEntryIDs from DayPatterns must be equal to DayEntryIDs from {current_or_next}Day.")
                    day_pattern_found = True
                    return

        if not day_pattern_found:
            asserts.fail(f"DayPattern not found for {current_or_next}DayEntryDate attribute value.")

    async def validate_tariff_component_ID_uniqueness_for_features(self, tariff_components_list: List[cluster.Structs.TariffComponentStruct]) -> None:

        # flags to check specific feature fields in mask
        features_fields_flags = {
            "price": 1 << 0,
            "friendlyCredit": 1 << 1,
            "auxiliaryLoad": 1 << 2,
            "peakPeriod": 1 << 3,
            "powerThreshold": 1 << 4
        }

        # store feature mask for each unique threshold value
        groups_by_threshold_field = {}

        # iterate over all tariff components
        for tariff_component in tariff_components_list:

            # skip predicted tariff components
            if tariff_component.predicted is True:
                continue

            # Add Threshold value if it is not in groups_by_threshold_field
            if tariff_component.threshold not in groups_by_threshold_field:
                groups_by_threshold_field[tariff_component.threshold] = 0

            # feature mask for current tariff_component
            current_component_features_flags = 0

            # iterate over all feature flags
            for feature_flag in features_fields_flags.keys():

                current_component_features_flags |= features_fields_flags[feature_flag] if getattr(
                    tariff_component, feature_flag, None) is not None else current_component_features_flags | 0

            # validate that there are no duplicate feature fields (comparing bitwise AND with current component feature flags mask and
            # feature mask for current threshold value in groups_by_threshold_field)
            asserts.assert_equal(current_component_features_flags & groups_by_threshold_field[tariff_component.threshold], 0,
                                 f"TariffComponentID {tariff_component.tariffComponentID} must have unique combination of feature fields for given Threshold.")

            # If validation above is successful then update feature mask for current threshold based on current component feature flags
            groups_by_threshold_field[tariff_component.threshold] |= current_component_features_flags
