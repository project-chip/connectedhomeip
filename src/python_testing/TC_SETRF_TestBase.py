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


import logging

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from mobly import asserts

logger = logging.getLogger(__name__)


class CommodityTariffTestBaseHelper:

    StartDate = None
    StartTime = None

    async def checkAuxiliaryLoadSwitchSettingsStruct(self,
                                                     endpoint: int = None,
                                                     cluster: Clusters.CommodityTariff = None,
                                                     struct: Clusters.CommodityTariff.Structs.AuxiliaryLoadSwitchSettingsStruct = None):
        matter_asserts.assert_valid_uint8(struct.number, 'Number')
        matter_asserts.assert_valid_enum(
            struct.requiredState, "RequiredState attribute must return a AuxiliaryLoadSettingEnum", cluster.Enums.AuxiliaryLoadSettingEnum)

    async def checkCalendarPeriodStruct(self,
                                        endpoint: int = None,
                                        cluster: Clusters.CommodityTariff = None,
                                        struct: Clusters.CommodityTariff.Structs.CalendarPeriodStruct = None):
        if struct.startDate is not NullValue:
            matter_asserts.assert_valid_uint32(struct.startDate, 'StartDate')
            asserts.assert_greater_equal(struct.startDate, self.StartDate)
        matter_asserts.assert_list(struct.dayPatternIDs, "DayPatternIDs attribute must return a list")
        matter_asserts.assert_list_element_type(
            struct.dayPatternIDs, int, "DayPatternIDs attribute must contain int elements")
        asserts.assert_greater_equal(len(struct.dayPatternIDs), 1, "DayPatternIDs must have at least 1 entries!")
        asserts.assert_less_equal(len(struct.dayPatternIDs), 7, "DayPatternIDs must have at most 7 entries!")

    async def checkCurrencyStruct(self,
                                  endpoint: int = None,
                                  cluster: Clusters.CommodityTariff = None,
                                  struct: Globals.Structs.CurrencyStruct = None):
        matter_asserts.assert_valid_uint16(struct.currency, 'Currency')
        asserts.assert_less_equal(struct.currency, 999)
        matter_asserts.assert_valid_uint8(struct.decimalPoints, 'DecimalPoints')

    async def checkDayEntryStruct(self,
                                  endpoint: int = None,
                                  cluster: Clusters.CommodityTariff = None,
                                  struct: Clusters.CommodityTariff.Structs.DayEntryStruct = None):
        matter_asserts.assert_valid_uint32(struct.dayEntryID, 'DayEntryID')
        matter_asserts.assert_valid_uint16(struct.startTime, 'StartTime')
        asserts.assert_less_equal(struct.startTime, 1499)
        if struct.duration is not None:
            matter_asserts.assert_valid_uint16(struct.duration, 'Duration')
            asserts.assert_less_equal(struct.duration, 1500 - struct.startTime)
        if self.check_pics("SETRF.S.F05"):
            matter_asserts.assert_valid_int16(struct.randomizationOffset, 'RandomizationOffset')
            matter_asserts.assert_valid_enum(
                struct.randomizationType, "RandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)
        else:
            asserts.assert_is_none(struct.randomizationOffset, "RandomizationOffset must be None")
            asserts.assert_is_none(struct.randomizationType, "RandomizationType must be None")

    async def checkDayPatternStruct(self,
                                    endpoint: int = None,
                                    cluster: Clusters.CommodityTariff = None,
                                    struct: Clusters.CommodityTariff.Structs.DayPatternStruct = None):
        matter_asserts.assert_valid_uint32(struct.dayPatternID, 'DayPatternID')
        matter_asserts.is_valid_int_value(struct.daysOfWeek)
        # Check bitmap value less than or equal to (Sunday | Monday | Tuesday | Wednesday | Thursday | Friday | Saturday)
        asserts.assert_less_equal(struct.daysOfWeek, 127)
        matter_asserts.assert_list(struct.dayEntryIDs, "DayEntryIDs attribute must return a list")
        matter_asserts.assert_list_element_type(
            struct.dayEntryIDs, int, "DayEntryIDs attribute must contain int elements")
        asserts.assert_greater_equal(len(struct.dayEntryIDs), 1, "DayEntryIDs must have at least 1 entries!")
        asserts.assert_less_equal(len(struct.dayEntryIDs), 96, "DayEntryIDs must have at most 96 entries!")

    async def checkDayStruct(self,
                             endpoint: int = None,
                             cluster: Clusters.CommodityTariff = None,
                             struct: Clusters.CommodityTariff.Structs.DayStruct = None):
        matter_asserts.assert_valid_uint32(struct.date, 'Date')
        matter_asserts.assert_valid_enum(
            struct.dayType, "DayType attribute must return a DayTypeEnum", cluster.Enums.DayTypeEnum)
        matter_asserts.assert_list(struct.dayEntryIDs, "DayEntryIDs attribute must return a list")
        matter_asserts.assert_list_element_type(
            struct.dayEntryIDs, int, "DayEntryIDs attribute must contain int elements")
        asserts.assert_greater_equal(len(struct.dayEntryIDs), 1, "DayEntryIDs must have at least 1 entries!")
        asserts.assert_less_equal(len(struct.dayEntryIDs), 96, "DayEntryIDs must have at most 96 entries!")

    async def checkPeakPeriodStruct(self,
                                    endpoint: int = None,
                                    cluster: Clusters.CommodityTariff = None,
                                    struct: Clusters.CommodityTariff.Structs.PeakPeriodStruct = None):
        matter_asserts.assert_valid_enum(
            struct.severity, "Severity attribute must return a PeakPeriodSeverityEnum", cluster.Enums.PeakPeriodSeverityEnum)
        matter_asserts.assert_valid_uint16(struct.peakPeriod, 'PeakPeriod')
        asserts.assert_greater_equal(struct.peakPeriod, 1)

    async def checkPowerThresholdStruct(self,
                                        endpoint: int = None,
                                        cluster: Clusters.CommodityTariff = None,
                                        struct: Globals.Structs.PowerThresholdStruct = None):
        if struct.powerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.powerThreshold, 'PowerThreshold')
        if struct.apparentPowerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.apparentPowerThreshold, 'ApparentPowerThreshold')
        if struct.powerThresholdSource is not NullValue:
            matter_asserts.assert_valid_enum(
                struct.powerThresholdSource, "PowerThresholdSource attribute must return a PowerThresholdSourceEnum", Globals.Enums.PowerThresholdSourceEnum)

    async def checkTariffComponentStruct(self,
                                         endpoint: int = None,
                                         cluster: Clusters.CommodityTariff = None,
                                         struct: Clusters.CommodityTariff.Structs.TariffComponentStruct = None):
        matter_asserts.assert_valid_uint32(struct.tariffComponentID, 'TariffComponentID')
        if self.check_pics("SETRF.S.F00"):
            if struct.price is not NullValue:
                asserts.assert_true(isinstance(
                    struct.price, cluster.Structs.TariffPriceStruct), "struct.price must be of type TariffPriceStruct")
                await self.checkTariffPriceStruct(endpoint=endpoint, cluster=cluster, struct=struct.price)
        else:
            asserts.assert_is_none(struct.price, "Price must be None")
        if self.check_pics("SETRF.S.F01"):
            matter_asserts.assert_valid_bool(struct.friendlyCredit, 'FriendlyCredit')
        else:
            asserts.assert_is_none(struct.friendlyCredit, "FriendlyCredit must be None")
        if self.check_pics("SETRF.S.F02"):
            asserts.assert_true(isinstance(
                struct.auxiliaryLoad, cluster.Structs.AuxiliaryLoadSwitchSettingsStruct), "struct.auxiliaryLoad must be of type AuxiliaryLoadSwitchSettingsStruct")
            await self.checkAuxiliaryLoadSwitchSettingsStruct(endpoint=endpoint, cluster=cluster, struct=struct.auxiliaryLoad)
        else:
            asserts.assert_is_none(struct.auxiliaryLoad, "AuxiliaryLoad must be None")
        if self.check_pics("SETRF.S.F03"):
            asserts.assert_true(isinstance(
                struct.peakPeriod, cluster.Structs.PeakPeriodStruct), "struct.peakPeriod must be of type PeakPeriodStruct")
            await self.checkPeakPeriodStruct(endpoint=endpoint, cluster=cluster, struct=struct.peakPeriod)
        else:
            asserts.assert_is_none(struct.peakPeriod, "PeakPeriod must be None")
        if self.check_pics("SETRF.S.F04"):
            asserts.assert_true(isinstance(
                struct.powerThreshold, Globals.Structs.PowerThresholdStruct), "struct.powerThreshold must be of type PowerThresholdStruct")
            await self.checkPowerThresholdStruct(endpoint=endpoint, cluster=cluster, struct=struct.powerThreshold)
        else:
            asserts.assert_is_none(struct.powerThreshold, "PowerThreshold must be None")
        if struct.threshold is not NullValue:
            matter_asserts.assert_valid_uint32(struct.threshold, 'Threshold')
        if struct.label is not NullValue and struct.label is not None:
            matter_asserts.assert_is_string(struct.label, "Label must be a string")
            asserts.assert_less_equal(len(struct.label), 128, "Label must have length at most 128!")
        if struct.predicted is not None:
            matter_asserts.assert_valid_bool(struct.predicted, 'Predicted')

    async def checkTariffInformationStruct(self,
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
            asserts.assert_true(isinstance(
                struct.currency, Globals.Structs.CurrencyStruct), "struct.currency must be of type CurrencyStruct")
            await self.checkCurrencyStruct(endpoint=endpoint, cluster=cluster, struct=struct.currency)
        if struct.blockMode is not NullValue:
            matter_asserts.assert_valid_enum(
                struct.blockMode, "BlockMode attribute must return a BlockModeEnum", cluster.Enums.BlockModeEnum)

    async def checkTariffPeriodStruct(self,
                                      endpoint: int = None,
                                      cluster: Clusters.CommodityTariff = None,
                                      struct: Clusters.CommodityTariff.Structs.TariffPeriodStruct = None):
        if struct.label is not NullValue:
            matter_asserts.assert_is_string(struct.label, "Label must be a string")
            asserts.assert_less_equal(len(struct.label), 128, "Label must have length at most 128!")
        matter_asserts.assert_list(struct.dayEntryIDs, "DayEntryIDs attribute must return a list")
        matter_asserts.assert_list_element_type(
            struct.dayEntryIDs, int, "DayEntryIDs attribute must contain int elements")
        asserts.assert_greater_equal(len(struct.dayEntryIDs), 1, "DayEntryIDs must have at least 1 entries!")
        asserts.assert_less_equal(len(struct.dayEntryIDs), 20, "DayEntryIDs must have at most 20 entries!")
        matter_asserts.assert_list(struct.tariffComponentIDs, "TariffComponentIDs attribute must return a list")
        matter_asserts.assert_list_element_type(
            struct.tariffComponentIDs, int, "TariffComponentIDs attribute must contain int elements")
        asserts.assert_greater_equal(len(struct.tariffComponentIDs), 1, "TariffComponentIDs must have at least 1 entries!")
        asserts.assert_less_equal(len(struct.tariffComponentIDs), 20, "TariffComponentIDs must have at most 20 entries!")

    async def checkTariffPriceStruct(self,
                                     endpoint: int = None,
                                     cluster: Clusters.CommodityTariff = None,
                                     struct: Clusters.CommodityTariff.Structs.TariffPriceStruct = None):
        matter_asserts.assert_valid_enum(
            struct.priceType, "PriceType attribute must return a TariffPriceTypeEnum", Globals.Enums.TariffPriceTypeEnum)
        if struct.price is not None:
            matter_asserts.assert_valid_int64(struct.price, 'Price')
        if struct.priceLevel is not None:
            matter_asserts.assert_valid_int16(struct.priceLevel, 'PriceLevel')
