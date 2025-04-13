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


import logging
import sys
import time

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityPrice


class TC_SEPR_2_1(MatterBaseTest):
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
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)."),
            TestStep("2", "TH reads from the DUT the TariffUnit attribute.",
                     "Verify that the DUT response contains an enum8 value."),
            TestStep("3", "TH reads from the DUT the Currency attribute.",
                     "Verify that the DUT response contains either null or a CurrencyStruct value."),
            TestStep("4", "TH reads from the DUT the CurrentPrice attribute.",
                     "Verify that the DUT response contains either null or a CommodityPriceStruct value."),
            TestStep("5", "TH reads from the DUT the PriceForecast attribute.",
                     "Verify that the DUT response contains a list of CommodityPriceStruct entries"),
        ]

        return steps

    @run_if_endpoint_matches(has_cluster(Clusters.CommodityPrice))
    async def test_TC_SEPR_2_1(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        self.step("1")
        # Commission DUT - already done

        self.step("2")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit)
        matter_asserts.assert_valid_enum(
            val, "TariffUnit attribute must return a TariffUnitEnum", Globals.Enums.TariffUnitEnum)

        self.step("3")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Currency)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, Globals.Structs.CurrencyStruct), "val must be of type CurrencyStruct")
            await self.test_checkCurrencyStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("4")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentPrice)
        if val is not NullValue:
            asserts.assert_true(isinstance(
                val, cluster.Structs.CommodityPriceStruct), "val must be of type CommodityPriceStruct")
            await self.test_checkCommodityPriceStruct(endpoint=endpoint, cluster=cluster, struct=val)

        self.step("5")
        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.PriceForecast):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PriceForecast)
            matter_asserts.assert_list(val, "PriceForecast attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, "PriceForecast attribute must contain CommodityPriceStruct elements",
                cluster.Structs.CommodityPriceStruct, allow_empty=True)
            for item in val:
                # In the PriceForecast attribute we must not have Description or Components in this returned list
                await self.test_checkCommodityPriceStruct(endpoint=endpoint, cluster=cluster, struct=item, bitmap=0)

    async def test_checkCommodityPriceComponentStruct(self,
                                                      endpoint: int = None,
                                                      cluster: Clusters.CommodityPrice = None,
                                                      struct: Clusters.CommodityPrice.Structs.CommodityPriceComponentStruct = None):
        matter_asserts.assert_valid_int64(struct.price, 'Price')
        matter_asserts.assert_valid_enum(
            struct.source, "Source attribute must return a TariffPriceTypeEnum", Globals.Enums.TariffPriceTypeEnum)
        if struct.description is not None:
            matter_asserts.assert_is_string(struct.description, "Description must be a string")
            asserts.assert_less_equal(len(struct.description), 32, "Description must have length at most 32!")
        if struct.tariffComponentId is not None:
            matter_asserts.assert_valid_uint32(struct.tariffComponentID, 'TariffComponentID')

    async def test_checkCommodityPriceStruct(self,
                                             endpoint: int = None,
                                             cluster: Clusters.CommodityPrice = None,
                                             struct: Clusters.CommodityPrice.Structs.CommodityPriceStruct = None,
                                             bitmap: Clusters.CommodityPrice.Bitmaps.CommodityPriceDetailBitmap = 0):
        matter_asserts.assert_valid_uint32(struct.periodStart, 'PeriodStart')
        if struct.periodEnd is not NullValue:
            matter_asserts.assert_valid_uint32(struct.periodEnd, 'PeriodEnd')
        asserts.assert_true(isinstance(
            struct.price, Globals.Structs.PriceStruct), "struct.price must be of type PriceStruct")
        await self.test_checkPriceStruct(endpoint=endpoint, cluster=cluster, struct=struct.price)

        # In the attribute description and components must not be included based on Bitmap (default 0)
        if bitmap & Clusters.CommodityPrice.Bitmaps.CommodityPriceDetailBitmap.kDescription:
            if struct.description is not None:
                matter_asserts.assert_is_string(struct.description, "Description must be a string")
                asserts.assert_less_equal(len(struct.description), 32, "Description must have length at most 32!")
        else:
            asserts.assert_is_none(struct.description)

        if bitmap & Clusters.CommodityPrice.Bitmaps.CommodityPriceDetailBitmap.kComponents:
            if struct.components is not None:
                matter_asserts.assert_list(struct.components, "Components attribute must return a list")
                matter_asserts.assert_list_element_type(
                    struct.components,
                    "Components attribute must contain CommodityPriceComponentStruct elements",
                    cluster.Structs.CommodityPriceComponentStruct, allow_empty=True)
                for item in struct.components:
                    await self.test_checkCommodityPriceComponentStruct(endpoint=endpoint, cluster=cluster, struct=item)
                asserts.assert_less_equal(len(struct.components), 10, "Components must have at most 10 entries!")
        else:
            asserts.assert_is_none(struct.components)

    async def test_checkCurrencyStruct(self,
                                       endpoint: int = None,
                                       cluster: Clusters.CommodityPrice = None,
                                       struct: Globals.Structs.CurrencyStruct = None):
        matter_asserts.assert_valid_uint16(struct.currency, 'Currency')
        asserts.assert_less_equal(struct.currency, 999)
        matter_asserts.assert_valid_uint8(struct.decimalPoints, 'DecimalPoints')

    async def test_checkPriceStruct(self,
                                    endpoint: int = None,
                                    cluster: Clusters.CommodityPrice = None,
                                    struct: Globals.Structs.PriceStruct = None):
        matter_asserts.assert_valid_int64(struct.amount, 'Amount')
        asserts.assert_true(isinstance(
            struct.currency, Globals.Structs.CurrencyStruct), "struct.currency must be of type CurrencyStruct")
        await self.test_checkCurrencyStruct(endpoint=endpoint, cluster=cluster, struct=struct.currency)


if __name__ == "__main__":
    default_matter_test_main()
