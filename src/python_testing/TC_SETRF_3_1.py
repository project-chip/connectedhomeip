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

from mobly import asserts
from TC_SETRF_TestBase import CommodityTariffTestBaseHelper

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class TC_SETRF_3_1(CommodityTariffTestBaseHelper):
    """Implementation of test case TC_SETRF_3_1."""

    def desc_TC_SETRF_3_1(self) -> str:
        """Returns a description of this test"""

        return "Subscription Report Verification with Server as DUT"

    def pics_TC_SETRF_3_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S"]

    def steps_TC_SETRF_3_1(self) -> list[TestStep]:

        return [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test).",
                     "DUT is commissioned to TH.", is_commissioning=True),
            TestStep("2", "Set up a subscription to the Commodity Tariff cluster attributes: MinIntervalFloor: 0, MaxIntervalCeiling: 10",
                     "Subscription successfully established."),
            TestStep("3", "TH reads from the DUT the TariffInfo attribute.", """
                     - Verify that the DUT response contains a null or a value of TariffInformationStruct type;
                     - Store the value as tariffInfoValue."""),
            TestStep("4", "TH reads from the DUT the TariffUnit attribute.", """
                     - Verify that the DUT response contains a null or a value of TariffUnitEnum type;
                     - Store the value as tariffUnitValue."""),
            TestStep("5", "TH reads from the DUT the StartDate attribute.", """
                     - Verify that the DUT response contains a null or a value of epoch-s type;
                     - Store the value as startDateAttributeValue;"""),
            TestStep("6", "TH reads from the DUT the IndividualDays attribute.", """
                     - Verify that the DUT response contains a null or a value that is a list  of  DayStruct  entries  with  list length less or equal 50;
                     - Store the value as individualDaysValue."""),
            TestStep("7", "TH reads from the DUT the DayEntries attribute.", """
                     - Verify that the DUT response contains a null or a value that is a list  of  DayEntryStruct  entries  with list length less or equal 672;
                     - Store the value as dayEntriesValue."""),
            TestStep("8", "TH reads from the DUT the DayPatterns attribute.", """
                     - Verify that the DUT response contains a null or a value that is a list of DayPatternStruct entries with list length less or equal 28;
                     - Store the value as dayPatternsValue."""),
            TestStep("9", "TH reads from the DUT the CalendarPeriods attribute.", """
                     - Verify that the DUT response contains a null or a value that is a list of CalendarPeriodStruct entries with list length in range 1 - 4;
                     - Store the value as calendarPeriodsValue."""),
            TestStep("10", "TH reads from the DUT the CurrentDay attribute.", """
                          - Verify that the DUT response contains a null or a value of DayStruct type;
                          - Store the value as currentDayValue."""),
            TestStep("11", "TH reads from the DUT the NextDay attribute.", """
                     - Verify that the DUT response contains a null or a value of DayStruct type;
                     - Store the value as nextDayValue."""),
            TestStep("12", "TH reads from the DUT the CurrentDayEntry attribute.", """
                     - Verify that the DUT response contains a null or a value of DayEntryStruct type;
                     - Store the value as currentDayEntryValue."""),
            TestStep("13", "TH reads from the DUT the CurrentDayEntryDate attribute.", """
                     - Verify that the DUT response contains a null or a value of epoch-s type;
                     - Store the value as currentDayEntryDateValue."""),
            TestStep("14", "TH reads from the DUT the NextDayEntry attribute.", """
                     - Verify that the DUT response contains a null or a value of DayEntryStruct type;
                     - Store the value as nextDayEntryValue."""),
            TestStep("15", "TH reads from the DUT the NextDayEntryDate attribute.", """
                     - Verify that the DUT response contains a null or an epoch-s value greater than currentDayEntryDateValue;
                     - Store the value as nextDayEntryDateValue."""),
            TestStep("16", "TH reads from the DUT the TariffComponents attribute.", """
                     - Verify that the DUT response contains a null or a list of TariffComponentStruct entries with list length in range 1-672;
                     - Store the value as tariffComponentsValue."""),
            TestStep("17", "TH reads from the DUT the TariffPeriods attribute.", """
                     - Verify that the DUT response contains a null or a list of TariffPeriodStruct entries with list length in range 1-672;
                     - Store the value as tariffPeriodsValue."""),
            TestStep("18", "TH reads from the DUT the CurrentTariffComponents attribute.", """
                     - Verify that the DUT response contains a null or a value that is a list of TariffComponentStruct entries with list length less or equal 20;
                     - Store the value as currentTariffComponentsValue."""),
            TestStep("19", "TH reads from the DUT the NextTariffComponents attribute.", """
                     - Verify that the DUT response contains a null or a value that is a list of TariffComponentStruct entries with list length less or equal 20;
                     - Store the value as nextTariffComponentsValue."""),
            TestStep("20", "TH reads from the DUT the DefaultRandomizationType attribute.", """
                     - Verify that the DUT response contains a null or a value of DayEntryRandomizationTypeEnum. Value has to be between a range of 0 - 4.
                     - Store the value as defaultRandomizationTypeValue."""),
            TestStep("21", "TH reads from the DUT the DefaultRandomizationOffset attribute.", """
                     - If defaultRandomizationTypeValue is null, Verify that the DUT response contains a null;
                     - If defaultRandomizationTypeValue is 0x01 (Fixed), Verify that the DUT response contains an int16 value;
                     - If defaultRandomizationTypeValue is 0x04 (RandomNegative), Verify that the DUT response contains an int16 value less or equal 0;
                     - If defaultRandomizationTypeValue is 0x00 (None), 0x02 (Random) or 0x03 (RandomPositive), Verify that the DUT response contains an int16 value greater or equal 0.
                     - Store the value as DefaultRandomizationOffsetValue."""),
            TestStep("22", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.", "Value has to be 1 (True)."),
            TestStep("23", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SETRF.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SETRF.TEST_EVENT_TRIGGER for Attributes Value Set Test Event.",
                     "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep("24", "TH awaits a ReportDataMessage from DUT containing a TariffInfo attribute with 10s timeout.", """
                     - Verify that the DUT response contains a null or a value of TariffInformationStruct type;
                        - Verify that TariffLabel field has type string with max length 128 or null;
                        - Verify that ProviderName field has type string with max length 128 or null;
                        - Verify that BlockMode field has type BlockModeEnum. Value has to be between a range of 0-2 or null;
                            - Store the value as BlockModeValue;
                     - Currency field:
                        - Verify that it does NOT contain Currency field if SETRF.S.F00(PRICE) is False;
                        - Verify that it contains Currency field if SETRF.S.F00(PRICE) is True;
                        - Verify that it has type currency or null;
                            - Verify that Currency field of Currency struct has type uint16 with value less or equal 999;
                            - Verify that DecimalPoints field of Currency struct has type uint8;
                    - The value does NOT match the tariffInfoValue;
                    - Store the value as tariffInfoValue."""),
            TestStep("25", "TH awaits a ReportDataMessage from DUT containing a TariffUnit attribute with 10s timeout.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value of TariffUnitEnum type;
                     - Value has to be between a range of 0 - 1;
                     - The value does NOT match the tariffUnitValue."""),
            TestStep("26", "TH reads from the DUT the StartDate attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value of epoch-s type;
                     - The value does NOT match the startDateAttributeValue;
                     - Store the value as startDateAttributeValue."""),
            TestStep("27", "TH reads from the DUT the IndividualDays attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value that is a list of DayStruct entries with list length less or equal 50;
                     - For each entry:
                        - Date field:
                                - Verify that Date field has epoch-s type;
                                - Verify that the DayStruct in this list are arranged in increasing order by the value of Date field;
                        - Verify that DayType field has DayTypeEnum type. Value has to be between a range of 0 - 3;
                        - Verify that DayEntryIDs of DayStruct field is a list of unique uint32 with list length in range 1 - 96;
                        - If DayType equals 3 (Event), store DayEntryIDs value in dayEntryIDsEvents list;
                    - The value does NOT match the individualDaysValue."""),
            TestStep("28", "TH reads from the DUT the DayEntries attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value that is a list of DayEntryStruct entries with list length less or equal 672;
                     - The value does NOT match the dayEntriesValue;
                     - For each entry
                        - Verify that DayEntryID has a unique uint32 value;
                        - Verify that StartTime field has uint16 type with value less or equal 1499;
                            - Store the field value as startTimeFieldValue;
                        - If DayEntryID is a value from dayEntryIDsEvents list, check that Duration field has a uint16 value less or equal 1500  - startTimeFieldValue;
                        - RandomizationType field:
                            - Verify that entry does NOT contain RandomizationType field if SETRF.S.F05(RNDM) is False;
                            - If SETRF.S.F05(RNDM) is True and RandomizationType field is presented it has DayEntryRandomizationType type. Value has to be between a range of 0 - 4;
                            - Store the field value randomizationTypeValue;
                     - RandomizationType field:
                        - Verify that entry does NOT contain RandomizationType field if SETRF.S.F05(RNDM) is False;
                        - If SETRF.S.F05(RNDM) is True and RandomizationType field is presented it has DayEntryRandomizationType type. Value has to be between a range of 0 - 4;
                        - Store the field value randomizationTypeValue;
                     - RandomizationOffset field:
                        - Verify that entry does NOT contain RandomizationOffset field if SETRF.S.F05(RNDM) is False;
                        - If SETRF.S.F05(RNDM) is True and randomizationTypeValue is 0x01 (Fixed) and RandomizationOffset field is presented, it has int16 value;
                        - If SETRF.S.F05(RNDM) is True and randomizationTypeValue is 0x04 (RandomNegative) and RandomizationOffset field is presented, it has an int16 value less or equal 0;
                        - If SETRF.S.F05(RNDM) is True and randomizationTypeValue is 0x00 (None), 0x02 (Random) or 0x03 (RandomPositive) and RandomizationOffset field is presented, it has an int16 value greater or equal 0."""),
            TestStep("29", "TH reads from the DUT the DayPatterns attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value that is a list of DayPatternStruct entries with list length less or equal 28;
                     - For each entry:
                        - Verify that DayPatternID field has uint32 type;
                        - Verify that DaysOfWeek field has DayPatternDayOfWeekBitmap type;
                        - Verify that DayEntryIDs field is a list of unique uint32 with list length in range 1 - 96;
                        - The value does NOT match the dayPatternsValue."""),
            TestStep("30", "TH reads from the DUT the CalendarPeriods attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value that is a list of CalendarPeriodStruct entries with list length in range 1 - 4;
                     - For each entry:
                        - StartDate field:
                            - Verify that StartDate field is null or has type epoch-s with value greater or equal startDateAttributeValue;
                            - Verify that the calendar period items in this list are arranged in increasing order by the value of StartDate field;
                            - Verify that if the startDateAttributeValue is null then the StartDate field is null on the first CalendarPeriodStruct item and is not null on any subsequent CalendarPeriodStruct items;
                        - Verify that DayPatternIDs field is a list of uint32 with list length in range 1 - 7;
                    - The value does NOT match the calendarPeriodsValue."""),
            TestStep("31", "TH reads from the DUT the CurrentDay attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value of DayStruct type;
                        - Verify that Date field of DayStruct has epoch-s type;
                        - Verify that DayType field of DayStruct has DayTypeEnum type. Value has to be between a range of 0 - 3;
                        - Verify that DayEntryIDs field is a list of unique uint32 with list length in range 1 - 96;
                     - The value does NOT match the currentDayValue."""),
            TestStep("32", "TH reads from the DUT the NextDay attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value of DayStruct type;
                        - Verify that DayType field of DayStruct has DayTypeEnum type. Value has to be between a range of 0 - 3;
                        - Verify that DayEntryIDs field is a list of unique uint32 with list length in range 1 - 96;
                     - The value does NOT match the nextDayValue."""),
            TestStep("33", "TH reads from the DUT the CurrentDayEntry attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value of DayEntryStruct type;
                     - Verify that the DUT response contains a null if tariffInfoValue equals null;
                     - The value does NOT match the currentDayEntryValue.
                     - Verify that DayEntryID field has uint32 type;
                     - Verify that StartTime field has uint16 type with value less or equal 1499;
                        - Store the field value as startTimeFieldValue;
                     - RandomizationType field:
                        - Verify that entry does NOT contain RandomizationType field if SETRF.S.F05(RNDM) is False;
                        - If SETRF.S.F05(RNDM) is True and RandomizationType field is presented it has DayEntryRandomizationType type. Value has to be between a range of 0 - 4;
                        - Store the field value randomizationTypeValue;
                     - RandomizationOffset field:
                        - Verify that entry does NOT contain RandomizationOffset field if SETRF.S.F05(RNDM) is False;
                        - If SETRF.S.F05(RNDM) is True and randomizationTypeValue is 0x01 (Fixed) and RandomizationOffset field is presented, it has an int16 value;
                        - If SETRF.S.F05(RNDM) is True and randomizationTypeValue is 0x04 (RandomNegative) and RandomizationOffset field is presented, it has an int16 value less or equal 0;
                        - If SETRF.S.F05(RNDM) is True and randomizationTypeValue is 0x00 (None), 0x02 (Random) or 0x03 (RandomPositive) and RandomizationOffset field is presented, it has an int16 value greater or equal 0."""),
            TestStep("34", "TH reads from the DUT the CurrentDayEntryDate attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value of epoch-s type;
                     - The value does NOT match the currentDayEntryDateValue;
                     - Store the value as currentDayEntryDateValue."""),
            TestStep("35", "TH reads from the DUT the NextDayEntry attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value of DayEntryStruct type;
                     - The value does NOT match the nextDayEntryValue;
                     - Verify that DayEntryID field has uint32 type;
                     - Verify that StartTime field has uint16 type with value less or equal 1499;
                        - Store the field value as startTimeFieldValue;
                     - RandomizationType field:
                        - Verify that entry does NOT contain RandomizationType field if SETRF.S.F05(RNDM) is False;
                        - If SETRF.S.F05(RNDM) is True and RandomizationType field is presented it has DayEntryRandomizationType type. Value has to be between a range of 0 - 4;
                        - Store the field value randomizationTypeValue;
                     - RandomizationOffset field:
                        - Verify that entry does NOT contain RandomizationOffset field if SETRF.S.F05(RNDM) is False;
                        - If SETRF.S.F05(RNDM) is True and randomizationTypeValue is 0x01 (Fixed) and RandomizationOffset field is presented, it has an int16 value;
                        - If SETRF.S.F05(RNDM) is True and randomizationTypeValue is 0x04 (RandomNegative) and RandomizationOffset field is presented, it has an int16 value less or equal 0;
                        - If SETRF.S.F05(RNDM) is True and randomizationTypeValue is 0x00 (None), 0x02 (Random) or 0x03 (RandomPositive) and RandomizationOffset field is presented, it has an int16 value greater or equal 0."""),
            TestStep("36", "TH reads from the DUT the NextDayEntryDate attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or an epoch-s value greater than currentDayEntryDateValue;
                     - The value does NOT match the nextDayEntryDateValue;"""),
            TestStep("37", "TH reads from the DUT the TariffComponents attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a list of TariffComponentStruct entries with list length in range 1-672;
                     - The value does NOT match the tariffComponentsValue;
                     - For each entry:
                        - Verify that TariffComponentID field has uint32 type;
                        - Threshold field:
                            - Verify that Threshold is null if BlockModeValue is 0 (NoBlock);
                            - Verify that Threshold has type int64 or null if BlockModeValue is 1 (Combined) or 2 (Individual);
                        - Verify that if Label field is presented it is null or has type string with max length 128;
                        - Predicted field:
                            - Verify that if Predicted field is presented it has type bool;
                        - Verify that it has at least one of the fields: Price, FriendlyCredit, AuxiliaryLoad, PeakPeriod, PowerThreshold;
                        - Price field:
                            - Verify that the entry does NOT contain Price field if SETRF.S.F00(PRICE) is False;
                            - If SETRF.S.F00(PRICE) is True and if Price field is presented it is null or has type TariffPriceStruct;
                                - Verify that PriceType field has TariffPriceTypeEnum type. Value has to be between a range of 0 - 4;
                                - Verify that if Price field is presented it has money type;
                                - Verify that if PriceLevel field is presented it has int16 type;
                        - FriendlyCredit field:
                            - Verify that entry does NOT contain FriendlyCredit field if SETRF.S.F01(FCRED) is False;
                            - If SETRF.S.F01(FCRED) is True and FriendlyCredit field is presented it has type bool;
                        - AuxiliaryLoad field:
                            - Verify that entry does NOT contain AuxiliaryLoad field if SETRF.S.F02(AUXLD) is False;
                            - If SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented it has AuxiliaryLoadSwitchSettingsStruct type;
                                - Verify that Number field has uint8 type;
                                - Verify that RequiredState field has AuxiliaryLoadSettingEnum type. Value has to be between a range of 0 - 2;
                        - PeakPeriod field:
                            - Verify that entry does NOT contain PeakPeriod field if SETRF.S.F03(PEAKP) is False;
                            - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented it has PeakPeriodStruct type;
                                - Verify that Severity field has PeakPeriodSeverityEnum type. Value has to be between a range of 0 - 3;
                                - Verify that PeakPeriod field has uint16 type and value greater or equal 1;
                        - PowerThreshold field:
                            - Verify that entry does NOT contain PowerThreshold field if SETRF.S.F04(PWRTHLD) is False;
                            - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented it has PowerThresholdStruct type;
                                - Verify that PowerThreshold field has power-mW type;
                                - Verify that ApparentPowerThreshold field has power-mVA type;
                                - Verify that PowerThresholdSource field has PowerThresholdSourceEnum type. Value has to be between a range of 0 - 2."""),
            TestStep("38", "TH reads from the DUT the TariffPeriods attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a list of TariffPeriodStruct entries with list length in range 1-672;
                     - For each entry:
                        - Verify that Label field is null or has type string with max length 128;
                        - Verify that DayEntryIDs field is a list of unique uint32 with list length in range 1 - 20;
                        - Verify that TariffComponentIDs field is a list of uint32 with list length in range 1 - 20;
                        - The value does NOT match the tariffPeriodsValue."""),
            TestStep("39", "TH reads from the DUT the CurrentTariffComponents attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value that is a list of TariffComponentStruct entries with list length less or equal 20;
                     - The value does NOT match the currentTariffComponentsValue;
                     - For each entry:
                        - Verify that TariffComponentID field has uint32 type;
                        - Threshold field:
                            - Verify that Threshold is null if BlockModeValue is 0 (NoBlock);
                            - Verify that Threshold has type int64 or null if BlockModeValue is 1 (Combined) or 2 (Individual);
                        - Verify that if Label field is presented it is null or has type string with max length 128;
                        - Predicted field:
                            - Verify that if Predicted field is presented it has type bool;
                        - Verify that it has at least one of the fields: Price, FriendlyCredit, AuxiliaryLoad, PeakPeriod, PowerThreshold;
                        - Price field:
                            - Verify that the entry does NOT contain Price field if SETRF.S.F00(PRICE) is False;
                            - If SETRF.S.F00(PRICE) is True and if Price field is presented it is null or has type TariffPriceStruct;
                                - Verify that PriceType field has TariffPriceTypeEnum type. Value has to be between a range of 0 - 4;
                                - Verify that if Price field is presented it has money type;
                                - Verify that if PriceLevel field is presented it has int16 type;
                        - FriendlyCredit field:
                            - Verify that entry does NOT contain FriendlyCredit field if SETRF.S.F01(FCRED) is False;
                            - If SETRF.S.F01(FCRED) is True and FriendlyCredit field is presented it has type bool;
                        - AuxiliaryLoad field:
                            - Verify that entry does NOT contain AuxiliaryLoad field if SETRF.S.F02(AUXLD) is False;
                            - If SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented it has AuxiliaryLoadSwitchSettingsStruct type;
                                - Verify that Number field has uint8 type;
                                - Verify that RequiredState field has AuxiliaryLoadSettingEnum type. Value has to be between a range of 0 - 2;
                        - PeakPeriod field:
                            - Verify that entry does NOT contain PeakPeriod field if SETRF.S.F03(PEAKP) is False;
                            - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented it has PeakPeriodStruct type;
                                - Verify that Severity field has PeakPeriodSeverityEnum type. Value has to be between a range of 0 - 3;
                                - Verify that PeakPeriod field has uint16 type and value greater or equal 1;
                        - PowerThreshold field:
                            - Verify that entry does NOT contain PowerThreshold field if SETRF.S.F04(PWRTHLD) is False;
                            - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented it has PowerThresholdStruct type;
                                - Verify that PowerThreshold field has power-mW type;
                                - Verify that ApparentPowerThreshold field has power-mVA type;
                                - Verify that PowerThresholdSource field has PowerThresholdSourceEnum type. Value has to be between a range of 0 - 2."""),
            TestStep("40", "TH reads from the DUT the NextTariffComponents attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value that is a list of TariffComponentStruct entries with list length less or equal 20;
                     - The value does NOT match the nextTariffComponentsValue;
                     - For each entry:
                        - Verify that TariffComponentID field has uint32 type;
                        - Threshold field:
                            - Verify that Threshold is null if BlockModeValue is 0 (NoBlock);
                            - Verify that Threshold has type int64 or null if BlockModeValue is 1 (Combined) or 2 (Individual);
                        - Verify that if Label field is presented it is null or has type string with max length 128;
                        - Predicted field:
                            - Verify that if Predicted field is presented it has type bool;
                        - Verify that it has at least one of the fields: Price, FriendlyCredit, AuxiliaryLoad, PeakPeriod, PowerThreshold;
                        - Price field:
                            - Verify that the entry does NOT contain Price field if SETRF.S.F00(PRICE) is False;
                            - If SETRF.S.F00(PRICE) is True and if Price field is presented it is null or has type TariffPriceStruct;
                                - Verify that PriceType field has TariffPriceTypeEnum type. Value has to be between a range of 0 - 4;
                                - Verify that if Price field is presented it has money type;
                                - Verify that if PriceLevel field is presented it has int16 type;
                        - FriendlyCredit field:
                            - Verify that entry does NOT contain FriendlyCredit field if SETRF.S.F01(FCRED) is False;
                            - If SETRF.S.F01(FCRED) is True and FriendlyCredit field is presented it has type bool;
                        - AuxiliaryLoad field:
                            - Verify that entry does NOT contain AuxiliaryLoad field if SETRF.S.F02(AUXLD) is False;
                            - If SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented it has AuxiliaryLoadSwitchSettingsStruct type;
                                - Verify that Number field has uint8 type;
                                - Verify that RequiredState field has AuxiliaryLoadSettingEnum type. Value has to be between a range of 0 - 2;
                        - PeakPeriod field:
                            - Verify that entry does NOT contain PeakPeriod field if SETRF.S.F03(PEAKP) is False;
                            - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented it has PeakPeriodStruct type;
                                - Verify that Severity field has PeakPeriodSeverityEnum type. Value has to be between a range of 0 - 3;
                                - Verify that PeakPeriod field has uint16 type and value greater or equal 1;
                        - PowerThreshold field:
                            - Verify that entry does NOT contain PowerThreshold field if SETRF.S.F04(PWRTHLD) is False;
                            - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented it has PowerThresholdStruct type;
                            - Verify that PowerThreshold field has power-mW type;
                            - Verify that ApparentPowerThreshold field has power-mVA type;
                            - Verify that PowerThresholdSource field has PowerThresholdSourceEnum type. Value has to be between a range of 0 - 2."""),
            TestStep("41", "TH reads from the DUT the DefaultRandomizationType attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If tariffInfoValue does NOT equal null, Verify that the DUT response contains a null or a value of DayEntryRandomizationTypeEnum. Value has to be between a range of 0 - 4;
                     - The value does NOT match the defaultRandomizationTypeValue;
                     - Store the value as defaultRandomizationTypeValue."""),
            TestStep("42", "TH reads from the DUT the DefaultRandomizationOffset attribute.", """
                     - If tariffInfoValue equals null, Verify that the DUT response contains a null;
                     - If defaultRandomizationTypeValue is null, Verify that the DUT response contains a null;
                     - Verify that the DUT response contains a null if tariffInfoValue equals null;
                     - If defaultRandomizationTypeValue is 0x01 (Fixed), Verify that the DUT response contains an int16 value;
                     - If defaultRandomizationTypeValue is 0x04 (RandomNegative), Verify that the DUT response contains an int16 value less or equal 0;
                     - If defaultRandomizationTypeValue is 0x00 (None), 0x02 (Random) or 0x03 (RandomPositive), Verify that the DUT response contains an int16 value greater or equal 0;
                     - The value does NOT match the DefaultRandomizationOffsetValue."""),
            TestStep("43", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SETRF.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SETRF.TEST_EVENT_TRIGGER for Test Event Clear.", """
                     Verify  DUT  responds  w/  status SUCCESS(0x00)."""),
            TestStep("44", "TH removes the subscription the Commodity Tariff cluster.", """
                     Subscription successfully removed."""),
        ]

    @async_test_body
    async def test_TC_SETRF_3_1(self):
        """Implementation of test case TC_SETRF_3_1."""

        endpoint = self.get_endpoint()
        matcher_list = self.get_mandatory_matchers()

        self.step("1")
        # Commissioning

        self.step("2")
        # TH establishes a subscription to Commodity Tariff cluster attributes
        subscription_handler = AttributeSubscriptionHandler(cluster)
        await subscription_handler.start(self.default_controller, self.dut_node_id,
                                         endpoint,
                                         min_interval_sec=0,
                                         max_interval_sec=10, keepSubscriptions=True)

        self.step("3")
        # TH reads TariffInfo attribute, expects a TariffInformationStruct
        TariffInfoValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                         attribute=cluster.Attributes.TariffInfo)
        await self.check_tariff_info_attribute(endpoint, TariffInfoValue)

        self.step("4")
        # TH reads TariffUnit attribute, expects a TariffUnitEnum
        TariffUnitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                         attribute=cluster.Attributes.TariffUnit)
        await self.check_tariff_unit_attribute(endpoint, TariffUnitValue)

        self.step("5")
        # TH reads StartDate attribute, expects a epoch-s
        StartDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                        attribute=cluster.Attributes.StartDate)
        await self.check_start_date_attribute(endpoint, StartDateValue)

        self.step("6")
        # TH reads IndividualDays attribute, expects a list of DayStruct
        IndividualDaysValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                             attribute=cluster.Attributes.IndividualDays)
        await self.check_individual_days_attribute(endpoint, IndividualDaysValue)

        self.step("7")
        # TH reads DayEntries attribute, expects a list of DayEntryStruct
        DayEntriesValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                         attribute=cluster.Attributes.DayEntries)
        await self.check_day_entries_attribute(endpoint, DayEntriesValue)

        self.step("8")
        # TH reads DayPatterns attribute, expects a list of DayPatternStruct
        DayPatternsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                          attribute=cluster.Attributes.DayPatterns)
        await self.check_day_patterns_attribute(endpoint, DayPatternsValue)

        self.step("9")
        # TH reads CalendarPeriods attribute, expects a list of CalendarPeriodStruct sorted by StartDate field in increasing order
        CalendarPeriodsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                              attribute=cluster.Attributes.CalendarPeriods)
        await self.check_calendar_periods_attribute(endpoint, CalendarPeriodsValue)

        self.step("10")
        # TH reads CurrentDay attribute, expects a DayStruct
        CurrentDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                         attribute=cluster.Attributes.CurrentDay)
        await self.check_current_day_attribute(endpoint, CurrentDayValue)

        self.step("11")
        # TH reads NextDay attribute, expects a DayStruct
        NextDayValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                      attribute=cluster.Attributes.NextDay)
        await self.check_next_day_attribute(endpoint, NextDayValue)

        self.step("12")
        # TH reads CurrentDayEntry attribute, expects a DayEntryStruct
        CurrentDayEntryValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                              attribute=cluster.Attributes.CurrentDayEntry)
        await self.check_current_day_entry_attribute(endpoint, CurrentDayEntryValue)

        self.step("13")
        # TH reads CurrentDayEntryDate attribute, expects a uint32
        CurrentDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                                  attribute=cluster.Attributes.CurrentDayEntryDate)
        await self.check_current_day_entry_date_attribute(endpoint, CurrentDayEntryDateValue)

        self.step("14")
        # TH reads NextDayEntry attribute, expects a DayEntryStruct
        NextDayEntryValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                           attribute=cluster.Attributes.NextDayEntry)
        await self.check_next_day_entry_attribute(endpoint, NextDayEntryValue)

        self.step("15")
        # TH reads NextDayEntryDate attribute, expects a uint32
        NextDayEntryDateValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                               attribute=cluster.Attributes.NextDayEntryDate)
        await self.check_next_day_entry_date_attribute(endpoint, NextDayEntryDateValue)

        self.step("16")
        # TH reads TariffComponents attribute, expects a list of TariffComponentStruct
        TariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                               attribute=cluster.Attributes.TariffComponents)
        await self.check_tariff_components_attribute(endpoint, TariffComponentsValue)

        self.step("17")
        # TH reads TariffPeriods attribute, expects a list of TariffPeriodStruct
        TariffPeriodsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                            attribute=cluster.Attributes.TariffPeriods)
        await self.check_tariff_periods_attribute(endpoint, TariffPeriodsValue)

        self.step("18")
        # TH reads CurrentTariffComponents attribute, expects a list of TariffComponentStruct
        CurrentTariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                                      attribute=cluster.Attributes.CurrentTariffComponents)
        await self.check_current_tariff_components_attribute(endpoint, CurrentTariffComponentsValue)

        self.step("19")
        # TH reads NextTariffComponents attribute, expects a list of TariffComponentStruct
        NextTariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                                   attribute=cluster.Attributes.NextTariffComponents)
        await self.check_next_tariff_components_attribute(endpoint, NextTariffComponentsValue)

        if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DefaultRandomizationType):

            self.step("20")

            if not self.check_pics("SETRF.S.A0012"):  # for cases when it is supported by DUT, but disabled in PICS
                log.warning("DefaultRandomizationType attribute is actually supported by DUT, but PICS SETRF.S.A0012 is False")

            # TH reads DefaultRandomizationType attribute, expects a DayEntryRandomizationTypeEnum
            DefaultRandomizationTypeValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                                           attribute=cluster.Attributes.DefaultRandomizationType)
            await self.check_default_randomization_type_attribute(endpoint, DefaultRandomizationTypeValue)
            matcher_list.append(self._default_randomization_type_matcher())
        else:

            if self.check_pics("SETRF.S.A0012"):  # for cases when it is not supported by DUT, but enabled in PICS
                self.step("20")
                asserts.fail(
                    "PICS file does not correspond to real DUT functionality. DefaultRandomizationType is not actually supported, but SETRF.S.A0012 is True.")
            else:  # attribute is not supported at all
                self.skip_step("20")

        if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DefaultRandomizationOffset):

            self.step("21")

            if not self.check_pics("SETRF.S.A0011"):  # for cases when it is supported by DUT, but disabled in PICS
                log.warning("DefaultRandomizationOffset attribute is actually supported by DUT, but PICS SETRF.S.A0011 is False")

            # TH reads DefaultRandomizationOffset attribute, expects a int16
            DefaultRandomizationOffsetValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                                                             attribute=cluster.Attributes.DefaultRandomizationOffset)
            await self.check_default_randomization_offset_attribute(endpoint, DefaultRandomizationOffsetValue)
            matcher_list.append(self._default_randomization_offset_matcher())
        else:

            if self.check_pics("SETRF.S.A0011"):  # for cases when it is not supported by DUT, but enabled in PICS
                self.step("21")
                asserts.fail(
                    "PICS file does not correspond to real DUT functionality. DefaultRandomizationOffset is not actually supported, but SETRF.S.A0011 is True.")
            else:  # attribute is not supported at all
                self.skip_step("21")

        self.step("22")
        # TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.
        await self.check_test_event_triggers_enabled()

        self.step("23")
        # TH sends TestEventTrigger command for Attributes Value Set Test Event
        await self.send_test_event_trigger_for_attributes_value_set()
        subscription_handler.await_all_expected_report_matches(matcher_list, timeout_sec=10)

        self.step("24")
        await self.check_tariff_info_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.TariffInfo][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.TariffInfo, "TariffInfo", TariffInfoValue)

        self.step("25")
        await self.check_tariff_unit_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.TariffUnit][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.TariffUnit, "TariffUnit", TariffUnitValue)

        self.step("26")
        await self.check_start_date_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.StartDate][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.StartDate, "StartDate", StartDateValue)

        self.step("27")
        await self.check_individual_days_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.IndividualDays][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports,
                                    cluster.Attributes.IndividualDays, "IndividualDays", IndividualDaysValue)

        self.step("28")
        await self.check_day_entries_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.DayEntries][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.DayEntries, "DayEntries", DayEntriesValue)

        self.step("29")
        await self.check_day_patterns_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.DayPatterns][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports,
                                    cluster.Attributes.DayPatterns, "DayPatterns", DayPatternsValue)

        self.step("30")
        await self.check_calendar_periods_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.CalendarPeriods][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports,
                                    cluster.Attributes.CalendarPeriods, "CalendarPeriods", CalendarPeriodsValue)

        self.step("31")
        await self.check_current_day_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.CurrentDay][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.CurrentDay, "CurrentDay", CurrentDayValue)

        self.step("32")
        await self.check_next_day_attribute(endpoint, subscription_handler.attribute_reports[cluster.Attributes.NextDay][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.NextDay, "NextDay", NextDayValue)

        self.step("33")
        await self.check_current_day_entry_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.CurrentDayEntry][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports,
                                    cluster.Attributes.CurrentDayEntry, "CurrentDayEntry", CurrentDayEntryValue)

        self.step("34")
        await self.check_current_day_entry_date_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.CurrentDayEntryDate][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.CurrentDayEntryDate,
                                    "CurrentDayEntryDate", CurrentDayEntryDateValue)

        self.step("35")
        await self.check_next_day_entry_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.NextDayEntry][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports,
                                    cluster.Attributes.NextDayEntry, "NextDayEntry", NextDayEntryValue)

        self.step("36")
        await self.check_next_day_entry_date_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.NextDayEntryDate][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports,
                                    cluster.Attributes.NextDayEntryDate, "NextDayEntryDate", NextDayEntryDateValue)

        self.step("37")
        await self.check_tariff_components_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.TariffComponents][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports,
                                    cluster.Attributes.TariffComponents, "TariffComponents", TariffComponentsValue)

        self.step("38")
        await self.check_tariff_periods_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.TariffPeriods][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports,
                                    cluster.Attributes.TariffPeriods, "TariffPeriods", TariffPeriodsValue)

        self.step("39")
        await self.check_current_tariff_components_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.CurrentTariffComponents][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.CurrentTariffComponents,
                                    "CurrentTariffComponents", CurrentTariffComponentsValue)

        self.step("40")
        await self.check_next_tariff_components_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.NextTariffComponents][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.NextTariffComponents,
                                    "NextTariffComponents", NextTariffComponentsValue)

        if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DefaultRandomizationType):

            self.step("41")

            if not self.check_pics("SETRF.S.A0012"):  # for cases when it is supported by DUT, but disabled in PICS
                log.warning("DefaultRandomizationType attribute is actually supported by DUT, but PICS SETRF.S.A0012 is False")

            # TH reads DefaultRandomizationType attribute, expects a DayEntryRandomizationTypeEnum
            await self.check_default_randomization_type_attribute(
                endpoint, subscription_handler.attribute_reports[cluster.Attributes.DefaultRandomizationType][0].value)
            await self.verify_reporting(subscription_handler.attribute_reports,
                                        cluster.Attributes.DefaultRandomizationType, "DefaultRandomizationType", DefaultRandomizationTypeValue)
        else:

            if self.check_pics("SETRF.S.A0012"):  # for cases when it is not supported by DUT, but enabled in PICS
                self.step("41")
                asserts.fail(
                    "PICS file does not correspond to real DUT functionality. DefaultRandomizationType is not actually supported, but SETRF.S.A0012 is True.")
            else:  # attribute is not supported at all
                self.skip_step("41")

        if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DefaultRandomizationOffset):

            self.step("42")

            if not self.check_pics("SETRF.S.A0011"):  # for cases when it is supported by DUT, but disabled in PICS
                log.warning("DefaultRandomizationOffset attribute is actually supported by DUT, but PICS SETRF.S.A0011 is False")

            await self.check_default_randomization_offset_attribute(
                endpoint, subscription_handler.attribute_reports[cluster.Attributes.DefaultRandomizationOffset][0].value)
            await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.DefaultRandomizationOffset,
                                        "DefaultRandomizationOffset", DefaultRandomizationOffsetValue)
        else:

            if self.check_pics("SETRF.S.A0011"):  # for cases when it is not supported by DUT, but enabled in PICS
                self.step("42")
                asserts.fail(
                    "PICS file does not correspond to real DUT functionality. DefaultRandomizationOffset is not actually supported, but SETRF.S.A0011 is True.")
            else:  # attribute is not supported at all
                self.skip_step("42")

        self.step("43")
        # TH sends TestEventTrigger command for Test Event Clear
        await self.send_test_event_trigger_clear()

        self.step("44")
        # TH removes the subscription to Commodity Tariff cluster attributes
        subscription_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
