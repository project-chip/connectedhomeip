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
#       --PICS src/app/tests/suites/certification/ci-pics-values
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
            TestStep("2", "TH reads TariffInfo attribute.", "DUT replies null or value of TariffInformationStruct type."),
            TestStep("3", "TH reads TariffUnit attribute.", "DUT replies null or value of enum8 (TariffUnitEnum) type."),
            TestStep("4", "TH reads StartDate attribute.", "DUT replies null or value of epoch-s type."),
            TestStep("5", "TH reads DayEntries attribute.", "DUT replies null or list of DayEntryStruct entries."),
            TestStep("6", "TH reads DayPatterns attribute.", "DUT replies null or list of DayPatternStruct entries."),
            TestStep("7", "TH reads CalendarPeriods attribute.", "DUT replies null or list of CalendarPeriodStruct entries."),
            TestStep("8", "TH reads IndividualDays attribute.", "DUT replies null or list of DayStruct entries."),
            TestStep("9", "TH reads CurrentDay attribute.", "DUT replies null or value of DayStruct type."),
            TestStep("10", "TH reads NextDay attribute.", "DUT replies null or value of DayStruct type."),
            TestStep("11", "TH reads CurrentDayEntry attribute.", "DUT replies null or value of DayEntryStruct type."),
            TestStep("12", "TH reads CurrentDayEntryDate attribute.", "DUT replies null or value of epoch-s type."),
            TestStep("13", "TH reads NextDayEntry attribute.", "DUT replies null or value of DayEntryStruct type."),
            TestStep("14", "TH reads NextDayEntryDate attribute.", "DUT replies null or value of epoch-s type."),
            TestStep("15", "TH reads TariffComponents attribute.", "DUT replies null or list of TariffComponentStruct entries."),
            TestStep("16", "TH reads TariffPeriods attribute.", "DUT replies null or list of TariffPeriodStruct entries."),
            TestStep("17", "TH reads CurrentTariffComponents attribute.",
                     "DUT replies null or list of TariffComponentStruct entries."),
            TestStep("18", "TH reads NextTariffComponents attribute.",
                     "DUT replies null or list of TariffComponentStruct entries."),
            TestStep("19", "TH reads DefaultRandomizationOffset attribute.", "DUT replies null or value of int16 type."),
            TestStep("20", "TH reads DefaultRandomizationType attribute.",
                     "DUT replies null or value of DayEntryRandomizationType type."),
            TestStep("21", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                           "TestEventTriggersEnabled is True"),
            TestStep("22", "TH sends TestEventTrigger command to General Diagnostics Cluster for Full Tariff Set Test Event",
                           "DUT replies with SUCCESS status code."),
            TestStep("23", "TH reads TariffInfo attribute.", """
                     - DUT replies a value of TariffInformationStruct type;
                     - Verify that contains Currency field if PICS SETRF.S.F00 is True."""),
            TestStep("24", "TH reads TariffUnit attribute.", """
                     - DUT replies a value of enum8 (TariffUnitEnum) type;
                     - Value is in range 0 - 8."""),
            TestStep("25", "TH reads StartDate attribute.", "DUT replies an value of epoch-s type."),
            TestStep("26", "TH reads DayEntries attribute.", """
                     - DUT replies a value that is a list of DayEntryStruct entries;
                     - Verify that entry does not contain RandomizationOffset and RandomizationType fields if PICS SETRF.S.F05 is False;"""),
            TestStep("27", "TH reads DayPatterns attribute.", """DUT replies a value that is a list of DayPatternStruct entries."""),
            TestStep("28", "TH reads CalendarPeriods attribute.", """
                     - DUT replies a value that is a list of CalendarPeriodStruct entries;
                     - Verify that the list length between 1 and 4 entries;
                     - Verify that the calendar period items in this list are arranged in increasing order by the value of StartDate field;
                     - Verify that if the value of the StartDate attribute is null, the StartDate field on the first CalendarPeriodStruct item SHALL also be null."""),
            TestStep("29", "TH reads IndividualDays attribute.", """
                     - DUT replies a value that is a list of DayStruct entries;
                     - Verify that the list has no more than 50 entries;
                     - Verify that the DayStruct in this list SHALL be arranged in increasing order by the value of Date field."""),
            TestStep("30", "TH reads CurrentDay attribute.", "DUT replies a value of DayStruct type."),
            TestStep("31", "TH reads NextDay attribute.", "DUT replies a value of DayStruct type."),
            TestStep("32", "TH reads CurrentDayEntry attribute.", """
                     - DUT a value of DayEntryStruct type;
                     - Verify that entry does not contain RandomizationOffset and RandomizationType fields if PICS SETRF.S.F05 is False;"""),
            TestStep("33", "TH reads CurrentDayEntryDate attribute.", "DUT replies an value of epoch-s type."),
            TestStep("34", "TH reads NextDayEntry attribute.", """
                     - DUT a value of DayEntryStruct type;
                     - Verify that entry does not contain RandomizationOffset and RandomizationType fields if PICS SETRF.S.F05 is False;"""),
            TestStep("35", "TH reads NextDayEntryDate attribute.", "DUT replies an value of epoch-s type."),
            TestStep("36", "TH reads TariffComponents attribute.", """
                     - DUT replies a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries;
                     - Verify that entry does not contain Price field if PICS SETRF.S.F00 is False;
                     - Verify that entry does not contain FriendlyCredit field if PICS SETRF.S.F01 is False;
                     - Verify that entry does not contain AuxiliaryLoad field if PICS SETRF.S.F02 is False;
                     - Verify that entry does not contain PeakPeriod field if PICS SETRF.S.F03 is False;
                     - Verify that entry does not contain PowerThreshold field if PICS SETRF.S.F04 is False."""),
            TestStep("37", "TH reads TariffPeriods attribute.", """
                     - DUT replies a value that is a list of TariffPeriodStruct entries;
                     - Verify that the list has 1 or more entries."""),
            TestStep("38", "TH reads CurrentTariffComponents attribute.", """
                     - DUT replies a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries;
                     - Verify that entry does not contain Price field if PICS SETRF.S.F00 is False;
                     - Verify that entry does not contain FriendlyCredit field if PICS SETRF.S.F01 is False;
                     - Verify that entry does not contain AuxiliaryLoad field if PICS SETRF.S.F02 is False;
                     - Verify that entry does not contain PeakPeriod field if PICS SETRF.S.F03 is False;
                     - Verify that entry does not contain PowerThreshold field if PICS SETRF.S.F04 is False."""),
            TestStep("39", "TH reads NextTariffComponents attribute.", """
                     - DUT replies a value that is a list of TariffComponentStruct entries;
                     - Verify that the list has 1 or more entries;
                     - Verify that entry does not contain Price field if PICS SETRF.S.F00 is False;
                     - Verify that entry does not contain FriendlyCredit field if PICS SETRF.S.F01 is False;
                     - Verify that entry does not contain AuxiliaryLoad field if PICS SETRF.S.F02 is False;
                     - Verify that entry does not contain PeakPeriod field if PICS SETRF.S.F03 is False;
                     - Verify that entry does not contain PowerThreshold field if PICS SETRF.S.F04 is False."""),
            TestStep("40", "TH reads DefaultRandomizationOffset attribute.", "DUT replies a value of int16 value."),
            TestStep("41", "TH reads DefaultRandomizationType attribute.",
                     "DUT replies a value of DayEntryRandomizationType value."),
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
        # TH reads TariffInfo attribute, expects a Null or TariffInformationStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo
        )
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.TariffInformationStruct), "val must be of type TariffInformationStruct")

        self.step("3")
        # TH reads TariffUnit attribute, expects a Null or TariffUnitEnum
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit
        )
        if val is not NullValue:
            matter_asserts.assert_valid_enum(
                val, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)

        self.step("4")
        # TH reads StartDate attribute, expects a Null or uint32
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate
        )
        if self.StartDate is not NullValue:
            matter_asserts.assert_valid_uint32(self.StartDate, 'StartDate')

        self.step("5")
        # TH reads DayEntries attribute, expects a Null or list of DayEntryStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries
        )
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayEntries attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayEntryStruct, "DayEntries attribute must contain DayEntryStruct elements")

        self.step("6")
        # TH reads DayPatterns attribute, expects a Null or list of DayPatternStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns
        )
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayPatterns attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayPatternStruct, "DayPatterns attribute must contain DayPatternStruct elements")

        self.step("7")
        # TH reads CalendarPeriods attribute, expects a Null or list of CalendarPeriodStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods
        )
        if val is not NullValue:
            matter_asserts.assert_list(val, "CalendarPeriods attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.CalendarPeriodStruct, "CalendarPeriods attribute must contain CalendarPeriodStruct elements")

        self.step("8")
        # TH reads IndividualDays attribute, expects a Null or list of DayStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays
        )
        if val is not NullValue:
            matter_asserts.assert_list(val, "IndividualDays attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayStruct, "IndividualDays attribute must contain DayStruct elements")

        self.step("9")
        # TH reads CurrentDay attribute, expects a Null or DayStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay
        )
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayStruct), "val must be of type DayStruct")
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("10")
        # TH reads NextDay attribute, expects a Null or DayStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay
        )
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayStruct), "val must be of type DayStruct")
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("11")
        # TH reads CurrentDayEntry attribute, expects a Null or DayEntryStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntry
        )
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayEntryStruct), "val must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("12")
        # TH reads CurrentDayEntryDate attribute, expects a Null or uint32
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate
        )
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'val must be of type uint32')

        self.step("13")
        # TH reads NextDayEntry attribute, expects a Null or DayEntryStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntry
        )
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayEntryStruct), "val must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("14")
        # TH reads NextDayEntryDate attribute, expects a Null or uint32
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate
        )
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'val must be of type uint32')

        self.step("15")
        # TH reads TariffComponents attribute, expects a Null or list of TariffComponentStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents
        )
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "TariffComponents attribute must contain TariffComponentStruct elements")

        self.step("16")
        # TH reads TariffPeriods attribute, expects a Null or list of TariffPeriodStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods
        )
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffPeriods attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffPeriodStruct, "TariffPeriods attribute must contain TariffPeriodStruct elements")

        self.step("17")
        # TH reads CurrentTariffComponents attribute, expects a Null or list of TariffComponentStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents
        )
        if val is not NullValue:
            matter_asserts.assert_list(val, "CurrentTariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "CurrentTariffComponents attribute must contain TariffComponentStruct elements")

        self.step("18")
        # TH reads NextTariffComponents attribute, expects a Null or list of TariffComponentStruct
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextTariffComponents
        )
        if val is not NullValue:
            matter_asserts.assert_list(val, "NextTariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "NextTariffComponents attribute must contain TariffComponentStruct elements")

        self.step("19")
        # TH reads DefaultRandomizationOffset attribute, expects a Null or int16
        if not self.check_pics("SETRF.S.A0011"):  # Checks that appropriate feature is enabled
            logger.info("PICS SETRF.S.A0011 is not True")
            self.mark_current_step_skipped()

        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationOffset
        )
        if val is not NullValue:
            matter_asserts.assert_valid_int16(val, 'DefaultRandomizationOffset')

        self.step("20")
        # TH reads DefaultRandomizationType attribute, expects a Null or DayEntryRandomizationTypeEnum
        if not self.check_pics("SETRF.S.A0012"):  # Checks that appropriate feature is enabled
            logger.info("PICS SETRF.S.A0012 is not True")
            self.mark_current_step_skipped()

        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationType
        )
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
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.TariffInformationStruct), "val must be of type TariffInformationStruct")
            await self.checkTariffInformationStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("24")
        # TH reads TariffUnit attribute, expects a TariffUnitEnum
        # Value has to be in range 0 - 8
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit)
        if val is not NullValue:
            matter_asserts.assert_valid_enum(
                val, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)
            asserts.assert_true(val >= 0 and val <= 8, "TariffUnit must be in range 0 - 8")

        self.step("25")
        # TH reads StartDate attribute, expects a uint32
        self.StartDate = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartDate)
        if self.StartDate is not NullValue:
            matter_asserts.assert_valid_uint32(self.StartDate, 'StartDate')

        self.step("26")
        # TH reads DayEntries attribute, expects a list of DayEntryStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayEntries attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayEntryStruct, "DayEntries attribute must contain DayEntryStruct elements")
            for item in val:
                await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("27")
        # TH reads DayPatterns attribute, expects a list of DayPatternStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayPatterns attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayPatternStruct, "DayPatterns attribute must contain DayPatternStruct elements")
            for item in val:
                await self.checkDayPatternStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("28")
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

        self.step("29")
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

        self.step("30")
        # TH reads CurrentDay attribute, expects a DayStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayStruct), "val must be of type DayStruct")
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("31")
        # TH reads NextDay attribute, expects a DayStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayStruct), "val must be of type DayStruct")
            await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("32")
        # TH reads CurrentDayEntry attribute, expects a DayEntryStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntry)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayEntryStruct), "val must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("33")
        # TH reads CurrentDayEntryDate attribute, expects a uint32
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntryDate)
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'val must be of type uint32')

        self.step("34")
        # TH reads NextDayEntry attribute, expects a DayEntryStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntry)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.DayEntryStruct), "val must be of type DayEntryStruct")
            await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("35")
        # TH reads NextDayEntryDate attribute, expects a uint32
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntryDate)
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'val must be of type uint32')

        self.step("36")
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

        self.step("37")
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

        self.step("38")
        # TH reads CurrentTariffComponents attribute, expects a list of TariffComponentStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "CurrentTariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "CurrentTariffComponents attribute must contain TariffComponentStruct elements")
            for item in val:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("39")
        # TH reads NextTariffComponents attribute, expects a list of TariffComponentStruct
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextTariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "NextTariffComponents attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "NextTariffComponents attribute must contain TariffComponentStruct elements")
            for item in val:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("40")
        if not self.check_pics("SETRF.S.A0011"):  # Checks if attribute is supported
            logger.info("PICS SETRF.S.A0011 is not True")
            self.mark_current_step_skipped()

        # TH reads DefaultRandomizationOffset attribute, expects a int16
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.DefaultRandomizationOffset):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationOffset)
            if val is not NullValue:
                matter_asserts.assert_valid_int16(val, 'DefaultRandomizationOffset')

        self.step("41")
        if not self.check_pics("SETRF.S.A0012"):  # Checks if attribute is supported
            logger.info("PICS SETRF.S.A0012 is not True")
            self.mark_current_step_skipped()

        # TH reads DefaultRandomizationType attribute, expects a DayEntryRandomizationTypeEnum
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.DefaultRandomizationType):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DefaultRandomizationType)
            if val is not NullValue:
                matter_asserts.assert_valid_enum(
                    val, "DefaultRandomizationType attribute must return a DayEntryRandomizationTypeEnum", cluster.Enums.DayEntryRandomizationTypeEnum)

        self.step("42")
        # TH sends TestEventTrigger command for Test Event Clear in order to reset cluster state to defaults
        self.send_test_event_trigger_clear()


if __name__ == "__main__":
    default_matter_test_main()
