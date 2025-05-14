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
# pylint: disable=invalid-name

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
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_SEPR_2_1."""


import chip.clusters as Clusters
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_SEPRTestBase import CommodityPriceTestBaseHelper

cluster = Clusters.CommodityPrice


class TC_SEPR_2_1(CommodityPriceTestBaseHelper, MatterBaseTest):
    """Implementation of test case TC_SEPR_2_1."""

    def desc_TC_SEPR_2_1(self) -> str:
        """Return a description of this test."""
        return "This test case verifies the non-global attributes of the Commodity Price cluster server"

    def pics_TC_SEPR_2_1(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "SEPR.S"
        ]
        return pics

    def steps_TC_SEPR_2_1(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test).",
                     is_commissioning=True),
            TestStep("2", "TH reads from the DUT the TariffUnit attribute.",
                     "Verify that the DUT response contains a TariffUnitEnum value."),
            TestStep("3", "TH reads from the DUT the Currency attribute.",
                     "Verify that the DUT response contains either null or a CurrencyStruct value."),
            TestStep("4", "TH reads from the DUT the CurrentPrice attribute.",
                     "Verify that the DUT response contains either null or a CommodityPriceStruct value."),
            TestStep("5", "TH reads from the DUT the PriceForecast attribute.",
                     "Verify that the DUT response contains a list of CommodityPriceStruct entries"),
        ]

        return steps

    @run_if_endpoint_matches(has_cluster(cluster))
    async def test_TC_SEPR_2_1(self):
        """Run the test steps."""
        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                             attribute=cluster.Attributes.TariffUnit)
        matter_asserts.assert_valid_enum(
            val, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)

        self.step("3")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                             attribute=cluster.Attributes.Currency)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, Globals.Structs.CurrencyStruct), "val must be of type CurrencyStruct")
            self.check_CurrencyStruct(struct=val)

        self.step("4")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster,
                                                             attribute=cluster.Attributes.CurrentPrice)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.CommodityPriceStruct), "val must be of type CommodityPriceStruct")
            self.check_CommodityPriceStruct(cluster=cluster, struct=val)

        self.step("5")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.PriceForecast):
            val = await self.read_single_attribute_check_success(endpoint=endpoint,
                                                                 cluster=cluster,
                                                                 attribute=cluster.Attributes.PriceForecast)
            matter_asserts.assert_list(val, "PriceForecast attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, "PriceForecast attribute must contain CommodityPriceStruct elements",
                cluster.Structs.CommodityPriceStruct, allow_empty=True)
            for item in val:
                # In the PriceForecast attribute we must not have Description or Components in this returned list
                self.check_CommodityPriceStruct(cluster=cluster, struct=item,
                                                details=Clusters.CommodityPrice.Bitmaps.CommodityPriceDetailBitmap(0))


if __name__ == "__main__":
    default_matter_test_main()
