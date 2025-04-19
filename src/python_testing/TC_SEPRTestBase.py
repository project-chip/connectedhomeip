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
from datetime import datetime, timedelta, timezone

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing import matter_asserts
from mobly import asserts

logger = logging.getLogger(__name__)


class CommodityPriceTestBaseHelper:

    async def test_priceForecast(self,
                                 endpoint: int = None,
                                 cluster: Clusters.CommodityPrice = None,
                                 priceForecast: list = None,
                                 details: Clusters.CommodityPrice.Bitmaps.CommodityPriceDetailBitmap = 0):

        # Verify that the DUT response contains GetDetailedForecastResponse with a
        # list of CommodityPriceStruct entries (it may be empty) and shall have not more than 56 entries.
        matter_asserts.assert_list(priceForecast, "PriceForecast must be a list")
        matter_asserts.assert_list_element_type(
            priceForecast, cluster.Structs.CommodityPriceStruct,
            "PriceForecast list must contain CommodityPriceStruct elements",
            allow_empty=True)

        asserts.assert_less_equal(len(priceForecast),
                                  56, "PriceForecast list must be less than 56 entries")
        for item in priceForecast:
            # The other aspects of this verification are handled by the helper
            await self.test_checkCommodityPriceStruct(endpoint=endpoint, cluster=cluster, struct=item,
                                                      details=details,
                                                      check_time=False)  # Do not check time limits for forecast

    async def test_checkCommodityPriceStruct(self,
                                             endpoint: int = None,
                                             cluster: Clusters.CommodityPrice = None,
                                             struct: Clusters.CommodityPrice.Structs.CommodityPriceStruct = None,
                                             details: Clusters.CommodityPrice.Bitmaps.CommodityPriceDetailBitmap = 0,
                                             check_time: bool = True):

        matter_asserts.assert_valid_uint32(struct.periodStart, 'PeriodStart')

        if check_time:  # Only check time limits when dealing with current price (not list of Forecast)
            # - verify that the PeriodStart is in the past.
            now_time_epoch_s = self.get_current_time_as_epoch_s()
            asserts.assert_less_equal(struct.periodStart, now_time_epoch_s,
                                      "PeriodStart must not be in the past")

            # - verify that the PeriodEnd is in the future or is null.
            if struct.periodEnd is not NullValue:
                matter_asserts.assert_valid_uint32(struct.periodEnd, 'PeriodEnd')
                asserts.assert_greater_equal(struct.periodEnd, now_time_epoch_s,
                                             "PeriodEnd must be in the future")

        bPriceIncluded = False
        bPriceLevelIncluded = False

        if struct.price is not NullValue:
            matter_asserts.assert_valid_int64(struct.price, 'Price')
            bPriceIncluded = True

        if struct.priceLevel is not NullValue:
            matter_asserts.assert_valid_int16(struct.priceLevel, 'PriceLevel')
            bPriceLevelIncluded = True

        asserts.assert_true(bPriceIncluded or bPriceLevelIncluded, "Either Price or PriceLevel must be included")

        # In the attribute description and components must not be included based on Bitmap (default 0)
        if details & cluster.Bitmaps.CommodityPriceDetailBitmap.kDescription:
            if struct.description is not None:
                matter_asserts.assert_is_string(struct.description, "Description must be a string")
                asserts.assert_less_equal(len(struct.description), 32, "Description must have length at most 32!")
        else:
            asserts.assert_is_none(struct.description)

        logger.info(f"PRICE: from: {self.convert_epoch_s_to_time(struct.periodStart, tz=None)} to {self.convert_epoch_s_to_time(struct.periodEnd, tz=None)} : Price: {struct.price} / PriceLevel: {struct.priceLevel} / Description: {struct.description}")

        if details & cluster.Bitmaps.CommodityPriceDetailBitmap.kComponents:
            if struct.components is not None:
                matter_asserts.assert_list(struct.components,  "Components attribute must return a list")
                matter_asserts.assert_list_element_type(
                    struct.components,
                    cluster.Structs.CommodityPriceComponentStruct,
                    "Components attribute must contain CommodityPriceComponentStruct elements",
                    allow_empty=True)
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
        if struct.tariffComponentID is not None:
            matter_asserts.assert_valid_uint32(struct.tariffComponentID, 'TariffComponentID')

        logger.info(
            f"  Component: price: {struct.price} source: {struct.source}, desc: {struct.description} tariffComponentID: {struct.tariffComponentID}")

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

    async def send_test_event_trigger_price_update(self):
        await self.send_test_event_triggers(eventTrigger=0x0095000000000000)

    async def send_test_event_trigger_forecast_update(self):
        await self.send_test_event_triggers(eventTrigger=0x0095000000000001)

    def get_current_time_as_epoch_s(self):
        """Returns current time in UTC in Matter Epoch_S"""
        # Matter epoch is 0 hours, 0 minutes, 0 seconds on Jan 1, 2000 UTC
        now_time = datetime.now()     # Get time in local time

        # Shift to UTC so we can use timezone aware subtraction from Matter epoch in UTC
        now_time = now_time.astimezone(timezone.utc)

        matter_base_time = datetime(2000, 1, 1, 0, 0, 0, 0, tzinfo=timezone.utc)

        now_time_delta = now_time - matter_base_time

        now_time_epoch_s = int(now_time_delta.total_seconds())
        return now_time_epoch_s

    def convert_epoch_s_to_time(self, epoch_s, tz=timezone.utc):
        if epoch_s is not NullValue:
            delta_from_epoch = timedelta(seconds=epoch_s)
            matter_epoch = datetime(2000, 1, 1, 0, 0, 0, 0, tz)

            return matter_epoch + delta_from_epoch
        else:
            return "None"
