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

import chip.clusters as Clusters
import test_plan_support
from chip.clusters import Globals
from chip.clusters.Types import Nullable, NullValue
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

        return "Attributes with Server as DUT"

    def pics_TC_SETRF_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_SETRF_2_1(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "TH reads TariffInfo attribute.", "DUT replies with null value of Nullable type."),
            TestStep("3", "TH reads TariffUnit attribute.", "DUT replies with null value of Nullable type."),
            TestStep("4", "TH reads StartDate attribute.", "DUT replies with null value of Nullable type."),
            TestStep("5", "TH reads DayEntries attribute.", "DUT replies with null value of Nullable type."),
            TestStep("6", "TH reads DayPatterns attribute.", "DUT replies with null value of Nullable type."),
            TestStep("7", "TH reads CalendarPeriods attribute.", "DUT replies with null value of Nullable type."),
            TestStep("8", "TH reads IndividualDays attribute.", "DUT replies with null value of Nullable type."),
            TestStep("9", "TH reads CurrentDay attribute.", "DUT replies with null value of Nullable type."),
            TestStep("10", "TH reads NextDay attribute.", "DUT replies with null value of Nullable type."),
            TestStep("11", "TH reads CurrentDayEntry attribute.", "DUT replies with null value of Nullable type."),
            TestStep("12", "TH reads CurrentDayEntryDate attribute.", "DUT replies with null value of Nullable type."),
            TestStep("13", "TH reads NextDayEntry attribute.", "DUT replies with null value of Nullable type."),
            TestStep("14", "TH reads NextDayEntryDate attribute.", "DUT replies with null value of Nullable type."),
            TestStep("15", "TH reads TariffComponents attribute.", "DUT replies with null value of Nullable type."),
            TestStep("16", "TH reads TariffPeriods attribute.", "DUT replies with null value of Nullable type."),
            TestStep("17", "TH reads CurrentTariffComponents attribute.", "DUT replies with null value of Nullable type."),
            TestStep("18", "TH reads NextTariffComponents attribute.", "DUT replies with null value of Nullable type."),
            TestStep("19", "TH reads DefaultRandomizationOffset attribute.", "DUT replies with null value of Nullable type."),
            TestStep("20", "TH reads DefaultRandomizationType attribute.", "DUT replies with null value of Nullable type."),
            TestStep("21", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster"),
            TestStep("22", "TH sends TestEventTrigger command to General Diagnostics Cluster for Full Tariff Set Test Event"),
            TestStep("23", "TH reads TariffInfo attribute.", """
                     - DUT replies a value of TariffInformationStruct type;
                     - Contains Currency field if PICS SETRF.S.F00 is True."""),
            TestStep("24", "TH reads TariffUnit attribute.", """
                     - DUT replies a value of enum8 (TariffUnitEnum) type;
                     - Value is in range 0 - 8."""),
            TestStep("25", "TH reads StartDate attribute.", "DUT replies an value of epoch-s type."),
            TestStep("26", "TH reads DayEntries attribute.", """
                     - DUT replies a value that is a list of DayEntryStruct entries;
                     - Contains RandomizationOffset and RandomizationType fields if PICS SETRF.S.F05 is True;"""),
            TestStep("27", "TH reads DayPatterns attribute.", """DUT replies a value that is a list of DayPatternStruct entries."""),
            TestStep("28", "TH reads CalendarPeriods attribute.", """
                     - DUT replies a value that is a list of CalendarPeriodStruct entries;
                     - Verify that the list length between 1 and 4 entries;
                     - The calendar periods in this list are arranged in increasing order by the value of StartDate field;
                     - If and only if the value of the StartDate attribute is null, the value of the StartDate field on the first
                       CalendarPeriodStruct in the CalendarPeriods attribute SHALL also be null."""),
            TestStep("29", "TH reads IndividualDays attribute.", """
                     - DUT replies a value that is a list of DayStruct entries;
                     - Verify that the list has no more than 50 entries;
                     - The DayStruct in this list SHALL be arranged in increasing order by the value of Date field."""),
            TestStep("30", "TH reads CurrentDay attribute.", "DUT replies a value of DayStruct type."),
            TestStep("31", "TH reads NextDay attribute.", "DUT replies a value of DayStruct type."),
            TestStep("32", "TH reads CurrentDayEntry attribute.", """
                     - DUT a value of DayEntryStruct type;
                     - Contains RandomizationOffset and RandomizationType fields if PICS SETRF.S.F05 is True;"""),
            TestStep("33", "TH reads CurrentDayEntryDate attribute.", "DUT replies an value of epoch-s type."),
            TestStep("34", "TH reads NextDayEntry attribute.", """
                     - DUT a value of DayEntryStruct type;
                     - Contains RandomizationOffset and RandomizationType fields if PICS SETRF.S.F05 is True;"""),
            TestStep("35", "TH reads NextDayEntryDate attribute.", "DUT replies an value of epoch-s type."),
            TestStep("36", "TH reads TariffComponents attribute.", """
                     - DUT replies a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries;
                     - Contains Price field if PICS SETRF.S.F00 is True;
                     - Contains FriendlyCredit field if PICS SETRF.S.F01 is True;
                     - Contains AuxiliaryLoad field if PICS SETRF.S.F02 is True;
                     - Contains PeakPeriod field if PICS SETRF.S.F03 is True;
                     - Contains PowerThreshold field if PICS SETRF.S.F04 is True."""),
            TestStep("37", "TH reads TariffPeriods attribute.", """
                     - DUT replies a value that is a list of TariffPeriodStruct entries;
                     - Verify that the list has 1 or more entries."""),
            TestStep("38", "TH reads CurrentTariffComponents attribute.", """
                     - DUT replies a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries;
                     - Contains Price field if PICS SETRF.S.F00 is True;
                     - Contains FriendlyCredit field if PICS SETRF.S.F01 is True;
                     - Contains AuxiliaryLoad field if PICS SETRF.S.F02 is True;
                     - Contains PeakPeriod field if PICS SETRF.S.F03 is True;
                     - Contains PowerThreshold field if PICS SETRF.S.F04 is True."""),
            TestStep("39", "TH reads NextTariffComponents attribute.", """
                     - DUT replies a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries;
                     - Contains Price field if PICS SETRF.S.F00 is True;
                     - Contains FriendlyCredit field if PICS SETRF.S.F01 is True;
                     - Contains AuxiliaryLoad field if PICS SETRF.S.F02 is True;
                     - Contains PeakPeriod field if PICS SETRF.S.F03 is True;
                     - Contains PowerThreshold field if PICS SETRF.S.F04 is True."""),
            TestStep("40", "TH reads DefaultRandomizationOffset attribute.", "DUT replies a value of int16 value."),
            TestStep("41", "TH reads DefaultRandomizationType attribute."),
            TestStep("42", "Reset Cluster state to defaults.", "DUT replies a value of DayEntryRandomizationType value."),
        ]
        return steps

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
        await self.send_test_event_trigger_for_fake_data()

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

        self.step("42")
        self.send_test_event_trigger_clear()


if __name__ == "__main__":
    default_matter_test_main()
