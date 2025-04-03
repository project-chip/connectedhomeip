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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --featureSet 0xa
#       --application evse
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

import logging

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityMetering


class COMMODITY_METERING_2_1(MatterBaseTest):

    def desc_COMMODITY_METERING_2_1(self) -> str:
        """Returns a description of this test"""
        return "Attributes with Server as DUT"

    def pics_COMMODITY_METERING_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return []

    def steps_COMMODITY_METERING_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Read MeteredQuantity attribute"),
            TestStep("2", "Read MeteredQuantityTimestamp attribute"),
            TestStep("3", "Read MeasurementType attribute"),
            TestStep("4", "Read MaximumMeteredQuantities attribute"),
        ]
        return steps

    MaximumMeteredQuantities = None

    @run_if_endpoint_matches(has_cluster(Clusters.CommodityMetering))
    async def test_COMMODITY_METERING_2_1(self):
        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        self.step("1")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantity)
        if val is not NullValue:
            matter_asserts.assert_list(val, "MeteredQuantity attribute must return a list")
            matter_asserts.assert_list_element_type(
                val, "MeteredQuantity attribute must contain MeteredQuantityStruct elements", cluster.Structs.MeteredQuantityStruct)
            for item in val:
                await self.test_checkMeteredQuantityStruct(endpoint=endpoint, cluster=cluster, struct=item)

        self.step("2")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantityTimestamp)
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'MeteredQuantityTimestamp')

        self.step("3")
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeasurementType)
        if val is not NullValue:
            matter_asserts.assert_valid_enum(
                val, "MeasurementType attribute must return a MeasurementTypeEnum", Globals.Enums.MeasurementTypeEnum)
            asserts.assert_less_equal(val, self.MaximumMeteredQuantities)

        self.step("4")
        self.MaximumMeteredQuantities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaximumMeteredQuantities)
        if self.MaximumMeteredQuantities is not NullValue:
            matter_asserts.assert_valid_uint16(self.MaximumMeteredQuantities, 'MaximumMeteredQuantities')

    async def test_checkMeteredQuantityStruct(self,
                                              endpoint: int = None,
                                              cluster: Clusters.CommodityMetering = None,
                                              struct: Clusters.CommodityMetering.Structs.MeteredQuantityStruct = None):
        matter_asserts.assert_list(struct.tariffComponentIDs, "TariffComponentIDs attribute must return a list")
        matter_asserts.assert_list_element_type(
            struct.tariffComponentIDs, "TariffComponentIDs attribute must contain int elements", int)
        matter_asserts.assert_valid_int64(struct.quantity, 'Quantity')


if __name__ == "__main__":
    default_matter_test_main()
