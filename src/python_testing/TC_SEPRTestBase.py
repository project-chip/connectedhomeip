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


import logging
import typing

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing import matter_asserts

from mobly import asserts

logger = logging.getLogger(__name__)

class CommodityPriceTestBaseHelper:

    async def test_checkCommodityPriceStruct(self,
                                             endpoint: int = None,
                                             cluster: Clusters.CommodityPrice = None,
                                             struct: Clusters.CommodityPrice.Structs.CommodityPriceStruct = None,
                                             bitmap: Clusters.CommodityPrice.Bitmaps.CommodityPriceDetailBitmap = 0):
        matter_asserts.assert_valid_uint32(struct.periodStart, 'PeriodStart')
        if struct.periodEnd is not NullValue:
            matter_asserts.assert_valid_uint32(struct.periodEnd, 'PeriodEnd')

        bPriceIncluded = false
        bPriceLevelIncluded = false

        if struct.price is not NullValue:
            asserts.assert_true(isinstance(
                struct.price, Globals.Structs.PriceStruct), "struct.price must be of type PriceStruct")
            await self.test_checkPriceStruct(endpoint=endpoint, cluster=cluster, struct=struct.price)
            bPriceIncluded = true

        if struct.priceLevel is not NullValue:
            matter_asserts.assert_valid_int16(struct.priceLevel, 'PriceLevel')
            bPriceLevelIncluded = true
        
        asserts.assert_true(bPriceIncluded or bPriceLevelIncluded, "Either Price or PriceLevel must be included")

        # In the attribute description and components must not be included based on Bitmap (default 0)
        if bitmap & cluster.Bitmaps.CommodityPriceDetailBitmap.kDescription:
            if struct.description is not None:
                matter_asserts.assert_is_string(struct.description, "Description must be a string")
                asserts.assert_less_equal(len(struct.description), 32, "Description must have length at most 32!")
        else:
            asserts.assert_is_none(struct.description)

        if bitmap & cluster.Bitmaps.CommodityPriceDetailBitmap.kComponents:
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


    async def send_get_detailed_price_request(self, endpoint: int = None, details: Clusters.CommodityPrice.Bitmaps = 0,
                                         timedRequestTimeoutMs: int = 3000,
                                         expected_status: Status = Status.Success):
        try:
            result = await self.send_single_cmd(cmd=Clusters.CommodityPrice.Commands.GetDetailedPriceRequest(
                details=details),
                endpoint=endpoint,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

            return result

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status,
                                 "Unexpected error returned")

    async def send_get_detailed_forecast_request(self, endpoint: int = None, details: Clusters.CommodityPrice.Bitmaps = 0,
                                         timedRequestTimeoutMs: int = 3000,
                                         expected_status: Status = Status.Success):
        try:
            result = await self.send_single_cmd(cmd=Clusters.CommodityPrice.Commands.GetDetailedForecastRequest(
                details=details),
                endpoint=endpoint,
                timedRequestTimeoutMs=timedRequestTimeoutMs)

            return result

        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status,
                                 "Unexpected error returned")
