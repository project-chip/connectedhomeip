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


from typing import List, Optional

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects, Globals
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.matter_testing import AttributeMatcher, AttributeValue, MatterBaseTest

cluster = Clusters.CommodityMetering


class CommodityMeteringTestBaseHelper(MatterBaseTest):

    test_event_fake_data = 0x0b07000000000000
    test_event_clear = 0x0b07000000000001

    MaximumMeteredQuantities = None

    async def send_test_event_trigger_attrs_value_update(self):
        await self.send_test_event_triggers(eventTrigger=self.test_event_fake_data)

    async def send_test_event_trigger_clear(self):
        await self.send_test_event_triggers(eventTrigger=self.test_event_clear)

    async def checkMeteredQuantityStruct(self, struct: Clusters.CommodityMetering.Structs.MeteredQuantityStruct = None):
        """Validate a MeteredQuantityStruct entities.

        Args:
            struct (Clusters.CommodityMetering.Structs.MeteredQuantityStruct, optional): The MeteredQuantityStruct entity to validate. Defaults to None.
        """

        matter_asserts.assert_list(struct.tariffComponentIDs,
                                   "TariffComponentIDs field of MeteredQuantityStruct must return a list with max length 128.", max_length=128)
        for tariffComponentID in struct.tariffComponentIDs:
            matter_asserts.assert_valid_uint32(
                tariffComponentID, 'TariffComponentID field of MeteredQuantityStruct must have uint32 type.')
        matter_asserts.assert_valid_int64(struct.quantity, 'Quantity field of MeteredQuantityStruct must be int64')

    async def check_maximum_metered_quantities_attribute(self, endpoint: int, attribute_value: Optional[int] = None) -> None:
        """Validate the MaximumMeteredQuantities attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[int], optional): The MaximumMeteredQuantities attribute value. Defaults to None.
        """

        self.MaximumMeteredQuantities = attribute_value

        if self.MaximumMeteredQuantities is None:
            self.MaximumMeteredQuantities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaximumMeteredQuantities)

        if self.MaximumMeteredQuantities is not NullValue:
            matter_asserts.assert_valid_uint16(self.MaximumMeteredQuantities, 'MaximumMeteredQuantities must be uint16')

    async def check_metered_quantity_attribute(self, endpoint: int, attribute_value: Optional[List[Clusters.CommodityMetering.Structs.MeteredQuantityStruct]] = None) -> None:
        """Validate the MeteredQuantity attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[List[Clusters.CommodityMetering.Structs.MeteredQuantityStruct]], optional): The MeteredQuantity attribute value. Defaults to None.
        """

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantity
            )

        if attribute_value is not NullValue:

            # Looks like MaximumMeteredQuantities can't be Null if MeteredQuantity is not Null due to it defines the constraints for the length of the list.
            # If MaximumMeteredQuantities is not set yet to a valid real value by any reasons, then we read it.
            if self.MaximumMeteredQuantities is None or self.MaximumMeteredQuantities is NullValue:
                self.MaximumMeteredQuantities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaximumMeteredQuantities)
            await self.check_maximum_metered_quantities_attribute(endpoint, self.MaximumMeteredQuantities)  # validate it
            # and check that it is not Null
            asserts.assert_not_equal(self.MaximumMeteredQuantities, NullValue,
                                     "MaximumMeteredQuantities must not be NullValue")

            matter_asserts.assert_list(attribute_value, "MeteredQuantity attribute must return a list",
                                       max_length=self.MaximumMeteredQuantities)
            matter_asserts.assert_list_element_type(
                attribute_value, cluster.Structs.MeteredQuantityStruct, "MeteredQuantity attribute must contain MeteredQuantityStruct elements")
            for item in attribute_value:
                await self.checkMeteredQuantityStruct(struct=item)

    async def check_metered_quantity_timestamp_attribute(self, endpoint: int, attribute_value: Optional[int] = None) -> None:
        """Validate the MeteredQuantityTimestamp attribute.

        Args:
            endpoint (int): endpoint;;
            attribute_value (Optional[int], optional): The MeteredQuantityTimestamp attribute value. Defaults to None.
        """

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantityTimestamp
            )

        if attribute_value is not NullValue:
            matter_asserts.assert_valid_uint32(attribute_value, 'MeteredQuantityTimestamp must be uint32')

    async def check_tariff_unit_attribute(self, endpoint: int, attribute_value: Optional[Globals.Enums.TariffUnitEnum] = None) -> None:
        """Validate the TariffUnit attribute.

        Args:
            endpoint (int): endpoint;
            attribute_value (Optional[Globals.Enums.TariffUnitEnum], optional): The TariffUnit attribute value. Defaults to None.
        """

        if not attribute_value:
            attribute_value = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.TariffUnit
            )

        if attribute_value is not NullValue:
            asserts.assert_is_instance(
                attribute_value, Globals.Enums.TariffUnitEnum, "TariffUnit attribute must return a TariffUnitEnum")
            matter_asserts.assert_int_in_range(attribute_value, 0, 1, "TariffUnit must be in range 0 - 1")

    async def verify_reporting(self, reports: dict, attribute: ClusterObjects.ClusterAttributeDescriptor, attribute_name: str, saved_value) -> None:
        """Verify that the reported value is different from the saved value.

        Args:
            reports (dict): The received reports.
            attribute (ClusterObjects.ClusterAttributeDescriptor): The attribute to validate.
            attribute_name (str): Human-readable name of the attribute.
            saved_value (Any): Previously saved value.

        Asserts:
            If attribute is not found in reports or ir's value length is 0.
            """

        try:
            asserts.assert_not_equal(reports[attribute][0].value, saved_value,
                                     f"""Reported '{attribute_name}' value should be different from saved value.
                                     Subscriptions should only report when values have changed.""")
        except (KeyError, IndexError) as err:
            asserts.fail(f"There are no reports for attribute {attribute_name}:\n{err}")

    @staticmethod
    def _metered_quantity_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            return report.attribute == cluster.Attributes.MeteredQuantity
        return AttributeMatcher.from_callable(description="MeteredQuantity", matcher=predicate)

    @staticmethod
    def _maximum_metered_quantities_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            return report.attribute == cluster.Attributes.MaximumMeteredQuantities
        return AttributeMatcher.from_callable(description="MaximumMeteredQuantities", matcher=predicate)

    @staticmethod
    def _metered_quantity_timestamp_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            return report.attribute == cluster.Attributes.MeteredQuantityTimestamp
        return AttributeMatcher.from_callable(description="MeteredQuantityTimestamp", matcher=predicate)

    @staticmethod
    def _tariff_unit_matcher() -> AttributeMatcher:
        def predicate(report: AttributeValue) -> bool:
            return report.attribute == cluster.Attributes.TariffUnit
        return AttributeMatcher.from_callable(description="TariffUnit", matcher=predicate)

    def get_mandatory_matchers(self) -> List[AttributeMatcher]:

        return [
            self._metered_quantity_matcher(),
            self._maximum_metered_quantities_matcher(),
            self._metered_quantity_timestamp_matcher(),
            self._tariff_unit_matcher()
        ]
