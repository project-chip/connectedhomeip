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

"""Define Matter test case TC_SETRF_2_4."""

import logging
import time

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
import test_plan_support

from TC_SETRF_TestBase import CommodityTariffTestBaseHelper

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class TC_SETRF_2_4(MatterBaseTest, CommodityTariffTestBaseHelper):
    """Implementation of test case TC_SETRF_2_4."""

    def desc_TC_SETRF_2_4(self) -> str:
        """Returns a description of this test"""

        return "Primary Functionality. Check tariff examples loading and changing with Server as DUT"

    def pics_TC_SETRF_2_4(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_SETRF_2_4(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster."),
            TestStep("3", "TH sends TestEventTrigger command for Test Event Clear."),
            TestStep("4", "TH sends TestEventTrigger command for Test Event Full Tariff Set."),
            TestStep("5", "TH reads TariffInfo attribute."),
            TestStep("6", "TH reads CurrentDay attribute and save it as Current_day."),
            TestStep("7", "TH reads NextDay attribute and save it as Next_day."),
            TestStep("8", "TH sends TestEventTrigger command for Test Event Change Day."),
            TestStep("9", "TH reads CurrentDay attribute."),
            TestStep("10", "TH reads CurrentDayEntry attribute and save it as Current_day_entry."),
            TestStep("11", "TH reads NextDayEntry attribute and save it as Next_day_entry."),
            TestStep("12", "TH sends TestEventTrigger command for Test Event Change Time."),
            TestStep("13", "TH reads CurrentDayEntry attribute."),
            TestStep("14", "TH reads CurrentTariffComponents attribute and save it as current_tariff_component."),
            TestStep("15", "TH reads NextTariffComponents attribute and save it as next_tariff_component."),
            TestStep("16", "TH sends TestEventTrigger command for Test Event Change Time."),
            TestStep("17", "TH reads CurrentTariffComponents attribute."),
        ]
        return steps

    @async_test_body
    async def test_TC_SETRF_2_4(self):
        """Runs the TC_SETRF_2_4 test case."""

        endpoint = self.get_endpoint()

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics("DGGEN.S") or not self.check_pics("DGGEN.S.A0008") or not self.check_pics("DGGEN.S.C00.Rsp"):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")

        self.step("2")
        await self.check_test_event_triggers_enabled()

        self.step("3")
        await self.send_test_event_triggers(eventTrigger=0x0700000000000001)
        time.sleep(3)

        self.step("4")
        await self.send_test_event_triggers(eventTrigger=0x0700000000000000)
        time.sleep(3)

        self.step("5")
        val: cluster.Structs.TariffInformationStruct = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffInfo
        )
        asserts.assert_is_instance(val, cluster.Structs.TariffInformationStruct,
                                   "TariffInfo attribute must return a TariffInformationStruct")

        self.step("6")
        current_day = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay
        )

        self.step("7")
        next_day = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDay
        )
        asserts.assert_not_equal(current_day, next_day, "CurrentDay and NextDay attributes must be different")

        self.step("8")
        await self.send_test_event_triggers(eventTrigger=0x0700000000000002)
        time.sleep(3)

        self.step("9")
        current_day = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDay
        )
        asserts.assert_equal(current_day, next_day, "CurrentDay and NextDay attributes must be the same")

        self.step("10")
        current_day_entry = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntry
        )

        self.step("11")
        next_day_entry = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextDayEntry
        )
        asserts.assert_not_equal(current_day_entry, next_day_entry, "CurrentDayEntry and NextDayEntry attributes must be different")

        self.step("12")
        await self.send_test_event_triggers(eventTrigger=0x0700000000000003)
        time.sleep(3)

        self.step("13")
        current_day_entry = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentDayEntry
        )
        asserts.assert_equal(current_day_entry, next_day_entry, "CurrentDayEntry and NextDayEntry attributes must be the same")

        self.step("14")
        current_tariff_component = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents

        )

        self.step("15")
        next_tariff_component = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NextTariffComponents
        )
        asserts.assert_not_equal(current_tariff_component, next_tariff_component,
                                 "CurrentTariffComponents and NextTariffComponents attributes must be different")

        self.step("16")
        await self.send_test_event_triggers(eventTrigger=0x0700000000000003)
        time.sleep(3)

        self.step("17")
        current_tariff_component = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentTariffComponents
        )
        asserts.assert_equal(current_tariff_component, next_tariff_component,
                             "CurrentTariffComponents and NextTariffComponents attributes must be the same")


if __name__ == "__main__":
    default_matter_test_main()
