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

import chip.clusters as Clusters
import test_plan_support
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_SETRF_TestBase import CommodityTariffTestBaseHelper

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class TC_SETRF_2_1(MatterBaseTest, CommodityTariffTestBaseHelper):
    """Implementation of test case TC_SETRF_2_1."""

    def desc_TC_SETRF_2_1(self) -> str:
        """Returns a description of this test"""

        return "Attributes with server as DUT"

    def pics_TC_SETRF_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_SETRF_2_1(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "TH reads TariffInfo attribute.", """
                     - DUT replies a Null or value of TariffInformationStruct type;
                     - Verify that TariffLabel field has type string with max length 128;
                     - Verify that ProviderName field has type string with max length 128;
                     - Verify that BlockMode field has type BlockModeEnum and value in range 0 - 2;
                     - Verify that does not contain Currency field if SETRF.S.F00(PRICE) is False;
                     - Verify that contains Currency field if SETRF.S.F00(PRICE) is True;
                     - If Currency field is presented it has type currency;
                     - Currency field of Currency struct has type uint16;
                     - DecimalPoints field of Currency struct has type uint8."""),
            TestStep("3", "TH reads TariffUnit attribute.", """
                     - DUT replies a Null or a value of TariffUnitEnum type;
                     - Value in range 0 - 8."""),
            TestStep("4", "TH reads StartDate attribute.", """
                     - DUT replies a Null or a value of epoch-s type;
                     - Store value as start_date_attribute."""),
            TestStep("5", "TH reads DayEntries attribute.", """
                     - DUT replies a Null or a value that is a list of DayEntryStruct entries;
                     - Verify that DayEntryID field has uint32 type;
                     - Verify that StartTime field has uint16 type;
                     - Verify that if Duration field is presented it has uint16 type;
                     - Verify that if Duration field is presented it’s value less or equal 1500 -start_date_attribute;
                     - Verify that entry does not contain RandomizationOffset and RandomizationType fields if SETRF.S.F05(RNDM) is False.
                     - If SETRF.S.F05(RNDM) and RandomizationType field is presented it has DayEntryRandomizationType type and value in range 0 - 2;
                     - Store value as randomization_type;
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is Fixed it has int16 type;
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is RandomNegative it has int16 type and value less or
                     - equal 0.
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is not Fixed or RandomNegative it has int16 type and
                     value greater or equal 0."""),
            TestStep("6", "TH reads DayPatterns attribute.", """
                     - DUT replies a Null or a value that is a list of DayPatternStruct entries;
                     - Verify that DayPatternID field has uint32 type;
                     - Verify that DaysOfWeek field has DayPatternDayOfWeekBitmap type;
                     - Verify that DayEntryIDs field is list of uint32 with length greater or equal 1 and less or equal 96;"""),
            TestStep("7", "TH reads CalendarPeriods attribute.", """
                     - DUT replies a Null or a value that is a list of CalendarPeriodStruct entries;
                     - Verify that the list length between 1 and 4 entries.
                     - Verify that StartDate is null or has type epoch-s and its value is greater than start_date_attribute;
                     - Verify that the calendar period items in this list are arranged in increasing order by the value of StartDate field.
                     - Verify that if the start_date_attribute is null, the StartDate field on the first CalendarPeriodStruct item SHALL also be null."""),
            TestStep("8", "TH reads IndividualDays attribute.", """
                     - DUT replies a Null or a value that is a list of DayStruct entries;
                     - Verify that the list has no more than 50 entries.
                     - Verify that Date field has epoch-s type;
                     - Verify that DayType field has DayTypeEnum type and value in range 0 - 3.
                     - Verify that DayEntryIDs field is list of uint32 with length greater or equal 1 and less or equal 96;
                     - Verify that the DayStruct in this list SHALL be arranged in increasing order by the value of Date field."""),
            TestStep("9", "TH reads CurrentDay attribute.", """
                     - DUT replies a Null or a value of DayStruct type;
                     - Verify that Date field has epoch-s type;
                     - Verify that DayType field has DayTypeEnum type and value in 0 - 3;
                     - Verify that DayEntryIDs field is list of uint32 with length greater or equal 1 and less or equal 96."""),
            TestStep("10", "TH reads NextDay attribute.", """
                     - DUT replies a Null or a value of DayStruct type;
                     - Verify that Date field has epoch-s type;
                     - Verify that DayType field has DayTypeEnum type and value in 0 - 3;
                     - Verify that DayEntryIDs field is list of uint32 with length greater or equal 1 and less or equal 96."""),
            TestStep("11", "TH reads CurrentDayEntry attribute.", """
                     - DUT replies a Null or a value of DayEntryStruct type;
                     - Verify that DayEntryID field has uint32 type;
                     - Verify that StartTime field has uint16 type;
                     - Verify that if Duration field is presented it has uint16 type;
                     - Verify that if Duration field is presented its value less or equal 1500 -start_date_attribute;
                     - Verify that entry does not contain RandomizationOffset and RandomizationType fields if SETRF.S.F05(RNDM) is False;
                     - If SETRF.S.F05(RNDM) and RandomizationType field is presented it has DayEntryRandomizationType type and value in range 0 - 2;
                     - Store value as randomization_type;
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is Fixed it has int16 type;
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is RandomNegative it has int16 type and value less or equal 0.
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is not Fixed or RandomNegative it has int16 type and value greater or equal 0."""),
            TestStep("12", "TH reads CurrentDayEntryDate attribute.", "DUT replies a Null or an value of epoch-s type."),
            TestStep("13", "TH reads NextDayEntry attribute.", """
                     - DUT replies a Null or a value of DayEntryStruct type;
                     - Verify that DayEntryID field has uint32 type;
                     - Verify that StartTime field has uint16 type;
                     - Verify that if Duration field is presented it has uint16 type;
                     - Verify that if Duration field is presented its value less or equal 1500 -start_date_attribute;
                     - Verify that entry does not contain RandomizationOffset and RandomizationType fields if SETRF.S.F05(RNDM) is False;
                     - If SETRF.S.F05(RNDM) and RandomizationType field is presented it has DayEntryRandomizationType type and value in range 0 - 2;
                     - Store value as randomization_type;
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is Fixed it has int16 type;
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is RandomNegative it has int16 type and value less or equal 0.
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is not Fixed or RandomNegative it has int16 type and value greater or equal 0"""),
            TestStep("14", "TH reads NextDayEntryDate attribute.", "DUT replies a Null or an value of epoch-s type."),
            TestStep("15", "TH reads TariffComponents attribute.", """
                     - DUT replies a Null or a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries.
                     - Verify that TariffComponentID field has uint32 type;
                     - Verify that Threshold field it is null or has type uint32;
                     - Verify that if Label field is presented it is null or has type string and max length is 128.
                     - Verify that if Predicted field is presented it has type bool;
                     - Verify that entry does not contain Currency field if SETRF.S.F00(PRICE) is False;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented it is null or has type currency;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented and it is not null then Currency field of Currency struct has type uint16;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented and it is not null then DecimalPoints field of Currency struct has type uint8;
                     - Verify that entry does not contain FriendlyCredit field if SETRF.S.F01(FCRED) is False;
                     - If SETRF.S.F01(FCRED) and FriendlyCredit field is presented it has type bool;
                     - Verify that entry does not contain AuxiliaryLoad field if SETRF.S.F02(AUXLD) is False;
                     - If SETRF.S.F02(AUXLD) and AuxiliaryLoad field is presented it has AuxiliaryLoadSwitchSettingsStruct type;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented Number field in AuxiliaryLoadSwitchSettingsStruct has uint8 type;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented RequiredState field in AuxiliaryLoadSwitchSettingsStruct has AuxiliaryLoadSettingEnum type and value in range 0 - 2;
                     - Verify that entry does not contain PeakPeriod field if SETRF.S.F03(PEAKP) is False;
                     - If SETRF.S.F03(PEAKP) and PeakPeriod field is presented it has PeakPeriodStruct type;
                     - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented Severity field in PeakPeriodStruct has type PeakPeriodSeverityEnum and value in range 0 - 3;
                     - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented PeakPeriod in PeakPeriodStruct has type uint16 and its value greater that 1;
                     - Verify that entry does not contain PowerThreshold field if SETRF.S.F04(PWRTHLD) is False;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented it has PowerThresholdStruct type;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented PowerThreshold field in PowerThresholdStruct has type int64;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented ApparentPowerThreshold field in PowerThresholdStruct has type int64;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented PowerThresholdSource field in PowerThresholdStruct has type PowerThresholdSourceEnum and value in range 0 - 2."""),
            TestStep("16", "TH reads TariffPeriods attribute.", """
                     - DUT replies a Null or a value that is a list of TariffPeriodStruct entries;
                     - Verify that the list has 1 or more entries;
                     - Verify that if Label field is null or has type string and max length is 128;
                     - Verify that DayEntryIDs field is a list of uint32 and its length is greater or equal 1 and less or equal 20;
                     - Verify that TariffComponentIDs field is a list of uint32 and its length is greater or equal 1 and less or equal 20;"""),
            TestStep("17", "TH reads CurrentTariffComponents attribute.", """
                     - DUT replies a Null or a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries.
                     - Verify that TariffComponentID field has uint32 type;
                     - Verify that Threshold field it is null or has type uint32;
                     - Verify that if Label field is presented it is null or has type string and max length is 128.
                     - Verify that if Predicted field is presented it has type bool;
                     - Verify that does not contain Currency field if SETRF.S.F00(PRICE) is False;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented it is null or has type currency;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented and it is not null then Currency field of Currency struct has type uint16;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented and it is not null then DecimalPoints field of Currency struct has type uint8;
                     - Verify that entry does not contain FriendlyCredit field if SETRF.S.F01(FCRED) is False;
                     - If SETRF.S.F01(FCRED) and FriendlyCredit field is presented it has type bool;
                     - Verify that entry does not contain AuxiliaryLoad field if SETRF.S.F02(AUXLD) is False;
                     - If SETRF.S.F02(AUXLD) and AuxiliaryLoad field is presented it has AuxiliaryLoadSwitchSettingsStruct type;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented Number field in AuxiliaryLoadSwitchSettingsStruct has uint8 type;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented RequiredState field in AuxiliaryLoadSwitchSettingsStruct has AuxiliaryLoadSettingEnum type and value in range 0 - 2;
                     - Verify that entry does not contain PeakPeriod field if SETRF.S.F03(PEAKP) is False;
                     - If SETRF.S.F03(PEAKP) and PeakPeriod field is presented it has PeakPeriodStruct type;
                     - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented Severity field in PeakPeriodStruct has type PeakPeriodSeverityEnum and value in range 0 - 3;
                     - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented PeakPeriod in PeakPeriodStruct has type uint16 and its value greater that 1;
                     - Verify that entry does not contain PowerThreshold field if SETRF.S.F04(PWRTHLD) is False;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented it has PowerThresholdStruct type;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented PowerThreshold field in PowerThresholdStruct has type int64;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented ApparentPowerThreshold field in PowerThresholdStruct has type int64;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented PowerThresholdSource field in PowerThresholdStruct has type PowerThresholdSourceEnum and value in range 0 - 2."""),
            TestStep("18", "TH reads NextTariffComponents attribute.", """
                     - DUT replies a Null or a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries.
                     - Verify that TariffComponentID field has uint32 type;
                     - Verify that Threshold field it is null or has type uint32;
                     - Verify that if Label field is presented it is null or has type string and max length is 128.
                     - Verify that if Predicted field is presented it has type bool;
                     - Verify that does not contain Currency field if SETRF.S.F00(PRICE) is False;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented it is null or has type currency;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented and it is not null then Currency field of Currency struct has type uint16;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented and it is not null then DecimalPoints field of Currency struct has type uint8;
                     - Verify that entry does not contain FriendlyCredit field if SETRF.S.F01(FCRED) is False;
                     - If SETRF.S.F01(FCRED) and FriendlyCredit field is presented it has type bool;
                     - Verify that entry does not contain AuxiliaryLoad field if SETRF.S.F02(AUXLD) is False;
                     - If SETRF.S.F02(AUXLD) and AuxiliaryLoad field is presented it has AuxiliaryLoadSwitchSettingsStruct type;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented Number field in AuxiliaryLoadSwitchSettingsStruct has uint8 type;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented RequiredState field in AuxiliaryLoadSwitchSettingsStruct has AuxiliaryLoadSettingEnum type and value in range 0 - 2;
                     - Verify that entry does not contain PeakPeriod field if SETRF.S.F03(PEAKP) is False;
                     - If SETRF.S.F03(PEAKP) and PeakPeriod field is presented it has PeakPeriodStruct type;
                     - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented Severity field in PeakPeriodStruct has type PeakPeriodSeverityEnum and value in range 0 - 3;
                     - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented PeakPeriod in PeakPeriodStruct has type uint16 and its value greater that 1;
                     - Verify that entry does not contain PowerThreshold field if SETRF.S.F04(PWRTHLD) is False;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented it has PowerThresholdStruct type;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented PowerThreshold field in PowerThresholdStruct has type int64;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented ApparentPowerThreshold field in PowerThresholdStruct has type int64;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented PowerThresholdSource field in PowerThresholdStruct has type PowerThresholdSourceEnum and value in range 0 - 2."""),
            TestStep("19", "TH reads DefaultRandomizationOffset attribute.", "DUT replies a Null or a value of int16 value."),
            TestStep("20", "TH reads DefaultRandomizationType attribute.",
                     "DUT replies a Null or a value of DayEntryRandomizationType and value in range 0 - 2."),
            TestStep("21", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                           "TestEventTriggersEnabled is True"),
            TestStep("22", "TH sends TestEventTrigger command to General Diagnostics Cluster for Full Tariff Set Test Event",
                           "DUT replies with SUCCESS status code."),
            TestStep("23", "TH reads TariffInfo attribute.", """
                     - DUT replies a value of TariffInformationStruct type;
                     - Verify that TariffLabel field has type string with max length 128;
                     - Verify that ProviderName field has type string with max length 128;
                     - Verify that BlockMode field has type BlockModeEnum and value in range 0 - 2;
                     - Verify that does not contain Currency field if SETRF.S.F00(PRICE) is False;
                     - Verify that contains Currency field if SETRF.S.F00(PRICE) is True;
                     - If Currency field is presented it has type currency;
                     - Currency field of Currency struct has type uint16;
                     - DecimalPoints field of Currency struct has type uint8."""),
            TestStep("24", "TH reads TariffUnit attribute.", """
                     - DUT replies a value of TariffUnitEnum type;
                     - Value in range 0 - 8."""),
            TestStep("25", "TH reads StartDate attribute.", """
                     - DUT replies an value of epoch-s type;
                     - Store value as start_date_attribute."""),
            TestStep("26", "TH reads DayEntries attribute.", """
                     - DUT replies a value that is a list of DayEntryStruct entries;
                     - Verify that DayEntryID field has uint32 type;
                     - Verify that StartTime field has uint16 type;
                     - Verify that if Duration field is presented it has uint16 type;
                     - Verify that if Duration field is presented its value less or equal 1500 -start_date_attribute;
                     - Verify that entry does not contain RandomizationOffset and RandomizationType fields if SETRF.S.F05(RNDM) is False.
                     - If SETRF.S.F05(RNDM) and RandomizationType field is presented it has DayEntryRandomizationType type and value in range 0 - 2;
                     - Store value as randomization_type;
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is Fixed it has int16 type;
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is RandomNegative it has int16 type and value less or
                     - equal 0.
                     - If SETRF.S.F05(RNDM) and RandomizationOffset field is presented and randomization_type is not Fixed or RandomNegative it has int16 type and
                     value greater or equal 0."""),
            TestStep("27", "TH reads DayPatterns attribute.", """
                     - DUT replies a value that is a list of DayPatternStruct entries;
                     - Verify that DayPatternID field has uint32 type;
                     - Verify that DaysOfWeek field has DayPatternDayOfWeekBitmap type;
                     - Verify that DayEntryIDs field is list of uint32 with length greater or equal 1 and less or equal 96;"""),
            TestStep("28", "TH reads CalendarPeriods attribute.", """
                     - DUT replies a value that is a list of CalendarPeriodStruct entries;
                     - Verify that the list length between 1 and 4 entries.
                     - Verify that StartDate is null or has type epoch-s and its value is greater than start_date_attribute;
                     - Verify that the calendar period items in this list are arranged in increasing order by the value of StartDate field.
                     - Verify that if the start_date_attribute is null, the StartDate field on the first CalendarPeriodStruct item SHALL also be null."""),
            TestStep("29", "TH reads IndividualDays attribute.", """
                     - DUT replies a value that is a list of DayStruct entries;
                     - Verify that the list has no more than 50 entries.
                     - Verify that Date field has epoch-s type;
                     - Verify that DayType field has DayTypeEnum type and value in range 0 - 3.
                     - Verify that DayEntryIDs field is list of uint32 with length greater or equal 1 and less or equal 96;
                     - Verify that the DayStruct in this list SHALL be arranged in increasing order by the value of Date field."""),
            TestStep("36", "TH reads TariffComponents attribute.", """
                     - DUT replies a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries.
                     - Verify that TariffComponentID field has uint32 type;
                     - Verify that Threshold field it is null or has type uint32;
                     - Verify that if Label field is presented it is null or has type string and max length is 128.
                     - Verify that if Predicted field is presented it has type bool;
                     - Verify that does not contain Currency field if SETRF.S.F00(PRICE) is False;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented it is null or has type currency;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented and it is not null then Currency field of Currency struct has type uint16;
                     - If SETRF.S.F00(PRICE) is True and if Currency field is presented and it is not null then DecimalPoints field of Currency struct has type uint8;
                     - Verify that entry does not contain FriendlyCredit field if SETRF.S.F01(FCRED) is False;
                     - If SETRF.S.F01(FCRED) and FriendlyCredit field is presented it has type bool;
                     - Verify that entry does not contain AuxiliaryLoad field if SETRF.S.F02(AUXLD) is False;
                     - If SETRF.S.F02(AUXLD) and AuxiliaryLoad field is presented it has AuxiliaryLoadSwitchSettingsStruct type;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented Number field in AuxiliaryLoadSwitchSettingsStruct has uint8 type;
                     - Verify that if SETRF.S.F02(AUXLD) is True and AuxiliaryLoad field is presented RequiredState field in AuxiliaryLoadSwitchSettingsStruct has AuxiliaryLoadSettingEnum type and value in range 0 - 2;
                     - Verify that entry does not contain PeakPeriod field if SETRF.S.F03(PEAKP) is False;
                     - If SETRF.S.F03(PEAKP) and PeakPeriod field is presented it has PeakPeriodStruct type;
                     - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented Severity field in PeakPeriodStruct has type PeakPeriodSeverityEnum and value in range 0 - 3;
                     - If SETRF.S.F03(PEAKP) is True and PeakPeriod field is presented PeakPeriod in PeakPeriodStruct has type uint16 and its value greater that 1;
                     - Verify that entry does not contain PowerThreshold field if SETRF.S.F04(PWRTHLD) is False;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented it has PowerThresholdStruct type;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented PowerThreshold field in PowerThresholdStruct has type int64;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented ApparentPowerThreshold field in PowerThresholdStruct has type int64;
                     - If SETRF.S.F04(PWRTHLD) is True and PowerThreshold field is presented PowerThresholdSource field in PowerThresholdStruct has type PowerThresholdSourceEnum and value in range 0 - 2."""),
            TestStep("37", "TH reads TariffPeriods attribute.", """
                     - DUT replies a value that is a list of TariffPeriodStruct entries;
                     - Verify that the list has 1 or more entries;
                     - Verify that if Label field is null or has type string and max length is 128;
                     - Verify that DayEntryIDs field is a list of uint32 and its length is greater or equal 1 and less or equal 20;
                     - Verify that TariffComponentIDs field is a list of uint32 and its length is greater or equal 1 and less or equal 20;"""),
            TestStep("40", "TH reads DefaultRandomizationOffset attribute.", "DUT replies a value of int16 value."),
            TestStep("41", "TH reads DefaultRandomizationType attribute.",
                     "DUT replies a value of DayEntryRandomizationType and value in range 0 - 2."),
            TestStep("42", "TH sends TestEventTrigger command to General Diagnostics Cluster for Test Event Clear",
                           "DUT replies with SUCCESS status code."),
        ]

        return steps

    @async_test_body
    async def test_TC_SETRF_2_1(self):
        """Implements test procedure for test case TC_SETRF_2_1."""

        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        # If TestEventTriggers is not enabled this TC can't be checked properly and has to be skipped.
        if not self.check_pics("DGGEN.S") or not self.check_pics("DGGEN.S.A0008") or not self.check_pics("DGGEN.S.C00.Rsp"):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")
        # Commissioning

        self.step("2")
        # TH reads TariffInfo attribute, expects a TariffInformationStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.TariffInformationStruct), "val must be of type TariffInformationStruct")
            await self.checkTariffInformationStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("3")
        # TH reads TariffUnit attribute, expects a TariffUnitEnum
        # Value has to be in range 0 - 8
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit)
        if val is not NullValue:
            matter_asserts.assert_valid_enum(
                val, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)
            asserts.assert_true(val >= 0 and val <= 8, "TariffUnit must be in range 0 - 8")

        self.step("4")
        # TH reads StartDate attribute, expects a uint32
        self.StartDate = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate)
        if self.StartDate is not NullValue:
            matter_asserts.assert_valid_uint32(self.StartDate, 'StartDate')

        self.step("5")
        # TH reads DayEntries attribute, expects a list of DayEntryStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayEntries attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayEntryStruct, "DayEntries attribute must contain DayEntryStruct elements")
            for item in val:
                await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("6")
        # TH reads DayPatterns attribute, expects a list of DayPatternStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayPatterns attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayPatternStruct, "DayPatterns attribute must contain DayPatternStruct elements")
            for item in val:
                await self.checkDayPatternStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("7")
        # TH reads CalendarPeriods attribute, expects a list of CalendarPeriodStruct sorted by StartDate field in increasing order
        # CalendarPeriods must have at least 1 entries and at most 4 entries
        # If StartDate attribute is not available, CalendarPeriods[0].startDate must be Null
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

        self.step("8")
        # TH reads IndividualDays attribute, expects a list of DayStruct
        # IndividualDays must have at most 50 entries
        # IndividualDays must be sorted by Date in increasing order
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

        self.step("9")
        # TH reads CurrentDay attribute, expects a DayStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayStruct), "val must be of type DayStruct")
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("10")
        # TH reads NextDay attribute, expects a DayStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayStruct), "val must be of type DayStruct")
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("11")
        # TH reads CurrentDayEntry attribute, expects a DayEntryStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntry)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayEntryStruct), "val must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("12")
        # TH reads CurrentDayEntryDate attribute, expects a uint32
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'val must be of type uint32')

        self.step("13")
        # TH reads NextDayEntry attribute, expects a DayEntryStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntry)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayEntryStruct), "val must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("14")
        # TH reads NextDayEntryDate attribute, expects a uint32
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'val must be of type uint32')

        self.step("15")
        # TH reads TariffComponents attribute, expects a list of TariffComponentStruct
        # TariffComponents must have at least 1 entries
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "TariffComponents attribute must contain TariffComponentStruct elements")
            for item in val:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)
            asserts.assert_greater_equal(len(val), 1, "TariffComponents must have at least 1 entries!")

        self.step("16")
        # TH reads TariffPeriods attribute, expects a list of TariffPeriodStruct
        # TariffPeriods must have at least 1 entries
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffPeriods attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffPeriodStruct, "TariffPeriods attribute must contain TariffPeriodStruct elements")
            for item in val:
                await self.checkTariffPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)
            asserts.assert_greater_equal(len(val), 1, "TariffPeriods must have at least 1 entries!")

        self.step("17")
        # TH reads CurrentTariffComponents attribute, expects a list of TariffComponentStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "CurrentTariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "CurrentTariffComponents attribute must contain TariffComponentStruct elements")
            for item in val:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("18")
        # TH reads NextTariffComponents attribute, expects a list of TariffComponentStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextTariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "NextTariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "NextTariffComponents attribute must contain TariffComponentStruct elements")
            for item in val:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("19")
        if not self.check_pics("SETRF.S.A0011"):  # Checks if attribute is supported
            logger.info("PICS SETRF.S.A0011 is not True")
            self.mark_current_step_skipped()

        # TH reads DefaultRandomizationOffset attribute, expects a int16
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.DefaultRandomizationOffset):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationOffset)
            if val is not NullValue:
                matter_asserts.assert_valid_int16(val, 'DefaultRandomizationOffset')

        self.step("20")
        if not self.check_pics("SETRF.S.A0012"):  # Checks if attribute is supported
            logger.info("PICS SETRF.S.A0012 is not True")
            self.mark_current_step_skipped()

        # TH reads DefaultRandomizationType attribute, expects a DayEntryRandomizationTypeEnum
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.DefaultRandomizationType):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationType)
            if val is not NullValue:
                matter_asserts.assert_valid_enum(
                    val, "DefaultRandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)

        self.step("21")
        # TH reads TestEventTriggersEnabled attribute, expects a True
        await self.check_test_event_triggers_enabled()

        self.step("22")
        # TH sends TestEventTrigger to propagate fake test data (similar to real data) to attributes
        await self.send_test_event_trigger_for_fake_data()

        self.step("23")
        # TH reads TariffInfo attribute, expects a TariffInformationStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo)
        asserts.assert_not_equal(val, NullValue, "val must not be NullValue")
        asserts.assert_true(isinstance(val, cluster.Structs.TariffInformationStruct), "val must be of type TariffInformationStruct")
        await self.checkTariffInformationStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("24")
        # TH reads TariffUnit attribute, expects a TariffUnitEnum
        # Value has to be in range 0 - 8
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit)
        asserts.assert_not_equal(val, NullValue, "val must not be NullValue")
        matter_asserts.assert_valid_enum(val, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)
        asserts.assert_true(val >= 0 and val <= 8, "TariffUnit must be in range 0 - 8")

        self.step("25")
        # TH reads StartDate attribute, expects a uint32
        self.StartDate = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate)
        if self.StartDate is not NullValue:
            matter_asserts.assert_valid_uint32(self.StartDate, 'StartDate')

        self.step("26")
        # TH reads DayEntries attribute, expects a list of DayEntryStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries)
        asserts.assert_not_equal(val, NullValue, "val must not be NullValue")
        matter_asserts.assert_list(val, "DayEntries attribute must return a list")
        matter_asserts.assert_list_element_type(val, cluster.Structs.DayEntryStruct,
                                                "DayEntries attribute must contain DayEntryStruct elements")
        for item in val:
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("27")
        # TH reads DayPatterns attribute, expects a list of DayPatternStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
        asserts.assert_not_equal(val, NullValue, "val must not be NullValue")
        matter_asserts.assert_list(val, "DayPatterns attribute must return a list")
        matter_asserts.assert_list_element_type(val, cluster.Structs.DayPatternStruct,
                                                "DayPatterns attribute must contain DayPatternStruct elements")
        for item in val:
            await self.checkDayPatternStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("28")
        # TH reads CalendarPeriods attribute, expects a list of CalendarPeriodStruct sorted by StartDate field in increasing order
        # CalendarPeriods must have at least 1 entries and at most 4 entries
        # If StartDate attribute is not available, CalendarPeriods[0].startDate must be Null
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods)
        asserts.assert_not_equal(val, NullValue, "val must not be NullValue")
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
        # TH reads IndividualDays attribute, expects a list of DayStruct
        # IndividualDays must have at most 50 entries
        # IndividualDays must be sorted by Date in increasing order
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays)
        asserts.assert_not_equal(val, NullValue, "val must not be NullValue")
        matter_asserts.assert_list(val, "IndividualDays attribute must return a list")
        matter_asserts.assert_list_element_type(
            val, cluster.Structs.DayStruct, "IndividualDays attribute must contain DayStruct elements")
        for item in val:
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=item)
        asserts.assert_less_equal(len(val), 50, "IndividualDays must have at most 50 entries!")
        for item in range(len(val) - 1):
            asserts.assert_less(val[item].date, val[item + 1].date,
                                "IndividualDays must be sorted by Date in increasing order!")

        self.step("36")
        # TH reads TariffComponents attribute, expects a list of TariffComponentStruct
        # TariffComponents must have at least 1 entries
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        asserts.assert_not_equal(val, NullValue, "val must not be NullValue")
        matter_asserts.assert_list(val, "TariffComponents attribute must return a list")
        matter_asserts.assert_list_element_type(
            val, cluster.Structs.TariffComponentStruct, "TariffComponents attribute must contain TariffComponentStruct elements")
        for item in val:
            await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)
        asserts.assert_greater_equal(len(val), 1, "TariffComponents must have at least 1 entries!")

        self.step("37")
        # TH reads TariffPeriods attribute, expects a list of TariffPeriodStruct
        # TariffPeriods must have at least 1 entries
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)
        asserts.assert_not_equal(val, NullValue, "val must not be NullValue")
        matter_asserts.assert_list(val, "TariffPeriods attribute must return a list")
        matter_asserts.assert_list_element_type(
            val, cluster.Structs.TariffPeriodStruct, "TariffPeriods attribute must contain TariffPeriodStruct elements")
        for item in val:
            await self.checkTariffPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)
        asserts.assert_greater_equal(len(val), 1, "TariffPeriods must have at least 1 entries!")

        self.step("40")
        if not self.check_pics("SETRF.S.A0011"):  # Checks if attribute is supported
            logger.info("PICS SETRF.S.A0011 is not True")
            self.mark_current_step_skipped()

        # TH reads DefaultRandomizationOffset attribute, expects a int16
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.DefaultRandomizationOffset):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationOffset)
            asserts.assert_not_equal(val, NullValue, "val must not be NullValue")
            matter_asserts.assert_valid_int16(val, 'DefaultRandomizationOffset')

        self.step("41")
        if not self.check_pics("SETRF.S.A0012"):  # Checks if attribute is supported
            logger.info("PICS SETRF.S.A0012 is not True")
            self.mark_current_step_skipped()

        # TH reads DefaultRandomizationType attribute, expects a DayEntryRandomizationTypeEnum
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.DefaultRandomizationType):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationType)
            asserts.assert_not_equal(val, NullValue, "val must not be NullValue")
            matter_asserts.assert_valid_enum(
                val, "DefaultRandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)

        self.step("42")
        # TH sends TestEventTrigger command for Test Event Clear in order to reset cluster state to defaults
        self.send_test_event_trigger_clear()


if __name__ == "__main__":
    default_matter_test_main()
