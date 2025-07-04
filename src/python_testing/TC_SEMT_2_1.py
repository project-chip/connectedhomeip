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

"""Define Matter test case TC_SEMT_2_1."""

import logging
import time

import chip.clusters as Clusters
import test_plan_support
from chip.clusters import Globals
from chip.clusters.Types import Nullable, NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityMetering


class TC_SEMT_2_1(MatterBaseTest):
    """Implementation of test case TC_SEMT_2_1."""

    def desc_TC_SEMT_2_1(self) -> str:
        """Returns a description of this test"""

        return "Attributes with Server as DUT"

    def pics_TC_SEMT_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["SEMT.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_SEMT_2_1(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "Read MeteredQuantity attribute"),
            TestStep("3", "Read MeteredQuantityTimestamp attribute"),
            TestStep("4", "Read TariffUnit attribute"),
            TestStep("5", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster"),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster for Attributes Value Update Test Event"),
            TestStep("7", "Read MeteredQuantity attribute"),
            TestStep("8", "Read MeteredQuantityTimestamp attribute"),
            TestStep("9", "Read TariffUnit attribute"),
        ]

        return steps

    MaximumMeteredQuantities = None

    async def checkMeteredQuantityStruct(self,
                                         endpoint: int = None,
                                         cluster: Clusters.CommodityMetering = None,
                                         struct: Clusters.CommodityMetering.Structs.MeteredQuantityStruct = None):
        matter_asserts.assert_list(struct.tariffComponentIDs, "TariffComponentIDs attribute must return a list", max_length=128)
        matter_asserts.assert_list_element_type(
            struct.tariffComponentIDs, int, "TariffComponentIDs attribute must contain int elements")
        matter_asserts.assert_valid_int64(struct.quantity, 'Quantity')

    @run_if_endpoint_matches(has_cluster(Clusters.CommodityMetering))
    async def test_TC_SEMT_2_1(self):

        endpoint = self.get_endpoint()

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics("DGGEN.S") or not self.check_pics("DGGEN.S.A0008") or not self.check_pics("DGGEN.S.C00.Rsp"):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")

        self.step("2")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantity
        )
        asserts.assert_true(val is NullValue, "MeteredQuantity attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "MeteredQuantity attribute must be a Nullable")

        self.step("3")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantityTimestamp
        )
        asserts.assert_true(val is NullValue, "MeteredQuantityTimestamp attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "MeteredQuantityTimestamp attribute must be a Nullable")

        self.step("4")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit
        )
        asserts.assert_true(val is NullValue, "TariffUnit attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "TariffUnit attribute must be a Nullable")

        self.step("5")
        await self.check_test_event_triggers_enabled()

        self.step("6")
        await self.send_test_event_triggers(eventTrigger=0x0b07000000000000)
        time.sleep(3)  # Wait for the DUT to process the event and update attributes after sending the test event trigger.

        self.step("7")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantity)
        if val is not NullValue:
            matter_asserts.assert_list(val, "MeteredQuantity attribute must return a list", max_length=128)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.MeteredQuantityStruct, "MeteredQuantity attribute must contain MeteredQuantityStruct elements")
            for item in val:
                await self.checkMeteredQuantityStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("8")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantityTimestamp)
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'MeteredQuantityTimestamp')

        self.step("9")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit)
        if val is not NullValue:
            matter_asserts.assert_valid_enum(
                val, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)


if __name__ == "__main__":
    default_matter_test_main()
