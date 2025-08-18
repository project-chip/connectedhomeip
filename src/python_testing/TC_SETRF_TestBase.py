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
import time

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.matter_testing import MatterBaseTest

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class CommodityTariffTestBaseHelper(MatterBaseTest):
    """This class contains supporting methods for the CommodityTariff test cases."""

    # Test event trigger IDs
    EventTriggerFakeData = 0x0700000000000000
    EventTriggerClear = 0x0700000000000001
    EventTriggerChangeDay = 0x0700000000000002
    EventTriggerChangeTime = 0x0700000000000003

    async def checkAuxiliaryLoadSwitchSettingsStruct(self,
                                                     endpoint: int = None,
                                                     cluster: Clusters.CommodityTariff = None,
                                                     struct: Clusters.CommodityTariff.Structs.AuxiliaryLoadSwitchSettingsStruct = None):
        """
        Checks the correctness of the AuxiliaryLoadSwitchSettingsStruct data type entries.
        """

        matter_asserts.assert_valid_uint8(struct.number, 'Number must has uint8 type.')
        matter_asserts.assert_valid_enum(
            struct.requiredState, "RequiredState attribute must return a AuxiliaryLoadSettingEnum", cluster.Enums.AuxiliaryLoadSettingEnum)
        asserts.assert_greater_equal(struct.requiredState, 0)
        asserts.assert_less_equal(struct.requiredState, 2)

    async def checkCalendarPeriodStruct(self,
                                        endpoint: int = None,
                                        cluster: Clusters.CommodityTariff = None,
                                        struct: Clusters.CommodityTariff.Structs.CalendarPeriodStruct = None,
                                        start_date_attribute: int = None):
        """
        Checks the correctness of the CalendarPeriodStruct data type entries."""

        if struct.startDate is not NullValue:
            matter_asserts.assert_valid_uint32(struct.startDate, 'StartDate')
            asserts.assert_greater_equal(struct.startDate, start_date_attribute)
        matter_asserts.assert_list(
            struct.dayPatternIDs, "DayPatternIDs attribute must return a list with length in range 1 - 7", min_length=1, max_length=7)
        for dayPatternID in struct.dayPatternIDs:
            matter_asserts.assert_valid_uint32(dayPatternID, 'DayPatternID must has uint32 type.')

    async def checkCurrencyStruct(self,
                                  endpoint: int = None,
                                  cluster: Clusters.CommodityTariff = None,
                                  struct: Globals.Structs.CurrencyStruct = None):
        """
        Checks the correctness of the CurrencyStruct data type entries."""

        matter_asserts.assert_valid_uint16(struct.currency, 'Currency')
        asserts.assert_less_equal(struct.currency, 999)
        matter_asserts.assert_valid_uint8(struct.decimalPoints, 'DecimalPoints')

    async def checkDayEntryStruct(self,
                                  endpoint: int = None,
                                  cluster: Clusters.CommodityTariff = None,
                                  struct: Clusters.CommodityTariff.Structs.DayEntryStruct = None):
        """
        Checks the correctness of the DayEntryStruct data type entries."""

        matter_asserts.assert_valid_uint32(struct.dayEntryID, 'DayEntryID must has uint32 type.')
        matter_asserts.assert_valid_uint16(struct.startTime, 'StartTime must has uint16 type.')
        asserts.assert_less_equal(struct.startTime, 1499)
        if struct.duration is not None:
            matter_asserts.assert_valid_uint16(struct.duration, 'Duration must has uint16 type.')
            asserts.assert_less_equal(struct.duration, 1500 - struct.startTime)
        if self.check_pics("SETRF.S.F05"):
            if struct.randomizationType is not None:
                matter_asserts.assert_valid_enum(
                    struct.randomizationType, "RandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)
                asserts.assert_greater_equal(struct.randomizationType, 0)
                asserts.assert_less_equal(struct.randomizationType, 4)
            if struct.randomizationOffset is not None:
                if struct.randomizationType is not None and struct.randomizationType == 1:
                    matter_asserts.assert_valid_int16(struct.randomizationOffset, 'RandomizationOffset must has int16 type.')
                elif struct.randomizationType is not None and struct.randomizationType == 4:
                    matter_asserts.assert_valid_int16(struct.randomizationOffset, 'RandomizationOffset must has int16 type.')
                    asserts.assert_less_equal(struct.randomizationOffset, 0, "RandomizationOffset must be less than 0.")
                else:
                    matter_asserts.assert_valid_int16(struct.randomizationOffset, 'RandomizationOffset must has int16 type.')
                    asserts.assert_greater_equal(struct.randomizationOffset, 0, "RandomizationOffset must be greater than 0.")
        else:
            asserts.assert_is_none(struct.randomizationOffset, "RandomizationOffset must be None")
            asserts.assert_is_none(struct.randomizationType, "RandomizationType must be None")

    async def checkDayPatternStruct(self,
                                    endpoint: int = None,
                                    cluster: Clusters.CommodityTariff = None,
                                    struct: Clusters.CommodityTariff.Structs.DayPatternStruct = None):
        """
        Checks the correctness of the DayPatternStruct data type entries."""

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
        """
        Checks the correctness of the DayStruct data type entries."""

        matter_asserts.assert_valid_uint32(struct.date, 'Date field must has epoch-s type.')
        matter_asserts.assert_valid_enum(
            struct.dayType, "DayType field must has a DayTypeEnum", cluster.Enums.DayTypeEnum)
        asserts.assert_greater_equal(struct.dayType, 0, "DayType must be greater than 0.")
        asserts.assert_less_equal(struct.dayType, 3, "DayType must be less than 3.")
        matter_asserts.assert_list(
            struct.dayEntryIDs, "DayEntryIDs attribute must return a list with length in range 1 - 96", min_length=1, max_length=96)
        for dayEntryID in struct.dayEntryIDs:
            matter_asserts.assert_valid_uint32(dayEntryID, 'DayEntryID must has uint32 type.')

    async def checkPeakPeriodStruct(self,
                                    endpoint: int = None,
                                    cluster: Clusters.CommodityTariff = None,
                                    struct: Clusters.CommodityTariff.Structs.PeakPeriodStruct = None):
        """
        Checks the correctness of the PeakPeriodStruct data type entries."""

        matter_asserts.assert_valid_enum(
            struct.severity, "Severity attribute must return a PeakPeriodSeverityEnum", cluster.Enums.PeakPeriodSeverityEnum)
        asserts.assert_greater_equal(struct.severity, 0, "Severity must be greater than 0.")
        asserts.assert_less_equal(struct.severity, 3, "Severity must be less than 3.")
        matter_asserts.assert_valid_uint16(struct.peakPeriod, 'PeakPeriod must has uint16 type.')
        asserts.assert_greater_equal(struct.peakPeriod, 1, "PeakPeriod must be greater or equal than 1.")

    async def checkPowerThresholdStruct(self,
                                        endpoint: int = None,
                                        cluster: Clusters.CommodityTariff = None,
                                        struct: Globals.Structs.PowerThresholdStruct = None):
        """
        Checks the correctness of the PowerThresholdStruct data type entries."""

        if struct.powerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.powerThreshold, 'PowerThreshold must has power-mW type.')
        if struct.apparentPowerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.apparentPowerThreshold, 'ApparentPowerThreshold must has power-mVA type.')
        if struct.powerThresholdSource is not NullValue:
            matter_asserts.assert_valid_enum(
                struct.powerThresholdSource, "PowerThresholdSource attribute must return a PowerThresholdSourceEnum", Globals.Enums.PowerThresholdSourceEnum)
            asserts.assert_greater_equal(struct.powerThresholdSource, 0, "PowerThresholdSource must be greater than 0.")
            asserts.assert_less_equal(struct.powerThresholdSource, 2, "PowerThresholdSource must be less than 2.")

    async def checkTariffComponentStruct(self,
                                         endpoint: int = None,
                                         cluster: Clusters.CommodityTariff = None,
                                         struct: Clusters.CommodityTariff.Structs.TariffComponentStruct = None):
        """
        Checks the correctness of the TariffComponentStruct data type entries."""

        matter_asserts.assert_valid_uint32(struct.tariffComponentID, 'TariffComponentID must has uint32 type.')

        asserts.assert_true(any(struct.price, struct.friendlyCredit, struct.auxiliaryLoad, struct.peakPeriod, struct.powerThreshold),
                            'At least one field from price, friendlyCredit, auxiliaryLoad, peakPeriod, powerThreshold must be set')

        if self.check_pics("SETRF.S.F00"):
            if struct.price is not None and struct.price is not NullValue:
                asserts.assert_true(isinstance(
                    struct.price, cluster.Structs.TariffPriceStruct), "Price field must be of type TariffPriceStruct")
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
                struct.peakPeriod, cluster.Structs.PeakPeriodStruct), "PeakPeriod must be of type PeakPeriodStruct")
            await self.checkPeakPeriodStruct(endpoint=endpoint, cluster=cluster, struct=struct.peakPeriod)
        else:
            asserts.assert_is_none(struct.peakPeriod, "PeakPeriod must be None")

        if self.check_pics("SETRF.S.F04"):
            asserts.assert_true(isinstance(
                struct.powerThreshold, Globals.Structs.PowerThresholdStruct), "PowerThreshold must be of type PowerThresholdStruct")
            await self.checkPowerThresholdStruct(endpoint=endpoint, cluster=cluster, struct=struct.powerThreshold)
        else:
            asserts.assert_is_none(struct.powerThreshold, "PowerThreshold must be None")
        if struct.threshold is not NullValue:
            matter_asserts.assert_valid_int64(struct.threshold, 'Threshold must has int64 type.')
        if struct.label is not None and struct.label is not NullValue:
            matter_asserts.assert_is_string(struct.label, "Label must be a string")
            asserts.assert_less_equal(len(struct.label), 128, "Label must have length at most 128!")
        if struct.predicted is not None:
            matter_asserts.assert_valid_bool(struct.predicted, 'Predicted must has bool type.')

    async def checkTariffInformationStruct(self,
                                           endpoint: int = None,
                                           cluster: Clusters.CommodityTariff = None,
                                           struct: Clusters.CommodityTariff.Structs.TariffInformationStruct = None):
        """
        Checks the correctness of the TariffInformationStruct data type entries."""

        if struct.tariffLabel is not NullValue:
            matter_asserts.assert_is_string(struct.tariffLabel, "TariffLabel must be a string")
            asserts.assert_less_equal(len(struct.tariffLabel), 128, "TariffLabel must have length at most 128!")
        if struct.providerName is not NullValue:
            matter_asserts.assert_is_string(struct.providerName, "ProviderName must be a string")
            asserts.assert_less_equal(len(struct.providerName), 128, "ProviderName must have length at most 128!")
        if self.check_pics("SETRF.S.F00"):
            asserts.assert_true(struct.currency is not None, "Currency must have real value or can be Null")
            if struct.currency is not NullValue:
                asserts.assert_true(isinstance(
                    struct.currency, Globals.Structs.CurrencyStruct), "Currency must be of type CurrencyStruct")
                await self.checkCurrencyStruct(endpoint=endpoint, cluster=cluster, struct=struct.currency)
        else:
            asserts.assert_is_none(struct.currency, "Currency must be None")
        if struct.blockMode is not NullValue:
            matter_asserts.assert_valid_enum(
                struct.blockMode, "BlockMode attribute must return a BlockModeEnum", cluster.Enums.BlockModeEnum)

    async def checkTariffPeriodStruct(self,
                                      endpoint: int = None,
                                      cluster: Clusters.CommodityTariff = None,
                                      struct: Clusters.CommodityTariff.Structs.TariffPeriodStruct = None):
        """
        Checks the correctness of the TariffPeriodStruct data type entries."""

        if struct.label is not NullValue:
            matter_asserts.assert_is_string(struct.label, "Label must be a string")
            asserts.assert_less_equal(len(struct.label), 128, "Label must have length at most 128!")

        matter_asserts.assert_list(
            struct.dayEntryIDs, "DayEntryIDs attribute must return a list with length in range 1 - 20", min_length=1, max_length=20)
        for dayEntryID in struct.dayEntryIDs:
            matter_asserts.assert_valid_uint32(dayEntryID, 'DayEntryID must has uint32 type.')

        matter_asserts.assert_list(
            struct.tariffComponentIDs, "TariffComponentIDs attribute must return a list with length in range 1 - 20", min_length=1, max_length=20)
        for tariffComponentID in struct.tariffComponentIDs:
            matter_asserts.assert_valid_uint32(tariffComponentID, 'TariffComponentID must has uint32 type.')

    async def checkTariffPriceStruct(self,
                                     endpoint: int = None,
                                     cluster: Clusters.CommodityTariff = None,
                                     struct: Clusters.CommodityTariff.Structs.TariffPriceStruct = None):
        """
        Checks the correctness of the TariffPriceStruct data type entries."""

        matter_asserts.assert_valid_enum(
            struct.priceType, "PriceType attribute must return a TariffPriceTypeEnum", Globals.Enums.TariffPriceTypeEnum)
        asserts.assert_greater_equal(struct.priceType, 0, "PriceType field of TariffPriceStruct must be greater than 0.")
        asserts.assert_less_equal(struct.priceType, 4, "PriceType field of TariffPriceStruct must be less than 4.")
        if struct.price is not None:
            matter_asserts.assert_valid_int64(struct.price, 'Price field of TariffPriceStruct must be money')
        if struct.priceLevel is not None:
            matter_asserts.assert_valid_int16(struct.priceLevel, 'PriceLevel field of TariffPriceStruct must be int16')

    async def send_test_event_trigger_for_fake_data(self):
        """Sends test event triggers to propagate fake data to the attributes instead of Null
           values that are default values after cluster initialization."""

        await self.send_test_event_triggers(eventTrigger=self.EventTriggerFakeData)
        time.sleep(3)  # Wait some time to be sure that fake data is propagated

    async def send_test_event_trigger_clear(self):
        """Reset cluster state to default (Null) values."""

        await self.send_test_event_triggers(eventTrigger=self.EventTriggerClear)
        time.sleep(3)  # Wait some time to be sure that the cluster state has been reset

    async def send_test_event_trigger_change_day(self):
        """This test event trigger ensure time shifting on 24h to simulate a day change."""

        await self.send_test_event_triggers(eventTrigger=self.EventTriggerChangeDay)
        time.sleep(3)  # Wait some time to be sure that test event triggers takes effect

    async def send_test_event_trigger_change_time(self):
        """This test event trigger ensure time shifting on 4h to simulate a time change."""

        await self.send_test_event_triggers(eventTrigger=self.EventTriggerChangeTime)
        time.sleep(3)  # Wait some time to be sure that test event triggers takes effect

    async def check_tariff_info_attribute(self, endpoint):

        tariff_info = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo)
        if tariff_info is not NullValue:
            asserts.assert_true(isinstance(
                tariff_info, cluster.Structs.TariffInformationStruct), "TariffInfo must be of type TariffInformationStruct")
            await self.checkTariffInformationStruct(endpoint=endpoint, cluster=cluster, struct=tariff_info)

    async def check_tariff_unit_attribute(self, endpoint):

        tariff_unit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit)
        if tariff_unit is not NullValue:
            matter_asserts.assert_valid_enum(
                tariff_unit, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)
            asserts.assert_true(tariff_unit >= 0 and tariff_unit <= 1, "TariffUnit must be in range 0 - 1")

    async def check_start_date_attribute(self, endpoint):

        self.startDateAttributeValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate)
        if self.startDateAttributeValue is not NullValue:
            matter_asserts.assert_valid_uint32(self.startDateAttributeValue, 'StartDate attribute must has uint32 type.')

    async def check_day_entries_attribute(self, endpoint):

        day_entries = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries)
        if day_entries is not NullValue:
            matter_asserts.assert_list(day_entries, "DayEntries attribute must return a list")
            matter_asserts.assert_list_element_type(
                day_entries, cluster.Structs.DayEntryStruct, "DayEntries attribute must contain DayEntryStruct elements")
            for item in day_entries:
                await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=item)

    async def check_day_patterns_attribute(self, endpoint):

        day_patterns = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
        if day_patterns is not NullValue:
            matter_asserts.assert_list(
                day_patterns, "DayPatterns attribute must return a list with length less or equal 28", min_length=0, max_length=28)
            if day_patterns:
                matter_asserts.assert_list_element_type(
                    day_patterns, cluster.Structs.DayPatternStruct, "DayPatterns attribute must contain DayPatternStruct elements")
                for item in day_patterns:
                    await self.checkDayPatternStruct(endpoint=endpoint, cluster=cluster, struct=item)

    async def check_calendar_periods_attribute(self, endpoint):

        calendar_periods = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods)
        if calendar_periods is not NullValue:
            matter_asserts.assert_list(
                calendar_periods, "CalendarPeriods attribute must return a list with length in range 1 - 4", min_length=1, max_length=4)
            matter_asserts.assert_list_element_type(
                calendar_periods, cluster.Structs.CalendarPeriodStruct, "CalendarPeriods attribute must contain CalendarPeriodStruct elements")
            for item in calendar_periods:
                await self.checkCalendarPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item, start_date_attribute=self.startDateAttributeValue)
            for item in range(len(calendar_periods) - 1):
                asserts.assert_less(calendar_periods[item].startDate, calendar_periods[item + 1].startDate,
                                    "CalendarPeriods must be sorted by Date in increasing order!")
            if self.startDateAttributeValue is NullValue:
                asserts.assert_true(calendar_periods[0].startDate is NullValue,
                                    "If StartDate is Null, the first CalendarPeriod item Start Date field must also be Null")
                if len(calendar_periods) > 1:
                    for item in range(1, len(calendar_periods)):
                        asserts.assert_true(calendar_periods[item].startDate is not NullValue,
                                            "If StartDate is Null only first CalendarPeriod item Start Date field must be Null, the other CalendarPeriod items Start Date field must not be Null")

    async def check_individual_days_attribute(self, endpoint):

        individual_days = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays)
        if individual_days is not NullValue:
            matter_asserts.assert_list(
                individual_days, "IndividualDays attribute must return a list with length less or equal 50", max_length=50)
            matter_asserts.assert_list_element_type(
                individual_days, cluster.Structs.DayStruct, "IndividualDays attribute must contain DayStruct elements")
            for item in individual_days:
                await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=item)
            for item in range(len(individual_days) - 1):
                asserts.assert_less(individual_days[item].date, individual_days[item + 1].date,
                                    "IndividualDays must be sorted by Date in increasing order!")

    async def check_current_day_attribute(self, endpoint):

        current_day = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)
        if current_day is not NullValue:
            asserts.assert_true(isinstance(
                current_day, cluster.Structs.DayStruct), "CurrentDay must be of type DayStruct")
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=current_day)

    async def check_next_day_attribute(self, endpoint):

        next_day = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        if next_day is not NullValue:
            asserts.assert_true(isinstance(
                next_day, cluster.Structs.DayStruct), "NextDay must be of type DayStruct")
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=next_day)

    async def check_current_day_entry_attribute(self, endpoint):

        current_day_entry = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntry)
        if current_day_entry is not NullValue:
            asserts.assert_true(isinstance(
                current_day_entry, cluster.Structs.DayEntryStruct), "CurrentDayEntry must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=current_day_entry)

    async def check_current_day_entry_date_attribute(self, endpoint):

        current_day_entry_date = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)
        if current_day_entry_date is not NullValue:
            matter_asserts.assert_valid_uint32(current_day_entry_date, 'CurrentDayEntryDate must be of type uint32')

    async def check_next_day_entry_attribute(self, endpoint):

        next_day_entry = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntry)
        if next_day_entry is not NullValue:
            asserts.assert_true(isinstance(
                next_day_entry, cluster.Structs.DayEntryStruct), "NextDayEntry must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=next_day_entry)

    async def check_next_day_entry_date_attribute(self, endpoint):

        next_day_entry_date = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        if next_day_entry_date is not NullValue:
            matter_asserts.assert_valid_uint32(next_day_entry_date, 'NextDayEntryDate must be of type uint32')

    async def check_tariff_components_attribute(self, endpoint):

        tariff_components = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        if tariff_components is not NullValue:
            matter_asserts.assert_list(
                tariff_components, "TariffComponents attribute must return a list with length greater or equal 1", min_length=1)
            matter_asserts.assert_list_element_type(
                tariff_components, cluster.Structs.TariffComponentStruct, "TariffComponents attribute must contain TariffComponentStruct elements")
            for item in tariff_components:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

    async def check_tariff_periods_attribute(self, endpoint):

        tariff_periods = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)
        if tariff_periods is not NullValue:
            matter_asserts.assert_list(
                tariff_periods, "TariffPeriods attribute must return a list with length greater or equal 1", min_length=1)
            matter_asserts.assert_list_element_type(
                tariff_periods, cluster.Structs.TariffPeriodStruct, "TariffPeriods attribute must contain TariffPeriodStruct elements")
            for item in tariff_periods:
                await self.checkTariffPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)

    async def check_current_tariff_components_attribute(self, endpoint):

        current_tariff_components = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents)
        if current_tariff_components is not NullValue:
            matter_asserts.assert_list(
                current_tariff_components, "CurrentTariffComponents attribute must return a list with length less or equal 20", max_length=20)
            matter_asserts.assert_list_element_type(
                current_tariff_components, cluster.Structs.TariffComponentStruct, "CurrentTariffComponents attribute must contain TariffComponentStruct elements")
            for item in current_tariff_components:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

    async def check_next_tariff_components_attribute(self, endpoint):

        next_tariff_components = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextTariffComponents)
        if next_tariff_components is not NullValue:
            matter_asserts.assert_list(
                next_tariff_components, "NextTariffComponents attribute must return a list with length less or equal 20", max_length=20)
            matter_asserts.assert_list_element_type(
                next_tariff_components, cluster.Structs.TariffComponentStruct, "NextTariffComponents attribute must contain TariffComponentStruct elements")
            for item in next_tariff_components:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

    async def check_default_randomization_offset_attribute(self, endpoint):

        if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DefaultRandomizationOffset):
            default_randomization_offset = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationOffset)
            if default_randomization_offset is not NullValue:
                matter_asserts.assert_valid_int16(default_randomization_offset, 'DefaultRandomizationOffset must be of type int16')

    async def check_default_randomization_type_attribute(self, endpoint):

        if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DefaultRandomizationType):
            default_randomization_type = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationType)
            if default_randomization_type is not NullValue:
                matter_asserts.assert_valid_enum(
                    default_randomization_type, "DefaultRandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)
                asserts.assert_greater_equal(default_randomization_type, 0,
                                             "DefaultRandomizationType must be greater or equal than 0.")
                asserts.assert_less_equal(default_randomization_type, 4, "DefaultRandomizationType must be less or equal than 4.")
