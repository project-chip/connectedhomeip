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


import asyncio
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects, Globals
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.matter_testing import MatterBaseTest

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityMetering


class CommodityMeteringTestBaseHelper(MatterBaseTest):

    test_event_fake_data = 0x0b07000000000000
    test_event_clear = 0x0b07000000000001

    async def send_test_event_trigger_attrs_value_update(self, t_wait=5):
        await asyncio.sleep(t_wait)

    async def send_test_event_trigger_clear(self, t_wait=5):
        await self.send_test_event_triggers(eventTrigger=self.test_event_clear)
        await asyncio.sleep(t_wait)

    async def checkMeteredQuantityStruct(self, struct: Clusters.CommodityMetering.Structs.MeteredQuantityStruct = None):
        """Supporting function to check MeteredQuantityStruct."""

        matter_asserts.assert_list(struct.tariffComponentIDs,
                                   "TariffComponentIDs field of MeteredQuantityStruct must return a list", max_length=128)
        matter_asserts.assert_list_element_type(
            struct.tariffComponentIDs, int, "TariffComponentIDs field of MeteredQuantityStruct must contain int elements")
        matter_asserts.assert_valid_int64(struct.quantity, 'Quantity field of MeteredQuantityStruct must be int64')

    async def check_maximum_metered_quantities_attribute(self, endpoint, attribute_value=None):

        if not attribute_value:
            self.MaximumMeteredQuantities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaximumMeteredQuantities)
        self.MaximumMeteredQuantities = attribute_value
        if self.MaximumMeteredQuantities is not NullValue:
            matter_asserts.assert_valid_uint16(self.MaximumMeteredQuantities, 'MaximumMeteredQuantities must be uint16')

    async def check_metered_quantity_attribute(self, endpoint, attribute_value=None):

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantity
            )

        if not self.MaximumMeteredQuantities:
            self.check_maximum_metered_quantities_attribute(endpoint)

        if attribute_value is not NullValue:
            # Looks like MaximumMeteredQuantities can't be Null if MeteredQuantity is not Null due to it defines the size of the list
            asserts.assert_not_equal(self.MaximumMeteredQuantities, NullValue, "MaximumMeteredQuantities must not be NullValue")
            matter_asserts.assert_list(attribute_value, "MeteredQuantity attribute must return a list",
                                       max_length=self.MaximumMeteredQuantities)
            matter_asserts.assert_list_element_type(
                attribute_value, cluster.Structs.MeteredQuantityStruct, "MeteredQuantity attribute must contain MeteredQuantityStruct elements")
            for item in attribute_value:
                await self.checkMeteredQuantityStruct(struct=item)

    async def check_metered_quantity_timestamp_attribute(self, endpoint, attribute_value=None):

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantityTimestamp
            )

        if attribute_value is not NullValue:
            matter_asserts.assert_valid_uint32(attribute_value, 'MeteredQuantityTimestamp must be uint32')

    async def check_tariff_unit_attribute(self, endpoint, attribute_value=None):

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit
            )

        if attribute_value is not NullValue:
            asserts.assert_is_instance(
                attribute_value, Globals.Enums.TariffUnitEnum, "TariffUnit attribute must return a TariffUnitEnum")
            matter_asserts.assert_int_in_range(attribute_value, 0, 1, "TariffUnit must be in range 0 - 1")

    async def verify_reporting(self, reports: dict, attribute: ClusterObjects.ClusterAttributeDescriptor, attribute_name: str, saved_value) -> None:

        try:
            asserts.assert_not_equal(reports[attribute][0].value, saved_value,
                                     "Reported value should be different from saved value")
        except KeyError as err:
            asserts.fail(f"There is not reports for attribute {attribute_name}:\n{err}")
