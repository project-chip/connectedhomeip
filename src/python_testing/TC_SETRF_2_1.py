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

from TC_SETRF_TestBase import CommodityTariffTestBaseHelper

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class TC_SETRF_2_1(MatterBaseTest, CommodityTariffTestBaseHelper):
    """Implementation of test case TC_SETRF_2_1."""

    def desc_TC_SETRF_2_1(self) -> str:
        """Returns a description of this test"""

        return "Attributes with server as DUT"

    def pics_TC_SETRF_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S"]

    def steps_TC_SETRF_2_1(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", "DUT is successfully commissioned", is_commissioning=True),
            TestStep("2", "TH reads TariffInfo attribute.", """
                     - DUT replies a Null or value of TariffInformationStruct type;
                     - Verify that TariffLabel field has type string with max length 128 or Null;
                     - Verify that ProviderName field has type string with max length 128 or Null;
                     - Verify that BlockMode field has type BlockModeEnum and value in range 0 - 2 or Null value;
                     - Verify that does not contain Currency field if SETRF.S.F00(PRICE) is False;
                     - Verify that contains Currency field if SETRF.S.F00(PRICE) is True;
                        - Verify that it has type currency or Null;
                        - Verify that Currency field of Currency struct has type uint16 with value less or equal 999;
                        - Verify that DecimalPoints field of Currency struct has type uint8."""),
            TestStep("3", "TH reads TariffUnit attribute.", """
                     - DUT replies a Null or a value of TariffUnitEnum type;
                     - Value in range 0 - 1."""),
            TestStep("4", "TH reads StartDate attribute.", """
                     - DUT replies a Null or a value of epoch-s type;
                     - Store value as startDateAttributeValue."""),
            TestStep("5", "TH reads IndividualDays attribute.", """
                     - DUT replies a Null or a value that is a list of DayStruct entries with list length less or equal 50;
                     - Verify that Date field has epoch-s type;
                     - Verify that DayType field has DayTypeEnum type and value in range 0 - 3.
                     - Verify that DayEntryIDs field is list of uint32 with length in range 1 - 96;
                     - Verify that the DayStruct in this list SHALL be arranged in increasing order by the value of Date field;
                     - Verify that the DayStruct in this list do not overlap."""),
            TestStep("6", "TH reads DayEntries attribute.", """
                     - DUT replies a Null or a value that is a list of DayEntryStruct entries;
                     - Verify that DayEntryID field has uint32 type;
                     - Verify that StartTime field has uint16 type with value less or equal 1499;
                        - Store value as startTimeFieldValue;
                     - Verify that if Duration field is presented it has uint16 type with value less or equal 1500 - startTimeFieldValue;
                     - Verify that entry does not contain RandomizationOffset and RandomizationType fields if SETRF.S.F05(RNDM) is False.
                     - Verify that if SETRF.S.F05(RNDM) is True and RandomizationType field is presented it has DayEntryRandomizationType type and value in range 0 - 4;
                        - Store value as randomizationTypeValue;
                        - Verify that if RandomizationOffset field is presented and randomizationTypeValue is 0x01 (Fixed) it has int16 type;
                        - Verify that if RandomizationOffset field is presented and randomizationTypeValue is 0x04 (RandomNegative) it has int16 type and value less or equal 0;
                        - Verify that if RandomizationOffset field is presented and randomizationTypeValue is not 0x01 (Fixed) or 0x04 (RandomNegative) it has int16 type and value greater or equal 0.
                        value greater or equal 0."""),
            TestStep("7", "TH reads DayPatterns attribute.", """
                     - DUT replies a Null or a value that is a list of DayPatternStruct entries with length less or equal 28;
                     - Verify that DayPatternID field has uint32 type;
                     - Verify that DaysOfWeek field has DayPatternDayOfWeekBitmap type;
                     - Verify that DayEntryIDs field is list of uint32 with length in range 1 - 96;"""),
            TestStep("8", "TH reads CalendarPeriods attribute.", """
                     - DUT replies a Null or a value that is a list of CalendarPeriodStruct entries;
                     - Verify that the list length between 1 and 4 entries.
                     - Verify that StartDate field is null or has type epoch-s with value greater or equal startDateAttributeValue;
                     - Verify that the calendar period items in this list are arranged in increasing order by the value of StartDate field;
                     - Verify that if the startDateAttributeValue is Null, the StartDate field on the first CalendarPeriodStruct item SHALL also be Null;
                     - Verify that if the startDateAttributeValue is Null, the StartDate field on any subsequent CalendarPeriodStruct items is not null;
                     - Verify that DayPatternIDs field is list of uint32 with length in range 1 - 7"""),
            TestStep("9", "TH reads CurrentDay attribute.", """
                     - DUT replies a Null or a value of DayStruct type;
                     - Verify that Date field has epoch-s type;
                     - Verify that DayType field has DayTypeEnum type and value in 0 - 3;
                     - Verify that DayEntryIDs field is list of uint32 with length in range 1 - 96."""),
            TestStep("10", "TH reads NextDay attribute.", """
                     - DUT replies a Null or a value of DayStruct type;
                     - Verify that Date field has epoch-s type;
                     - Verify that DayType field has DayTypeEnum type and value in 0 - 3;
                     - Verify that DayEntryIDs field is list of uint32 with length in range 1 - 96."""),
            TestStep("11", "TH reads CurrentDayEntry attribute.", """
                     - DUT replies a Null or a value of DayEntryStruct type;
                     - Verify that DayEntryID field has uint32 type;
                     - Verify that StartTime field has uint16 type and value less or equal 1499;
                        - Store value as startTimeFieldValue;
                     - Verify that if Duration field is presented it has uint16 type;
                     - Verify that if Duration field is presented its value less or equal 1500 - startTimeFieldValue;
                     - Verify that entry does not contain RandomizationOffset and RandomizationType fields if SETRF.S.F05(RNDM) is False;
                     - Verify that if SETRF.S.F05(RNDM) is True and RandomizationType field is presented it has DayEntryRandomizationType type and value in range 0 - 4;
                        - Store value as randomizationTypeValue;
                        - If RandomizationOffset field is presented and randomizationTypeValue is 0x01 (Fixed) it has int16 type;
                        - If RandomizationOffset field is presented and randomizationTypeValue is 0x04 (RandomNegative) it has int16 type and value less or equal 0.
                        - If RandomizationOffset field is presented and randomizationTypeValue is not 0x01 (Fixed) or 0x04 (RandomNegative) it has int16 type and value greater or equal 0."""),
            TestStep("12", "TH reads CurrentDayEntryDate attribute.", "DUT replies a Null or an value of epoch-s type."),
            TestStep("13", "TH reads NextDayEntry attribute.", """
                     - DUT replies a Null or a value of DayEntryStruct type;
                     - Verify that DayEntryID field has uint32 type;
                     - Verify that StartTime field has uint16 type and value less or equal 1499;
                        - Store value as startTimeFieldValue;
                     - Verify that if Duration field is presented it has uint16 type;
                     - Verify that if Duration field is presented its value less or equal 1500 - startTimeFieldValue;
                     - Verify that entry does not contain RandomizationOffset and RandomizationType fields if SETRF.S.F05(RNDM) is False;
                     - Verify that if SETRF.S.F05(RNDM) is True and RandomizationType field is presented it has DayEntryRandomizationType type and value in range 0 - 4;
                        - Store value as randomizationTypeValue;
                        - If RandomizationOffset field is presented and randomizationTypeValue is 0x01 (Fixed) it has int16 type;
                        - If RandomizationOffset field is presented and randomizationTypeValue is 0x04 (RandomNegative) it has int16 type and value less or equal 0.
                        - If RandomizationOffset field is presented and randomizationTypeValue is not 0x01 (Fixed) or 0x04 (RandomNegative) it has int16 type and value greater or equal 0."""),
            TestStep("14", "TH reads NextDayEntryDate attribute.", "DUT replies a Null or an value of epoch-s type."),
            TestStep("15", "TH reads TariffComponents attribute.", """
                     - DUT replies a Null or a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries.
                     - Verify that TariffComponentID field has uint32 type;
                     - Verify that Threshold field is Null or has type int64;
                     - Verify that if Label field is presented it is Null or has type string and max length is 128.
                     - Verify that if Predicted field is presented it has type bool;
                     - Verify that if Predicated is False TariffComponentID is a unique identifier for the combination of values of the Price, Threshold, FriendlyCredit, AuxiliaryLoad and PeakPeriod fields with the value 
                     of the DayEntryIDs field on the encompassing TariffPeriodStruct;
                     - Verify that the entry does not contain Price field if SETRF.S.F00(PRICE) is False;
                     - Verify that if SETRF.S.F00(PRICE) is True and if Price field is presented it is Null or has type TariffPriceStruct;
                     - Verify that if SETRF.S.F00(PRICE) is True and if Price field is presented and it is not Null: 
                        - PriceType field in TariffPriceStruct has TariffPriceTypeEnum type and value in range 0 - 4;
                        - If Price field is presented in TariffPriceStruct it has money type;
                        - If PriceLevel field is presented in TariffPriceStruct it has int16 type;
                     - Verify that entry does not contain FriendlyCredit field if SETRF.S.F01(FCRED) is False;
                     - Verify that if SETRF.S.F01(FCRED) and FriendlyCredit field is presented it has type bool;
                     - Verify that entry does not contain AuxiliaryLoad field if SETRF.S.F02(AUXLD) is False;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented it has AuxiliaryLoadSwitchSettingsStruct type;
                        - Verify that Number field in AuxiliaryLoadSwitchSettingsStruct has uint8 type;
                        - Verify that RequiredState field in AuxiliaryLoadSwitchSettingsStruct has AuxiliaryLoadSettingEnum type and value in range 0 - 2;
                     - Verify that entry does not contain PeakPeriod field if SETRF.S.F03(PEAKP) is False;
                     - Verify that if SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented it has PeakPeriodStruct type;
                        - Verify that Severity field in PeakPeriodStruct has type PeakPeriodSeverityEnum and value in range 0 - 3;
                        - Verify that PeakPeriod in PeakPeriodStruct has type uint16 and its value greater that 1;
                     - Verify that entry does not contain PowerThreshold field if SETRF.S.F04(PWRTHLD) is False;
                     - Verify that if SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented it has PowerThresholdStruct type;
                        - Verify that PowerThreshold field in PowerThresholdStruct has type power-mW;
                        - Verify that ApparentPowerThreshold field in PowerThresholdStruct has type power-mVA;
                        - Verify that PowerThresholdSource field in PowerThresholdStruct has type PowerThresholdSourceEnum and value in range 0 - 2."""),
            TestStep("16", "TH reads TariffPeriods attribute.", """
                     - DUT replies a Null or a value that is a list of TariffPeriodStruct entries;
                     - Verify that the list has 1 or more entries;
                     - Verify that if Label field is Null or has type string and max length is 128;
                     - Verify that DayEntryIDs field is a list of uint32 and its length is greater or equal 1 and less or equal 20;
                     - Verify that TariffComponentIDs field is a list of uint32 and its length is greater or equal 1 and less or equal 20;"""),
            TestStep("17", "TH reads CurrentTariffComponents attribute.", """
                     - DUT replies a Null or a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries.
                     - Verify that TariffComponentID field has uint32 type;
                     - Verify that Threshold field is Null or has type int64;
                     - Verify that if Label field is presented it is Null or has type string and max length is 128.
                     - Verify that if Predicted field is presented it has type bool;
                     - Verify that if Predicated is False TariffComponentID is a unique identifier for the combination of values of the Price, Threshold, FriendlyCredit, AuxiliaryLoad and PeakPeriod fields with the value 
                     of the DayEntryIDs field on the encompassing TariffPeriodStruct;
                     - Verify that the entry does not contain Price field if SETRF.S.F00(PRICE) is False;
                     - Verify that if SETRF.S.F00(PRICE) is True and if Price field is presented it is Null or has type TariffPriceStruct;
                     - Verify that if SETRF.S.F00(PRICE) is True and if Price field is presented and it is not Null: 
                        - PriceType field in TariffPriceStruct has TariffPriceTypeEnum type and value in range 0 - 4;
                        - If Price field is presented in TariffPriceStruct it has money type;
                        - If PriceLevel field is presented in TariffPriceStruct it has int16 type;
                     - Verify that entry does not contain FriendlyCredit field if SETRF.S.F01(FCRED) is False;
                     - Verify that if SETRF.S.F01(FCRED) and FriendlyCredit field is presented it has type bool;
                     - Verify that entry does not contain AuxiliaryLoad field if SETRF.S.F02(AUXLD) is False;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented it has AuxiliaryLoadSwitchSettingsStruct type;
                        - Verify that Number field in AuxiliaryLoadSwitchSettingsStruct has uint8 type;
                        - Verify that RequiredState field in AuxiliaryLoadSwitchSettingsStruct has AuxiliaryLoadSettingEnum type and value in range 0 - 2;
                     - Verify that entry does not contain PeakPeriod field if SETRF.S.F03(PEAKP) is False;
                     - Verify that if SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented it has PeakPeriodStruct type;
                        - Verify that Severity field in PeakPeriodStruct has type PeakPeriodSeverityEnum and value in range 0 - 3;
                        - Verify that PeakPeriod in PeakPeriodStruct has type uint16 and its value greater that 1;
                     - Verify that entry does not contain PowerThreshold field if SETRF.S.F04(PWRTHLD) is False;
                     - Verify that if SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented it has PowerThresholdStruct type;
                        - Verify that PowerThreshold field in PowerThresholdStruct has type power-mW;
                        - Verify that ApparentPowerThreshold field in PowerThresholdStruct has type power-mVA;
                        - Verify that PowerThresholdSource field in PowerThresholdStruct has type PowerThresholdSourceEnum and value in range 0 - 2."""),
            TestStep("18", "TH reads NextTariffComponents attribute.", """
                     - DUT replies a Null or a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries.
                     - Verify that TariffComponentID field has uint32 type;
                     - Verify that Threshold field is Null or has type int64;
                     - Verify that if Label field is presented it is Null or has type string and max length is 128.
                     - Verify that if Predicted field is presented it has type bool;
                     - Verify that if Predicated is False TariffComponentID is a unique identifier for the combination of values of the Price, Threshold, FriendlyCredit, AuxiliaryLoad and PeakPeriod fields with the value 
                     of the DayEntryIDs field on the encompassing TariffPeriodStruct;
                     - Verify that the entry does not contain Price field if SETRF.S.F00(PRICE) is False;
                     - Verify that if SETRF.S.F00(PRICE) is True and if Price field is presented it is Null or has type TariffPriceStruct;
                     - Verify that if SETRF.S.F00(PRICE) is True and if Price field is presented and it is not Null: 
                        - PriceType field in TariffPriceStruct has TariffPriceTypeEnum type and value in range 0 - 4;
                        - If Price field is presented in TariffPriceStruct it has money type;
                        - If PriceLevel field is presented in TariffPriceStruct it has int16 type;
                     - Verify that entry does not contain FriendlyCredit field if SETRF.S.F01(FCRED) is False;
                     - Verify that if SETRF.S.F01(FCRED) and FriendlyCredit field is presented it has type bool;
                     - Verify that entry does not contain AuxiliaryLoad field if SETRF.S.F02(AUXLD) is False;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented it has AuxiliaryLoadSwitchSettingsStruct type;
                        - Verify that Number field in AuxiliaryLoadSwitchSettingsStruct has uint8 type;
                        - Verify that RequiredState field in AuxiliaryLoadSwitchSettingsStruct has AuxiliaryLoadSettingEnum type and value in range 0 - 2;
                     - Verify that entry does not contain PeakPeriod field if SETRF.S.F03(PEAKP) is False;
                     - Verify that if SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented it has PeakPeriodStruct type;
                        - Verify that Severity field in PeakPeriodStruct has type PeakPeriodSeverityEnum and value in range 0 - 3;
                        - Verify that PeakPeriod in PeakPeriodStruct has type uint16 and its value greater that 1;
                     - Verify that entry does not contain PowerThreshold field if SETRF.S.F04(PWRTHLD) is False;
                     - Verify that if SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented it has PowerThresholdStruct type;
                        - Verify that PowerThreshold field in PowerThresholdStruct has type power-mW;
                        - Verify that ApparentPowerThreshold field in PowerThresholdStruct has type power-mVA;
                        - Verify that PowerThresholdSource field in PowerThresholdStruct has type PowerThresholdSourceEnum and value in range 0 - 2."""),
            TestStep("19", "TH reads DefaultRandomizationType attribute.", """
                     -DUT replies a Null or a value of DayEntryRandomizationType in range 0 - 4;
                     - Store value as a DefaultRandomizationTypeValue."""),
            TestStep("20", "TH reads DefaultRandomizationOffset attribute.", """
                     - DUT replies a Null or a value of int16 value;
                     - If DefaultRandomizationTypeValue is 0x00 (Fixed), DUT replies a int16 value;
                     - If DefaultRandomizationTypeValue is 0x04 (RandomNegative), DUT replies a int16 value less or equal 0;
                     - If DefaultRandomizationTypeValue is not 0x01 (Fixed) or 0x04 (RandomNegative), DUT replies a int16 value greater or equal 0."""),
        ]

        return steps

    @async_test_body
    async def test_TC_SETRF_2_1(self):
        """Implements test procedure for test case TC_SETRF_2_1."""

        endpoint = self.get_endpoint()

        self.step("1")
        # Commissioning

        self.step("2")
        # TH reads TariffInfo attribute, expects a TariffInformationStruct
        await self.check_tariff_info_attribute(endpoint)

        self.step("3")
        # TH reads TariffUnit attribute, expects a TariffUnitEnum
        await self.check_tariff_unit_attribute(endpoint)

        self.step("4")
        # TH reads StartDate attribute, expects a epoch-s
        await self.check_start_date_attribute(endpoint)

        self.step("5")
        # TH reads IndividualDays attribute, expects a list of DayStruct
        await self.check_individual_days_attribute(endpoint)

        self.step("6")
        # TH reads DayEntries attribute, expects a list of DayEntryStruct
        await self.check_day_entries_attribute(endpoint)

        self.step("7")
        # TH reads DayPatterns attribute, expects a list of DayPatternStruct
        await self.check_day_patterns_attribute(endpoint)

        self.step("8")
        # TH reads CalendarPeriods attribute, expects a list of CalendarPeriodStruct sorted by StartDate field in increasing order
        await self.check_calendar_periods_attribute(endpoint)

        self.step("9")
        # TH reads CurrentDay attribute, expects a DayStruct
        await self.check_current_day_attribute(endpoint)

        self.step("10")
        # TH reads NextDay attribute, expects a DayStruct
        await self.check_next_day_attribute(endpoint)

        self.step("11")
        # TH reads CurrentDayEntry attribute, expects a DayEntryStruct
        await self.check_current_day_entry_attribute(endpoint)

        self.step("12")
        # TH reads CurrentDayEntryDate attribute, expects a uint32
        await self.check_current_day_entry_date_attribute(endpoint)

        self.step("13")
        # TH reads NextDayEntry attribute, expects a DayEntryStruct
        await self.check_next_day_entry_attribute(endpoint)

        self.step("14")
        # TH reads NextDayEntryDate attribute, expects a uint32
        await self.check_next_day_entry_date_attribute(endpoint)

        self.step("15")
        # TH reads TariffComponents attribute, expects a list of TariffComponentStruct
        await self.check_tariff_components_attribute(endpoint)

        self.step("16")
        # TH reads TariffPeriods attribute, expects a list of TariffPeriodStruct
        await self.check_tariff_periods_attribute(endpoint)

        self.step("17")
        # TH reads CurrentTariffComponents attribute, expects a list of TariffComponentStruct
        await self.check_current_tariff_components_attribute(endpoint)

        self.step("18")
        # TH reads NextTariffComponents attribute, expects a list of TariffComponentStruct
        await self.check_next_tariff_components_attribute(endpoint)

        if not self.check_pics("SETRF.S.A0012"):  # Checks if attribute is supported
            logger.info("PICS SETRF.S.A0012 is not True")
            self.skip_step("19")
        else:
            self.step("19")
            # TH reads DefaultRandomizationType attribute, expects a DayEntryRandomizationTypeEnum
            await self.check_default_randomization_type_attribute(endpoint)

        if not self.check_pics("SETRF.S.A0011"):  # Checks if attribute is supported
            logger.info("PICS SETRF.S.A0011 is not True")
            self.skip_step("20")
        else:
            self.step("20")
            # TH reads DefaultRandomizationOffset attribute, expects a int16
            await self.check_default_randomization_offset_attribute(endpoint)


if __name__ == "__main__":
    default_matter_test_main()
