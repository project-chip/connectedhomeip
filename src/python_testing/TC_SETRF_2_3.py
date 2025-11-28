#!/usr/bin/env python3
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
#     app: ${energy-gateway}
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

        return [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test).",
                     "DUT is commissioning to TH.", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.",
                     "Value has to be 1 (True)."),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SETRF.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SETRF.TEST_EVENT_TRIGGER for Test Event Attributes Value Set Test Event.",
                     "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep("4", "TH reads from the DUT the CurrentDayEntryDate attribute.", """
                     - Verify that the DUT response contains a value of epoch-s type;
                     - Store the value as currentDayEntryDateValue;
                     - Define day of week based on currentDayEntryDateValue epoch-s value. Store the defined value as currentDayEntryDayofWeekValue with DayPatternDayOfWeekBitmap type."""),
            TestStep("5", "TH reads from the DUT the CurrentDay attribute.", """
                     - Verify that the DUT response contains a value of DayStruct type;
                     - Store the value as currentDayValue;
                     - Store Date field value as dateCurrentValue;
                     - Store DayType field value as dayTypeCurrentValue;
                     - Store DayEntryIDs field value as dayEntryIDsCurrentValue"""),
            TestStep("6", "If dayTypeCurrentValue equals 3 (Event), TH reads from the DUT the IndividualDays attribute.", """
                     - Verify that the DUT response contains a value that is a list of DayStruct entries with list length less or equal 50;
                     - For each entry:
                        - If Date field value equals dateCurrentValue, verify that Date field value is unique in the entries list, DayType field value equals dayTypeCurrentValue and DayEntryIDs field value equals dayEntryIDsCurrentValue."""),
            TestStep("7a", "If dayTypeCurrentValue does NOT equal 3 (Event), TH reads from the DUT the CalendarPeriods attribute.", """
                     - Verify that the DUT response contains a value that is a list of CalendarPeriodStruct entries with list length in range 1 - 4;
                     - Check the list in reverse order and store DayPatternIDs field value as dayPatternIDsCurrentValue for the first entity that fits the condition:
                        - StartDate field value is less than currentDayEntryDateValue."""),
            TestStep("7b", "TH reads from the DUT the DayPatterns attribute.", """
                     - Verify that the DUT response contains a value that is a list of DayPatternStruct entries with list length less or equal 28;
                     - For each entry:
                        - If DayPatternID field value is included in dayPatternIDsCurrentValue and bitwise AND DaysOfWeek field value and currentDayEntryDayofWeekValue equals currentDayEntryDayofWeekValue, verify that DayEntryIDs field value equals dayEntryIDsCurrentValue."""),
            TestStep("8", "TH reads from the DUT the CurrentTariffComponents attribute.", """
                     - Verify that the DUT response contains a value that is a list of TariffComponentStruct entries with list length less or equal 20;
                     - Store the value as currentTariffComponentsValue;
                     - For each entry store TariffComponentID field value in tariffComponentIDsCurrentValue list."""),
            TestStep("9", "TH reads from the DUT the TariffComponents attribute.", """
                     - Verify that the DUT response contains a null or a list of TariffComponentStruct entries with list length in range 1-672;
                     - For each entry:
                        - If TariffComponentID equals any from tariffComponentIDsCurrentValue, save the entry value in tariffComponentsCurrentValue list;
                        - Verify that tariffComponentsCurrentValue matches currentTariffComponentsValue."""),
            TestStep("10", "TH reads from the DUT the NextDayEntryDate attribute.", """
                     - Verify that the DUT response contains a value of epoch-s type;
                     - Store the value as nextDayEntryDateValue."""),
            TestStep("11", "TH reads from the DUT the NextDay attribute.", """
                     - Verify that the DUT response contains a value of DayStruct type;
                     - Store the value as nextDayValue."""),
            TestStep("12", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SETRF.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SETRF.TEST_EVENT_TRIGGER for Change Time Test Event.", """
                     Verify DUT responds w/ status SUCCESS(0x00)."""),
            TestStep("13", "TH reads from the DUT the CurrentDayEntryDate attribute.", """
                     - Verify that the DUT response contains a value of epoch-s type;
                     - The value does NOT match the currentDayEntryDateValue;
                     - The value matches the nextDayEntryDateValue;
                     - Store the value as currentDayEntryDateValue."""),
            TestStep("14", "TH reads from the DUT the CurrentDay attribute.", """
                     - Verify that the DUT response contains a value of DayStruct type;
                     - The value matches the currentDayValue;
                     - Store the value as currentDayValue."""),
            TestStep("15", "TH reads from the DUT the NextDayEntryDate attribute.", """
                     - Verify that the DUT response contains a value of epoch-s type;
                     - The value does NOT match the nextDayEntryDateValue;
                     - The value does NOT match currentDayEntryDateValue;
                     - Store the value as nextDayEntryDateValue."""),
            TestStep("16", "TH reads from the DUT the NextDay attribute.", """
                     - Verify that the DUT response contains a value of DayStruct type;
                     - The value does NOT match currentDayValue and matches the nextDayValue;
                     - Store the value as nextDayValue."""),
            TestStep("17", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SETRF.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SETRF.TEST_EVENT_TRIGGER for Change Day Test Event.", """
                     Verify DUT responds w/ status SUCCESS(0x00)."""),
            TestStep("18", "TH reads from the DUT the CurrentDayEntryDate attribute.", """
                     - Verify that the DUT response contains a value of epoch-s type;
                     - The value does NOT match the currentDayEntryDateValue;
                     - The value does NOT match the nextDayEntryDateValue."""),
            TestStep("19", "TH reads from the DUT the CurrentDay attribute.", """
                     - Verify that the DUT response contains a value of DayStruct type;
                     - The value does NOT match the currentDayValue;
                     - The value matches nextDayValue;
                     - Store the value as currentDayValue."""),
            TestStep("20", "TH reads from the DUT the NextDayEntryDate attribute.", """
                     - Verify that the DUT response contains a value of epoch-s type;
                     - The value does NOT match currentDayEntryDateValue;
                     - The value does NOT match the nextDayEntryDateValue."""),
            TestStep("21", "TH reads from the DUT the NextDay attribute.", """
                     - Verify that the DUT response contains a value of DayStruct type;
                     - The value does NOT match the nextDayValue;
                     - The value does NOT match the currentDayValue."""),
            TestStep("22", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SETRF.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SETRF.TEST_EVENT_TRIGGER for Test Event Clear.", """
                     Verify DUT responds w/ status SUCCESS(0x00)."""),
        ]

    @async_test_body
    async def test_TC_SETRF_2_3(self):
        """Implements test procedure for test case TC_SETRF_2_3."""

        endpoint = self.get_endpoint()

        # Variables that will be used in the test to store intermediate values
        currentDayEntryDayofWeekValue: int = None
        dateCurrentValue: int = None
        dayTypeCurrentValue: cluster.Enums.DayTypeEnum = None
        dayEntryIDsCurrentValue: List[int] = []
        tariffComponentIDsCurrentValue: List[int] = []
        tariffComponentsCurrentValue: List[cluster.Structs.TariffComponentStruct] = []
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
        currentDayEntryDayofWeekValue = await self.get_day_of_week_from_day_entry_date(self.currentDayEntryDateValue)

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
            self.individualDaysValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays)
            await self.check_individual_days_attribute(endpoint, self.individualDaysValue)
            for day in self.individualDaysValue:
                if day.date == dateCurrentValue:
                    asserts.assert_equal(day.dayType, dayTypeCurrentValue,
                                         "DayType from CurrentDay must be equal to DayType from IndividualDays.")
                    asserts.assert_equal(day.dayEntryIDs, dayEntryIDsCurrentValue,
                                         "DayEntryIDs from IndividualDays must be equal to DayEntryIDs from CurrentDay.")
            # Steps 7a and 7b can be skipped if CurrentDay is Event
            self.skip_step("7a")
            self.skip_step("7b")
        else:  # If CurrentDay is not Event
            self.skip_step("6")
            # we need to define active CalendarPeriod and get DayPatternIDs for this period
            self.step("7a")
            self.calendarPeriodsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods)
            await self.check_calendar_periods_attribute(endpoint, self.calendarPeriodsValue)
            dayPatternIDsCurrentValue = await self.get_day_pattern_IDs_for_active_calendar_period(next=False)

            self.step("7b")
            # we search DayPattern that corresponds to day of week defined on step 4 and compare DayEntryIDs from CurrentDay and found DayPattern
            self.dayPatternsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
            await self.check_day_patterns_attribute(endpoint, self.dayPatternsValue)
            await self.search_day_pattern_for_given_day_of_week_check_day_entry_IDs(list_of_day_pattern_IDs=dayPatternIDsCurrentValue,
                                                                                    day_of_week=currentDayEntryDayofWeekValue,
                                                                                    list_of_day_entry_IDs=dayEntryIDsCurrentValue,
                                                                                    current_or_next="Current")

        self.step("8")
        # TH reads CurrentTariffComponents attribute, expects a list of TariffComponentStruct. Gets TariffComponentIDs from CurrentTariffComponents.
        self.currentTariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents)
        await self.check_current_tariff_components_attribute(endpoint, self.currentTariffComponentsValue)
        for component in self.currentTariffComponentsValue:
            tariffComponentIDsCurrentValue.append(component.tariffComponentID)

        self.step("9")
        # TH reads TariffComponents attribute, expects a list of TariffComponentStruct. Gets TariffComponents that correspond to TariffComponentIDs from CurrentTariffComponents.
        # Verifies that all TariffComponentStruct entries from CurrentTariffComponents are present in TariffComponents.
        self.tariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        await self.check_tariff_components_attribute(endpoint, self.tariffComponentsValue)
        for component in self.tariffComponentsValue:
            if component.tariffComponentID in tariffComponentIDsCurrentValue:
                tariffComponentsCurrentValue.append(component)

        asserts.assert_equal(sorted(tariffComponentsCurrentValue), sorted(self.currentTariffComponentsValue),
                             "All TariffComponentStruct entries from CurrentTariffComponents must be presented in TariffComponents.")

        self.step("10")
        # TH reads NextDayEntryDate attribute, expects a epoch-s
        self.nextDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        await self.check_next_day_entry_date_attribute(endpoint, self.nextDayEntryDateValue)

        self.step("11")
        # TH reads NextDay attribute, expects a NextDayStruct
        self.nextDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        await self.check_next_day_attribute(endpoint, self.nextDayValue)

        self.step("12")
        # TH sends TestEventTrigger for Change Time Test Event.
        await self.send_test_event_trigger_change_time()

        self.step("13")
        # TH reads CurrentDayEntryDate attribute, expects a epoch-s. Checks that CurrentDayEntryDate changed after TestEventTrigger Change Time Test Event.
        # Checks that CurrentDayEntryDate is equal to NextDayEntryDate (previously read) after TestEventTrigger Change Time Test Event.
        currentDayEntryDateValue_previous = self.currentDayEntryDateValue
        self.currentDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)
        await self.check_current_day_entry_date_attribute(endpoint, self.currentDayEntryDateValue)
        asserts.assert_not_equal(self.currentDayEntryDateValue, currentDayEntryDateValue_previous,
                                 "CurrentDayEntryDate must change after TestEventTrigger.")
        asserts.assert_equal(self.currentDayEntryDateValue, self.nextDayEntryDateValue,
                             "CurrentDayEntryDate must be equal to NextDayEntryDate after TestEventTrigger.")

        self.step("14")
        # TH reads CurrentDay attribute, expects a DayStruct. Checks that CurrentDay did not change after TestEventTrigger Change Time Test Event.
        currentDayValue_previous = self.currentDayValue
        self.currentDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)
        await self.check_current_day_attribute(endpoint, self.currentDayValue)
        asserts.assert_equal(self.currentDayValue, currentDayValue_previous,
                             "CurrentDay must not change after TestEventTrigger for Change Time Test Event.")

        self.step("15")
        # TH reads NextDayEntryDate attribute, expects a epoch-s. Checks that NextDayEntryDate changed after TestEventTrigger Change Time Test Event.
        nextDayEntryDateValue_previous = self.nextDayEntryDateValue
        self.nextDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        await self.check_next_day_entry_date_attribute(endpoint, self.nextDayEntryDateValue)
        asserts.assert_not_equal(self.nextDayEntryDateValue, nextDayEntryDateValue_previous,
                                 "NextDayEntryDate must change after TestEventTrigger for Change Time Test Event.")

        self.step("16")
        # TH reads NextDay attribute, expects a DayStruct. Checks that NextDay did not change after TestEventTrigger Change Time Test Event.
        nextDayValue_previous = self.nextDayValue
        self.nextDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        await self.check_next_day_attribute(endpoint, self.nextDayValue)
        asserts.assert_equal(self.nextDayValue, nextDayValue_previous,
                             "NextDay must not change after TestEventTrigger.")
        asserts.assert_not_equal(self.nextDayValue, self.currentDayValue,
                                 "NextDay must not be equal to CurrentDay after TestEventTrigger for Change Time Test Event.")

        self.step("17")
        # TH sends TestEventTrigger for Change Day Test Event.
        await self.send_test_event_trigger_change_day()

        self.step("18")
        # TH reads CurrentDayEntryDate attribute, expects a epoch-s. Checks that CurrentDayEntryDate changed after TestEventTrigger for Change Day Test Event.
        # Checks that CurrentDayEntryDate is not equal to NextDayEntryDate (previously read) after TestEventTrigger for Change Day Test Event.
        currentDayEntryDateValue_previous = self.currentDayEntryDateValue
        self.currentDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)
        await self.check_current_day_entry_date_attribute(endpoint, self.currentDayEntryDateValue)
        asserts.assert_not_equal(self.currentDayEntryDateValue, currentDayEntryDateValue_previous,
                                 "CurrentDayEntryDate must change after TestEventTrigger for Change Day Test Event.")
        asserts.assert_not_equal(self.currentDayEntryDateValue, self.nextDayEntryDateValue,
                                 "CurrentDayEntryDate must be not equal to NextDayEntryDate after TestEventTrigger for Change Day Test Event.")

        self.step("19")
        # TH reads CurrentDay attribute, expects a DayStruct. Checks that CurrentDay changed after TestEventTrigger for Change Day Test Event.
        # Checks that CurrentDay is equal to NextDay (previously read) after TestEventTrigger for Change Day Test Event.
        currentDayValue_previous = self.currentDayValue
        self.currentDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)
        await self.check_current_day_attribute(endpoint, self.currentDayValue)
        asserts.assert_not_equal(self.currentDayValue, currentDayValue_previous,
                                 "CurrentDay must be changed after TestEventTrigger for Change Day Test Event.")
        asserts.assert_equal(self.currentDayValue, self.nextDayValue,
                             "CurrentDay must be equal to NextDay after TestEventTrigger for Change Day Test Event.")

        self.step("20")
        # TH reads NextDayEntryDate attribute, expects a epoch-s. Checks that NextDayEntryDate changed after TestEventTrigger for Change Day Test Event.
        # Checks that NextDayEntryDate is not equal to CurrentDayEntryDate after TestEventTrigger for Change Day Test Event.
        nextDayEntryDateValue_previous = self.nextDayEntryDateValue
        self.nextDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        await self.check_next_day_entry_date_attribute(endpoint, self.nextDayEntryDateValue)
        asserts.assert_not_equal(self.nextDayEntryDateValue, nextDayEntryDateValue_previous,
                                 "NextDayEntryDate must change after TestEventTrigger for Change Day Test Event.")
        asserts.assert_not_equal(self.nextDayEntryDateValue, self.currentDayEntryDateValue,
                                 "NextDayEntryDate must not be equal to CurrentDayEntryDate after TestEventTrigger for Change Day Test Event.")

        self.step("21")
        # TH reads NextDay attribute, expects a DayStruct. Checks that NextDay changed after TestEventTrigger for Change Day Test Event.
        # Checks that NextDay is not equal to CurrentDay after TestEventTrigger for Change Day Test Event.
        nextDayValue_previous = self.nextDayValue
        self.nextDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        await self.check_next_day_attribute(endpoint, self.nextDayValue)
        asserts.assert_not_equal(self.nextDayValue, nextDayValue_previous,
                                 "NextDay must change after TestEventTrigger for Change Day Test Event.")
        asserts.assert_not_equal(self.nextDayValue, self.currentDayValue,
                                 "NextDay must not be equal to CurrentDay after TestEventTrigger for Change Day Test Event.")

        self.step("22")
        await self.send_test_event_trigger_clear()


if __name__ == "__main__":
    default_matter_test_main()
