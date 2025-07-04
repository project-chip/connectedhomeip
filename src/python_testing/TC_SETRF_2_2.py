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

"""Define Matter test case TC_SETRF_2_2."""

import logging

import chip.clusters as Clusters
import test_plan_support
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_SETRF_TestBase import CommodityTariffTestBaseHelper

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityTariff


class TC_SETRF_2_2(MatterBaseTest, CommodityTariffTestBaseHelper):
    """Implementation of test case TC_SETRF_2_2."""

    def desc_TC_SETRF_2_2(self) -> str:
        """Returns a description of this test"""

        return "Commands with Server as DUT"

    def pics_TC_SETRF_2_2(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SETRF.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_SETRF_2_2(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "TH sends command GetTariffComponent command with TariffComponentID field set to 0."),
            TestStep("3", "TH sends command GetTariffComponent command with TariffComponentID field set to 100."),
            TestStep("4", "TH sends command GetDayEntry command with DayEntryID field set to 0."),
            TestStep("5", "TH sends command GetDayEntry command with DayEntryID field set to 100."),
        ]
        return steps

    @async_test_body
    async def test_TC_SETRF_2_2(self):

        endpoint = self.get_endpoint()

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics("DGGEN.S") or not self.check_pics("DGGEN.S.A0008") or not self.check_pics("DGGEN.S.C00.Rsp"):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")

        self.step("2")
        command = Clusters.CommodityTariff.Commands.GetTariffComponent(tariffComponentID=10)
        result: cluster.Commands.GetTariffComponentResponse = await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=3000)
        asserts.assert_true(isinstance(result, cluster.Commands.GetTariffComponentResponse),
                            "Command must be of type GetTariffComponentResponse.")
        if result.label is not NullValue:
            matter_asserts.assert_is_string(result.label, "Label must be a string.")
            matter_asserts.assert_string_length(result.label, "Label must be between 0 and 32 characters.", 0, 128)
        matter_asserts.assert_list(result.dayEntryIDs, "DayEntryIDs attribute must return a list.", 1)
        for item in result.dayEntryIDs:
            matter_asserts.assert_valid_uint32(item, "DayEntryIDs list element must have uint32 type")
        asserts.assert_is_instance(result.tariffComponent, cluster.Structs.TariffComponentStruct,
                                   "TariffComponent must be a TariffComponentStruct.")
        await self.checkTariffComponentStruct(endpoint=endpoint, cluster=cluster, struct=result.tariffComponent)

        self.step("3")
        try:
            command = Clusters.CommodityTariff.Commands.GetTariffComponent(tariffComponentID=100)
            result: cluster.Commands.GetTariffComponentResponse = await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=3000)
            asserts.fail("Unexpected command success on an absence TariffComponent")
        except InteractionModelError as err:
            asserts.assert_equal(
                err.status, Status.Failure, "Unexpected error returned"
            )

        self.step("4")
        command = Clusters.CommodityTariff.Commands.GetDayEntry(dayEntryID=10)
        result: cluster.Commands.GetDayEntryResponse = await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=3000)
        asserts.assert_true(isinstance(result, cluster.Commands.GetDayEntryResponse),
                            "Command must be of type GetDayEntryResponse.")
        asserts.assert_is_instance(result.dayEntry, cluster.Structs.DayEntryStruct,
                                   "DayEntry must be a DayEntryStruct.")
        await self.checkDayEntryStruct(endpoint=endpoint, cluster=cluster, struct=result.dayEntry)

        self.step("5")
        try:
            command = Clusters.CommodityTariff.Commands.GetDayEntry(dayEntryID=100)
            result: cluster.Commands.GetDayEntryResponse = await self.send_single_cmd(cmd=command, endpoint=endpoint, timedRequestTimeoutMs=3000)
            asserts.fail("Unexpected command success on an absence DayEntry")
        except InteractionModelError as err:
            asserts.assert_equal(
                err.status, Status.Failure, "Unexpected error returned"
            )


if __name__ == "__main__":
    default_matter_test_main()
