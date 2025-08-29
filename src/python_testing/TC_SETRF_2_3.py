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
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


import logging
from typing import List

from mobly import asserts
from TC_SETRF_TestBase import CommodityTariffTestBaseHelper

import matter.clusters as Clusters
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class TC_SETRF_2_3(CommodityTariffTestBaseHelper):
    """Implementation of test case TC_SETRF_2_3."""

    def desc_TC_SETRF_2_3(self) -> str:
        """Returns a description of this test"""

        return "Primary Functionality with server as DUT. Check attributes updating with date changing"

    def pics_TC_SETRF_2_3(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S"]

    def steps_TC_SETRF_2_3(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", "DUT is commissioned.", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "TestEventTriggersEnabled is True"),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster for Fake Tariff Set Test Event",
                     "DUT replies with SUCCESS status code."),
        ]

        return steps

    @async_test_body
    async def test_TC_SETRF_2_3(self):
        """Implements test procedure for test case TC_SETRF_2_3."""

        endpoint = self.get_endpoint()

        # Variables that will be used in the test to store intermediate values
        currentDayEntryDayofWeekValue: int = None
        nextDayEntryDayofWeekValue: int = None
        dateCurrentValue: int = None
        dayTypeCurrentValue: cluster.Enums.DayTypeEnum = None
        dayEntryIDsCurrentValue: List[int] = []
        dateNextValue: int = None
        dayTypeNextValue: cluster.Enums.DayTypeEnum = None
        dayEntryIDsNextValue: List[int] = []
        tariffComponentIDsCurrentValue: List[int] = []
        tariffComponentIDsNextValue: List[int] = []
        tariffComponentsCurrentValue: List[cluster.Structs.TariffComponentStruct] = []
        tariffComponentsNextValue: List[cluster.Structs.TariffComponentStruct] = []
        dayPatternIDsCurrentValue: List[int] = []

        self.step("1")
        # Commissioning

        self.step("2")
        # TH reads TestEventTriggersEnabled attribute, expects a True.
        await self.check_test_event_triggers_enabled()

        self.step("3")
        # TH sends TestEventTrigger command to General Diagnostics Cluster for Attributes Value Set Test Event, expects a SUCCESS status code.
        await self.send_test_event_trigger_for_attributes_value_set()

        self.step("4")
        # TH reads CurrentDayEntryDate attribute, expects a epoch-s. Define day of week based on CurrentDayEntryDate attribute value.
        self.currentDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)
        await self.check_current_day_entry_date_attribute(endpoint, self.currentDayEntryDateValue)
        currentDayEntryDayofWeekValue = await self.get_day_of_week_from_current_day_entry_date(self.currentDayEntryDateValue)

        self.step("5")
        # TH reads CurrentDay attribute, expects a CurrentDayStruct
        self.currentDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)
        await self.check_current_day_attribute(endpoint, self.currentDayValue)
        # This split storing is necessary to correspond the Test Plan description
        dateCurrentValue = self.currentDayValue.date
        dayTypeCurrentValue = self.currentDayValue.dayType
        dayEntryIDsCurrentValue = self.currentDayValue.dayEntryIDs

        if dayTypeCurrentValue == 3:  # If CurrentDay has Event type.
            # we check CurrentDay against corresponding DayStruct from IndividualDays
            self.step("6")
            self.individualDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays)
            await self.check_individual_days_attribute(endpoint, self.individualDayValue)
            for day in self.individualDayValue:
                if day.date == dateCurrentValue:
                    asserts.assert_equal(day.dayType, dayTypeCurrentValue,
                                         "DayType from CurrentDay must be equal to DayType from IndividualDays.")
                    asserts.assert_equal(day.dayEntryIDs, dayEntryIDsCurrentValue,
                                         "DayEntryIDs from IndividualDays must be equal to DayEntryIDs from CurrentDay.")
            # Steps 7.1 and 7.2 can be skipped if CurrentDay is Event
            self.skip_step("7.1")
            self.skip_step("7.2")
        else:  # If CurrentDay is not Event
            self.skip_step("6")
            # we need to define active CalendarPeriod and get DayPatternIDs for this period
            self.step("7.1")
            self.calendarPeriodsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods)
            await self.check_calendar_periods_attribute(endpoint, self.calendarPeriodsValue)
            dayPatternIDsCurrentValue = await self.get_day_pattern_IDs_for_active_calendar_period(next=False)
            # if len(self.calendarPeriodsValue) == 1:
            #     dayPatternIDsCurrentValue = self.calendarPeriodsValue[0].dayPatternIDs
            # else:
            #     for period in self.calendarPeriodsValue[::-1]:
            #         if period.startDate < self.currentDayEntryDateValue:
            #             dayPatternIDsCurrentValue = period.dayPatternIDs
            #             break

            self.step("7.2")
            # we search DayPattern that corresponds to day of week defined on step 4 and compare DayEntryIDs from CurrentDay and found DayPattern
            self.dayPatternsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
            await self.check_day_patterns_attribute(endpoint, self.dayPatternsValue)
            await self.search_day_pattern_for_given_day_of_week_check_day_entry_IDs(list_of_day_pattern_IDs=dayPatternIDsCurrentValue,
                                                                                    day_of_week=currentDayEntryDayofWeekValue,
                                                                                    list_of_day_entry_IDs=dayEntryIDsCurrentValue,
                                                                                    current_or_next="Current")
            # for dayPattern in self.dayPatternsValue:
            #     if dayPattern.dayPatternID in dayPatternIDsCurrentValue:
            #         if (dayPattern.daysOfWeek & currentDayEntryDayofWeekValue) == currentDayEntryDayofWeekValue:
            #             asserts.assert_equal(sorted(dayPattern.dayEntryIDs), sorted(dayEntryIDsCurrentValue),
            #                                  "DayEntryIDs from DayPatterns must be equal to DayEntryIDs from CurrentDay.")
            #             day_pattern_found = True
            #             break

            # if not day_pattern_found:
            #     asserts.fail("DayPattern not found for CurrentDayEntryDate attribute value.")

        self.step("8")
        # TH reads CurrentTariffComponents attribute, expects a list of TariffComponentStruct. Gets TariffComponentIDs from CurrentTariffComponents.
        self.currentTariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents)
        await self.check_current_tariff_components_attribute(endpoint, self.currentTariffComponentsValue)
        for component in self.currentTariffComponentsValue:
            tariffComponentIDsCurrentValue.append(component.tariffComponentID)

        self.step("9")
        # TH reads TariffPeriods attribute, expects a list of TariffPeriodStruct. Define TariffPeriod corresponding to active TariffComponents.
        # Checks DayEntryIDs from defined TariffPeriod against DayEntryIDs from CurrentDay.
        self.tariffPeriodsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)
        await self.check_tariff_periods_attribute(endpoint, self.tariffPeriodsValue)
        for tariffPeriod in self.tariffPeriodsValue:
            if sorted(tariffPeriod.tariffComponentIDs) == sorted(tariffComponentIDsCurrentValue):
                asserts.assert_equal(sorted(tariffPeriod.dayEntryIDs), sorted(dayEntryIDsCurrentValue),
                                     "DayEntryIDs from TariffPeriods must be equal to DayEntryIDs from CurrentDay.")
                break

        self.step("10")
        # TH reads TariffComponents attribute, expects a list of TariffComponentStruct. Gets TariffComponents that correspond to TariffComponentIDs from CurrentTariffComponents.
        # Verifies that all TariffComponentStruct entries from CurrentTariffComponents are present in TariffComponents.
        self.tariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        await self.check_tariff_components_attribute(endpoint, self.tariffComponentsValue)
        for component in self.tariffComponentsValue:
            if component.tariffComponentID in tariffComponentIDsCurrentValue:
                tariffComponentsCurrentValue.append(component)

        asserts.assert_equal(sorted(tariffComponentsCurrentValue), sorted(self.currentTariffComponentsValue),
                             "All TariffComponentStruct entries from CurrentTariffComponents must be presented in TariffComponents.")

        self.step("11")
        self.nextDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        await self.check_next_day_entry_date_attribute(endpoint, self.nextDayEntryDateValue)
        nextDayEntryDayofWeekValue = await self.get_day_of_week_from_current_day_entry_date(self.nextDayEntryDateValue)

        self.step("12")
        # TH reads NextDay attribute, expects a NextDayStruct
        self.nextDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        await self.check_next_day_attribute(endpoint, self.nextDayValue)
        # This split storing is necessary to correspond the Test Plan description
        dateNextValue = self.nextDayValue.date
        dayTypeNextValue = self.nextDayValue.dayType
        dayEntryIDsNextValue = self.nextDayValue.dayEntryIDs
        asserts.assert_not_equal(self.currentDayValue, self.nextDayValue, "NextDay must not be equal to CurrentDay.")

        if dayTypeCurrentValue == 3:  # If NextDay has Event type.
            # we check NextDay against corresponding DayStruct from IndividualDays
            self.step("13")
            self.individualDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays)
            await self.check_individual_days_attribute(endpoint, self.individualDayValue)
            for day in self.individualDayValue:
                if day.date == dateNextValue:
                    asserts.assert_equal(day.dayType, dayTypeNextValue,
                                         "DayType from NextDay must be equal to DayType from IndividualDays.")
                    asserts.assert_equal(day.dayEntryIDs, dayEntryIDsNextValue,
                                         "DayEntryIDs from IndividualDays must be equal to DayEntryIDs from NextDay.")
            # Steps 14.1 and 14.2 can be skipped if NextDay is Event
            self.skip_step("14.1")
            self.skip_step("14.2")
        else:  # If NextDay is not Event
            self.skip_step("13")
            # we need to define active CalendarPeriod and get DayPatternIDs for this period
            self.step("14.1")
            self.calendarPeriodsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods)
            await self.check_calendar_periods_attribute(endpoint, self.calendarPeriodsValue)
            dayPatternIDsNextValue = await self.get_day_pattern_IDs_for_active_calendar_period(next=True)
            # if len(self.calendarPeriodsValue) == 1:
            #     dayPatternIDsNextValue = self.calendarPeriodsValue[0].dayPatternIDs
            # else:
            #     for period in self.calendarPeriodsValue[::-1]:
            #         if period.startDate < self.nextDayEntryDateValue:
            #             dayPatternIDsNextValue = period.dayPatternIDs
            #             break

            self.step("14.2")
            # we search DayPattern that corresponds to day of week defined on step 12 and compare DayEntryIDs from NextDay and found DayPattern
            self.dayPatternsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
            await self.check_day_patterns_attribute(endpoint, self.dayPatternsValue)
            await self.search_day_pattern_for_given_day_of_week_check_day_entry_IDs(
                dayPatternIDsNextValue, nextDayEntryDayofWeekValue, dayEntryIDsNextValue, "Next")
            # for dayPattern in self.dayPatternsValue:
            #     if dayPattern.dayPatternID in dayPatternIDsNextValue:
            #         if (dayPattern.daysOfWeek & nextDayEntryDayofWeekValue) == nextDayEntryDayofWeekValue:
            #             asserts.assert_equal(sorted(dayPattern.dayEntryIDs), sorted(dayEntryIDsNextValue),
            #                                  "DayEntryIDs from DayPatterns must be equal to DayEntryIDs from NextDay.")
            #             day_pattern_found = True
            #             break

            # if not day_pattern_found:
            #     asserts.fail("DayPattern not found for NextDayEntryDate attribute value.")

        self.step("15")
        # TH reads NextTariffComponents attribute, expects a list of TariffComponentStruct. Gets TariffComponentIDs from NextTariffComponents.
        self.nextTariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextTariffComponents)
        await self.check_next_tariff_components_attribute(endpoint, self.nextTariffComponentsValue)
        for component in self.nextTariffComponentsValue:
            tariffComponentIDsNextValue.append(component.tariffComponentID)

        self.step("16")
        # TH reads TariffPeriods attribute, expects a list of TariffPeriodStruct. Define TariffPeriod corresponding to active TariffComponents.
        # Checks DayEntryIDs from defined TariffPeriod against DayEntryIDs from NextDay.
        self.tariffPeriodsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)
        await self.check_tariff_periods_attribute(endpoint, self.tariffPeriodsValue)
        for tariffPeriod in self.tariffPeriodsValue:
            if sorted(tariffPeriod.tariffComponentIDs) == sorted(tariffComponentIDsNextValue):
                asserts.assert_equal(sorted(tariffPeriod.dayEntryIDs), sorted(dayEntryIDsNextValue),
                                     "DayEntryIDs from TariffPeriods must be equal to DayEntryIDs from NextDay.")
                break

        self.step("17")
        # TH reads TariffComponents attribute, expects a list of TariffComponentStruct. Gets TariffComponents that correspond to TariffComponentIDs from NextTariffComponents.
        # Verifies that all TariffComponentStruct entries from NextTariffComponents are present in TariffComponents.
        self.tariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        await self.check_tariff_components_attribute(endpoint, self.tariffComponentsValue)
        for component in self.tariffComponentsValue:
            if component.tariffComponentID in tariffComponentIDsNextValue:
                tariffComponentsNextValue.append(component)

        asserts.assert_equal(sorted(tariffComponentsNextValue), sorted(self.nextTariffComponentsValue),
                             "All TariffComponentStruct entries from NextTariffComponents must be presented in TariffComponents.")

        self.step("18")
        # TH sends TestEventTrigger for Change Time Test Event.
        await self.send_test_event_trigger_change_time()

        self.step("19")
        # TH reads CurrentDayEntryDate attribute, expects a epoch-s. Checks that CurrentDayEntryDate changed after TestEventTrigger.
        # Checks that CurrentDayEntryDate is equal to NextDayEntryDate (previously read) after TestEventTrigger.
        currentDayEntryDateValue_previous = self.currentDayEntryDateValue
        self.currentDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)
        await self.check_current_day_entry_date_attribute(endpoint, self.currentDayEntryDateValue)
        asserts.assert_not_equal(self.currentDayEntryDateValue, currentDayEntryDateValue_previous,
                                 "CurrentDayEntryDate must change after TestEventTrigger.")
        asserts.assert_equal(self.currentDayEntryDateValue, self.nextDayEntryDateValue,
                             "CurrentDayEntryDate must be equal to NextDayEntryDate after TestEventTrigger.")

        self.step("20")
        currentDayValue_previous = self.currentDayValue
        self.currentDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)
        await self.check_current_day_attribute(endpoint, self.currentDayValue)
        asserts.assert_equal(self.currentDayValue, currentDayValue_previous,
                             "CurrentDay must not change after TestEventTrigger for Change Time Test Event.")

        self.step("21")
        nextDayEntryDateValue_previous = self.nextDayEntryDateValue
        self.nextDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        await self.check_next_day_entry_date_attribute(endpoint, self.nextDayEntryDateValue)
        asserts.assert_not_equal(self.nextDayEntryDateValue, nextDayEntryDateValue_previous,
                                 "NextDayEntryDate must change after TestEventTrigger for Change Time Test Event.")

        self.step("22")
        nextDayValue_previous = self.nextDayValue
        self.nextDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        await self.check_next_day_attribute(endpoint, self.nextDayValue)
        asserts.assert_equal(self.nextDayValue, nextDayValue_previous,
                             "NextDay must not change after TestEventTrigger.")
        asserts.assert_not_equal(self.nextDayValue, self.currentDayValue,
                                 "NextDay must not be equal to CurrentDay after TestEventTrigger for Change Time Test Event.")

        self.step("23")
        await self.send_test_event_trigger_change_day()

        self.step("24")
        currentDayEntryDateValue_previous = self.currentDayEntryDateValue
        self.currentDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)
        await self.check_current_day_entry_date_attribute(endpoint, self.currentDayEntryDateValue)
        asserts.assert_not_equal(self.currentDayEntryDateValue, currentDayEntryDateValue_previous,
                                 "CurrentDayEntryDate must change after TestEventTrigger for Change Day Test Event.")
        asserts.assert_not_equal(self.currentDayEntryDateValue, self.nextDayEntryDateValue,
                                 "CurrentDayEntryDate must be not equal to NextDayEntryDate after TestEventTrigger for Change Day Test Event.")

        self.step("25")
        currentDayValue_previous = self.currentDayValue
        self.currentDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)
        await self.check_current_day_attribute(endpoint, self.currentDayValue)
        asserts.assert_not_equal(self.currentDayValue, currentDayValue_previous,
                                 "CurrentDay must be changed after TestEventTrigger for Change Day Test Event.")
        asserts.assert_equal(self.currentDayValue, self.nextDayValue,
                             "CurrentDay must be equal to NextDay after TestEventTrigger for Change Day Test Event.")

        self.step("26")
        nextDayEntryDateValue_previous = self.nextDayEntryDateValue
        self.nextDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        await self.check_next_day_entry_date_attribute(endpoint, self.nextDayEntryDateValue)
        asserts.assert_not_equal(self.nextDayEntryDateValue, nextDayEntryDateValue_previous,
                                 "NextDayEntryDate must change after TestEventTrigger for Change Day Test Event.")
        asserts.assert_not_equal(self.nextDayEntryDateValue, self.currentDayEntryDateValue,
                                 "NextDayEntryDate must not be equal to CurrentDayEntryDate after TestEventTrigger for Change Day Test Event.")

        self.step("27")
        nextDayValue_previous = self.nextDayValue
        self.nextDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        await self.check_next_day_attribute(endpoint, self.nextDayValue)
        asserts.assert_not_equal(self.nextDayValue, nextDayValue_previous,
                                 "NextDay must change after TestEventTrigger for Change Day Test Event.")
        asserts.assert_not_equal(self.nextDayValue, self.currentDayValue,
                                 "NextDay must not be equal to CurrentDay after TestEventTrigger for Change Day Test Event.")

        self.step("28")
        await self.send_test_event_trigger_clear()


if __name__ == "__main__":
    default_matter_test_main()
