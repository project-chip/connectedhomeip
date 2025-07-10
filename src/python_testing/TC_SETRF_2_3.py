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

"""Define Matter test case TC_SETRF_2_3."""

import logging

import chip.clusters as Clusters
import test_plan_support
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_SETRF_TestBase import CommodityTariffTestBaseHelper

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class TC_SETRF_2_3(MatterBaseTest, CommodityTariffTestBaseHelper):
    """Implementation of test case TC_SETRF_2_3."""

    def desc_TC_SETRF_2_3(self) -> str:
        """Returns a description of this test"""

        return "Primary Functionality. Check tariff examples loading and changing with Server as DUT"

    def pics_TC_SETRF_2_3(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_SETRF_2_3(self) -> list[TestStep]:
        """Returns a list of steps to run this test case"""

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.",
                     "TestEventTriggersEnabled is True"),
            TestStep("3", "TH reads TariffInfo attribute.", "TariffInfo is Null"),
            TestStep("4", "TH sends TestEventTrigger command for Fake Tariff Test Event.",
                     "DUT replies with SUCCESS status code."),
            TestStep("5", "TH reads TariffInfo attribute.", """
                          - DUT replies a value of TariffInformationStruct type;
                          - TariffLabel is equal to "Full Tariff One"."""),
            TestStep("6", "TH reads DayEntries attribute.", """
                          - DUT replies a value that is a list of DayEntryStruct entries;
                          - Verify that the list length is equal to 5."""),
            TestStep("7", "TH reads DayPatterns attribute.", """
                          - DUT replies a value that is a list of DayPatternStruct entries;
                          - Verify that the list length is equal to 3."""),
            TestStep("8", "TH reads CalendarPeriods attribute.", """
                          - DUT replies a value that is a list of CalendarPeriodStruct entries;
                          - Verify that the list length is equal to 1."""),
            TestStep("9", "TH reads IndividualDays attribute.", """
                          - DUT replies a value that is a list of DayStruct entries;
                          - Verify that the list length is equal to 1."""),
            TestStep("10", "TH reads TariffComponents attribute.", """
                          - DUT replies a value that is a list of TariffComponentStruct entries;
                          - Verify that the list length is equal to 2."""),
            TestStep("11", "TH reads TariffPeriods attribute.", """
                          - DUT replies a value that is a list of TariffPeriodStruct entries;
                          - Verify that the list length is equal to 4."""),
            TestStep("12", "TH sends TestEventTrigger command for Fake Tariff Test Event.",
                     "DUT replies with SUCCESS status code."),
            TestStep("13", "TH reads TariffInfo attribute.", """
                          - DUT replies a value of TariffInformationStruct type;
                          - TariffLabel is equal to "Full Tariff Two"."""),
            TestStep("14", "TH reads DayEntries attribute.", """
                          - DUT replies a value that is a list of DayEntryStruct entries;
                          - Verify that the list length is equal to 4."""),
            TestStep("15", "TH reads DayPatterns attribute.", """
                          - DUT replies a value that is a list of DayPatternStruct entries;
                          - Verify that the list length is equal to 2."""),
            TestStep("16", "TH reads CalendarPeriods attribute.", """
                          - DUT replies a value that is a list of CalendarPeriodStruct entries;
                          - Verify that the list length is equal to 1."""),
            TestStep("17", "TH reads IndividualDays attribute.", """
                          - DUT replies a value that is a list of DayStruct entries;
                          - Verify that the list length is equal to 1."""),
            TestStep("18", "TH reads TariffComponents attribute.", """
                          - DUT replies a value that is a list of TariffComponentStruct entries;
                          - Verify that the list length is equal to 3."""),
            TestStep("19", "TH reads TariffPeriods attribute.", """
                          - DUT replies a value that is a list of TariffPeriodStruct entries;
                          - Verify that the list length is equal to 3."""),
            TestStep("20", "TH sends TestEventTrigger command for Test Event Clear.", "DUT replies with SUCCESS status code."),
        ]

        return steps

    @async_test_body
    async def test_TC_SETRF_2_3(self):
        """Implements test procedure for TC_SETRF_2_3 test case."""

        endpoint = self.get_endpoint()

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics("DGGEN.S") or not self.check_pics("DGGEN.S.A0008") or not self.check_pics("DGGEN.S.C00.Rsp"):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")
        # Commissioning

        self.step("2")
        # TH reads TestEventTriggersEnabled attribute, expects a True
        await self.check_test_event_triggers_enabled()

        self.step("3")
        # TH reads TariffInfo attribute, expects a Null
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo
        )
        asserts.assert_true(val is NullValue, "TariffInfo attribute must return a Null")

        self.step("4")
        # TH sends TestEventTrigger to propagate fake test data to the attributes
        await self.send_test_event_trigger_for_fake_data()

        self.step("5")
        # TH reads TariffInfo attribute, expects a TariffInformationStruct
        # TariffLabel is equal to "Full Tariff One"
        val: cluster.Structs.TariffInformationStruct = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo
        )
        asserts.assert_true(isinstance(
            val, cluster.Structs.TariffInformationStruct), "val must be of type TariffInformationStruct")
        asserts.assert_equal(val.tariffLabel, "Full Tariff One", "TariffLabel field must be equal to 'Full Tariff One'")

        self.step("6")
        # TH reads DayEntries attribute, expects a list of DayEntryStruct
        # Verify that the list length is equal to 5
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayEntries attribute must return a list", 5, 5)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayEntryStruct, "DayEntries attribute must contain DayEntryStruct elements")
            for item in val:
                await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("7")
        # TH reads DayPatterns attribute, expects a list of DayPatternStruct
        # Verify that the list length is equal to 3
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayPatterns attribute must return a list", 3, 3)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayPatternStruct, "DayPatterns attribute must contain DayPatternStruct elements")
            for item in val:
                await self.checkDayPatternStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("8")
        # TH reads CalendarPeriods attribute, expects a list of CalendarPeriodStruct
        # Verify that the list length is equal to 1
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods)
        if val is not NullValue:
            matter_asserts.assert_list(val, "CalendarPeriods attribute must return a list", 1, 1)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.CalendarPeriodStruct, "CalendarPeriods attribute must contain CalendarPeriodStruct elements")
            for item in val:
                await self.checkCalendarPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("9")
        # TH reads IndividualDays attribute, expects a list of DayStruct
        # Verify that the list length is equal to 1
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays)
        if val is not NullValue:
            matter_asserts.assert_list(val, "IndividualDays attribute must return a list", 1, 1)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayStruct, "IndividualDays attribute must contain DayStruct elements")
            for item in val:
                await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("10")
        # TH reads TariffComponents attribute, expects a list of TariffComponentStruct
        # Verify that the list length is equal to 2
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffComponents attribute must return a list", 2, 2)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "TariffComponents attribute must contain TariffComponentStruct elements")
            for item in val:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("11")
        # TH reads TariffPeriods attribute, expects a list of TariffPeriodStruct
        # Verify that the list length is equal to 4
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffPeriods attribute must return a list", 4, 4)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffPeriodStruct, "TariffPeriods attribute must contain TariffPeriodStruct elements")
            for item in val:
                await self.checkTariffPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("12")
        # TH sends TestEventTrigger to change fake test data sample
        await self.send_test_event_trigger_for_fake_data()

        self.step("13")
        # TH reads TariffInfo attribute, expects a TariffInformationStruct
        # TariffLabel is equal to "Full Tariff Two"
        val: cluster.Structs.TariffInformationStruct = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo
        )
        asserts.assert_true(isinstance(
            val, cluster.Structs.TariffInformationStruct), "val must be of type TariffInformationStruct")
        asserts.assert_equal(val.tariffLabel, "Full Tariff Two", "TariffLabel field must be equal to 'Full Tariff Two'")

        self.step("14")
        # TH reads DayEntries attribute, expects a list of DayEntryStruct
        # Verify that the list length is equal to 4
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayEntries attribute must return a list", 4, 4)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayEntryStruct, "DayEntries attribute must contain DayEntryStruct elements")
            for item in val:
                await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("15")
        # TH reads DayPatterns attribute, expects a list of DayPatternStruct
        # Verify that the list length is equal to 2
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayPatterns)
        if val is not NullValue:
            matter_asserts.assert_list(val, "DayPatterns attribute must return a list", 2, 2)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayPatternStruct, "DayPatterns attribute must contain DayPatternStruct elements")
            for item in val:
                await self.checkDayPatternStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("16")
        # TH reads CalendarPeriods attribute, expects a list of CalendarPeriodStruct
        # Verify that the list length is equal to 1
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CalendarPeriods)
        if val is not NullValue:
            matter_asserts.assert_list(val, "CalendarPeriods attribute must return a list", 1, 1)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.CalendarPeriodStruct, "CalendarPeriods attribute must contain CalendarPeriodStruct elements")
            for item in val:
                await self.checkCalendarPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("17")
        # TH reads IndividualDays attribute, expects a list of DayStruct
        # Verify that the list length is equal to 1
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.IndividualDays)
        if val is not NullValue:
            matter_asserts.assert_list(val, "IndividualDays attribute must return a list", 1, 1)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.DayStruct, "IndividualDays attribute must contain DayStruct elements")
            for item in val:
                await self.checkDayStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("18")
        # TH reads TariffComponents attribute, expects a list of TariffComponentStruct
        # Verify that the list length is equal to 3
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffComponents attribute must return a list", 3, 3)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffComponentStruct, "TariffComponents attribute must contain TariffComponentStruct elements")
            for item in val:
                await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("19")
        # TH reads TariffPeriods attribute, expects a list of TariffPeriodStruct
        # Verify that the list length is equal to 3
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)
        if val is not NullValue:
            matter_asserts.assert_list(val, "TariffPeriods attribute must return a list", 3, 3)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.TariffPeriodStruct, "TariffPeriods attribute must contain TariffPeriodStruct elements")
            for item in val:
                await self.checkTariffPeriodStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("20")
        # TH resets cluster state to defaults
        self.send_test_event_trigger_clear()


if __name__ == "__main__":
    default_matter_test_main()
