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
from typing import List

from mobly import asserts
from TC_SETRF_TestBase import CommodityTariffTestBaseHelper

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class TC_SETRF_2_2(CommodityTariffTestBaseHelper):
    """Implementation of test case TC_SETRF_2_2."""

    def desc_TC_SETRF_2_2(self) -> str:
        """Returns a description of this test"""

        return "Commands with Server as DUT"

    def pics_TC_SETRF_2_2(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S"]

    def steps_TC_SETRF_2_2(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test).",
                     "DUT is commissioned.", is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.",
                     "Value has to be 1 (True)."),
            TestStep("3", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SETRF.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SETRF.TEST_EVENT_TRIGGER for Attributes Value Set Test Event",
                     "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep("4", "TH reads from the DUT the TariffComponents attribute.", """
                     - Verify that the DUT response contains a list of TariffComponentStruct entries with list length in range 1-672;
                        - Store the values of TariffComponentID field of TariffComponentStruct for all entries as tariffComponentIDs;
                        - Store the value of TariffComponentID field of the TariffComponentStruct of the first entry as tariffComponentID1;
                        - Store the TariffComponentStruct of the first entry as tariffComponentStruct1."""),
            TestStep("5", "TH reads from the DUT the TariffPeriods attribute.", """
                     - Verify that the DUT response contains a list of TariffPeriodStruct entries with list length in range 1-672;
                        - Find TariffPeriodStruct entries that contain tariffComponentID1 value in TariffComponentIDs field;
                            - Store DayEntryIDs field values of the found TariffPeriodStruct entries from min to max as dayEntryIDs1 list."""),
            TestStep("6", "TH sends command GetTariffComponent command with TariffComponentID set to tariffComponentID1.", """
                     - Verify that the DUT response contains a GetTariffComponentResponse command;
                        - Verify that Label field is a string with max length 128 or null;
                        - Verify that DayEntryIDs field is a list of unique uint32 entries with list length in range 1 - 96;
                        - Verify that DayEntryIDs field equals dayEntryIDs1;
                        - Verify that TariffComponent field is a TariffComponentStruct equal to tariffComponentStruct1."""),
            TestStep("7", "TH sends command GetTariffComponent command with TariffComponentID set to an uint32 value not equal any from tariffComponentIDs.",
                     "Verify that the DUT response contains status NOT_FOUND(0x8b)."),
            TestStep("8", "TH reads from the DUT the DayEntries attribute.", """
                     - Verify that the DUT response contains a list of DayEntryStruct entries with list length less or equal 672;
                        - Store the values of DayEntryID field of DayEntryStruct for all entries as dayEntryIDs;
                        - Store the value of DayEntryID field of the DayEntryStruct of the first entry as dayEntryID1;
                        - Store the DayEntryStruct of the first entry as dayEntryStruct1."""),
            TestStep("9", "TH sends command GetDayEntry command with DayEntryID set to dayEntryID1.", """
                     - Verify that the DUT response contains a GetDayEntry command;
                        - Verify that DayEntry field is a DayEntryStruct equal to dayEntryStruct1."""),
            TestStep("10", "TH sends command GetDayEntry command with DayEntryID set to an uint32 value not equal any from dayEntryIDs.",
                     "Verify that the DUT response contains status NOT_FOUND(0x8b)."),
            TestStep("11", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SETRF.TEST_EVENT_TRIGGER_KEY and EventTrigger field set to PIXIT.SETRF.TEST_EVENT_TRIGGER for Test Event Clear",
                     "Verify  DUT  responds  w/  status SUCCESS(0x00)."),
        ]

        return steps

    @async_test_body
    async def test_TC_SETRF_2_2(self):
        """Implements test procedure for test case TC_SETRF_2_2."""

        endpoint = self.get_endpoint()

        # Variables that will be used in the test to store intermediate values
        tariffComponentIDs: List[int] = []
        tariffComponentID1: int | None = None
        tariffComponentStruct1: Clusters.CommodityTariff.Structs.TariffComponentStruct | None = None
        dayEntryIDs: List[int] = []
        dayEntryIDs1: List[int] = []
        dayEntryID1: int | None = None
        dayEntryStruct1: Clusters.CommodityTariff.Structs.DayEntryStruct | None = None

        self.step("1")
        # Commissioning

        self.step("2")
        # TH reads TestEventTriggersEnabled attribute, expects a True
        await self.check_test_event_triggers_enabled()

        self.step("3")
        # TH sends TestEventTrigger command to General Diagnostics Cluster for Attributes Value Set Test Event, expects a SUCCESS status code
        await self.send_test_event_trigger_for_attributes_value_set()

        self.step("4")
        # TH reads TariffComponents attribute, expects a list of TariffComponentStruct
        self.tariffComponentsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffComponents)
        await self.check_tariff_components_attribute(endpoint, self.tariffComponentsValue)
        tariffComponentIDs.extend(await self.get_tariff_components_IDs_from_tariff_components_attribute(self.tariffComponentsValue))
        tariffComponentID1 = self.tariffComponentsValue[0].tariffComponentID
        tariffComponentStruct1 = self.tariffComponentsValue[0]

        self.step("5")
        # TH reads TariffPeriods attribute, expects a list of TariffPeriodStruct
        self.tariffPeriodsValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffPeriods)
        await self.check_tariff_periods_attribute(endpoint, self.tariffPeriodsValue)
        dayEntryIDs1 = await self.get_day_entry_IDs_from_tariff_periods_for_particular_tariff_component(tariffComponentID1)

        self.step("6")
        # TH sends command GetTariffComponent command with TariffComponentID field set ID of the first TariffComponentStruct, expects a GetTariffComponentResponse
        try:
            command = Clusters.CommodityTariff.Commands.GetTariffComponent(tariffComponentID=tariffComponentID1)
            result: cluster.Commands.GetTariffComponentResponse = await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=3000)
        except InteractionModelError as err:
            asserts.fail(f"Unexpected error returned: {err.status}")

        asserts.assert_true(isinstance(result, cluster.Commands.GetTariffComponentResponse),
                            "Command must be of type GetTariffComponentResponse.")

        if result.label is not NullValue:
            matter_asserts.assert_is_string(result.label, "Label must be a string.")
            matter_asserts.assert_string_length(result.label, "Label must be between 0 and 128 characters.", 0, 128)

        matter_asserts.assert_list(result.dayEntryIDs, "DayEntryIDs attribute must return a list.", 1, 96)
        for item in result.dayEntryIDs:
            matter_asserts.assert_valid_uint32(item, "DayEntryIDs list element must have uint32 type")
            await self.check_list_elements_uniqueness(result.dayEntryIDs, "DayEntryIDs")
        asserts.assert_equal(sorted(result.dayEntryIDs), sorted(dayEntryIDs1),
                             "DayEntryIDs field in GetTariffComponentResponse must be equal to the DayEntryIDs list of all TariffPeriods where first TariffComponentID is present.")

        asserts.assert_is_instance(result.tariffComponent, cluster.Structs.TariffComponentStruct,
                                   "TariffComponent must be a TariffComponentStruct.")
        await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=result.tariffComponent)
        asserts.assert_equal(result.tariffComponent, tariffComponentStruct1,
                             "TariffComponent field must be equal to the first TariffComponentStruct in TariffComponents attribute.")

        self.step("7")
        # TH sends command GetTariffComponent command with absent TariffComponentID value, expects NOT_FOUND status code
        absentTariffComponentID = await self.generate_unique_uint32_for_IDs(tariffComponentIDs)
        try:
            command = Clusters.CommodityTariff.Commands.GetTariffComponent(
                tariffComponentID=absentTariffComponentID)
            result: cluster.Commands.GetTariffComponentResponse = await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=3000)
            # We expect an exception (InteractionModelError) with NOT_FOUND error, so the line below should not be reached.
            asserts.fail("Unexpected command success on an absent TariffComponent")
        except InteractionModelError as err:
            asserts.assert_equal(
                err.status, Status.NotFound, "Unexpected error returned"
            )

        self.step("8")
        # TH reads DayEntries attribute, expects a list of DayEntryStruct
        self.dayEntriesValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.DayEntries)
        dayEntryIDs.extend(await self.get_day_entry_IDs_from_day_entries_attribute(self.dayEntriesValue))
        dayEntryID1 = self.dayEntriesValue[0].dayEntryID
        dayEntryStruct1 = self.dayEntriesValue[0]

        self.step("9")
        # TH sends command GetDayEntry command with DayEntryID field set to the first DayEntryStruct, TH awaits a GetDayEntryResponse
        try:
            command = Clusters.CommodityTariff.Commands.GetDayEntry(dayEntryID=dayEntryID1)
            result: cluster.Commands.GetDayEntryResponse = await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=3000)
        except InteractionModelError as err:
            asserts.fail(f"Unexpected error returned: {err.status}")

        asserts.assert_true(isinstance(result, cluster.Commands.GetDayEntryResponse),
                            "Command must be of type GetDayEntryResponse.")
        asserts.assert_is_instance(result.dayEntry, cluster.Structs.DayEntryStruct,
                                   "DayEntry must be a DayEntryStruct.")
        await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=result.dayEntry)
        asserts.assert_equal(result.dayEntry, dayEntryStruct1,
                             "DayEntry field must be equal to the first DayEntryStruct in DayEntries attribute.")

        self.step("10")
        # TH sends command GetDayEntry command with DayEntryID field set to an absent value, expects NOT_FOUND status code
        absentDayEntryID = await self.generate_unique_uint32_for_IDs(dayEntryIDs)
        try:
            command = Clusters.CommodityTariff.Commands.GetDayEntry(dayEntryID=absentDayEntryID)
            result: cluster.Commands.GetDayEntryResponse = await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=3000)
            # We expect an exception (InteractionModelError) with NOT_FOUND error, so the line below should not be reached.
            asserts.fail("Unexpected command success on an absent DayEntry")
        except InteractionModelError as err:
            asserts.assert_equal(
                err.status, Status.NotFound, "Unexpected error returned"
            )

        self.step("11")
        # TH sends TestEventTrigger command to General Diagnostics Cluster for Test Event Clear, expects a SUCCESS status code
        await self.send_test_event_trigger_clear()


if __name__ == "__main__":
    default_matter_test_main()
