#!/usr/bin/env -S python3 -B
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
#

"""
Integration test for manufacturer-specific cluster handling without controller-side codegen.

Validates that the Python controller gracefully handles a cluster that:
1. Exists on the device (advertised in Descriptor.ServerList).
2. Is NOT in the controller's ClusterObjects.ALL_CLUSTERS.
3. Can still be read via tlvAttributes using a raw AttributePath.
4. Is correctly classified by global_attribute_ids.cluster_id_type() as kTest.

Scenario:
A vendor's device was sent through TC_IDM_2_2 during SVE while their vendor ID
and manufacturer-specific cluster were not yet known to the test harness.
The wildcard read inside TC_IDM_2_2 crashed in ChipDeviceCtrl because the
tuple form of Read() expects pathTuple[1] to be a generated Cluster subclass
and the harness had no class for the cluster. Test scripts must instead reach
unknown clusters via AttributePath / tlvAttributes.

Backed by the TestHiddenManufacturerSpecific cluster (0xFFF1FC21) defined in
src/app/zap-templates/zcl/data-model/chip/test-hidden-manufacturer-specific-cluster.xml
and enabled on endpoint 1 of all-clusters-app. The cluster is present in
controller-clusters.matter but excluded from Python ClusterObjects.ALL_CLUSTERS
via the allowlist in
src/controller/python/templates/python-cluster-Objects-py.zapt, which is what
makes it invisible to controller-side codegen.
"""

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects
from matter.clusters.Attribute import AttributePath
from matter.testing import global_attribute_ids
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID = 0xFFF1FC21
TEST_HIDDEN_MANUFACTURER_SPECIFIC_ENDPOINT = 1
TEST_HIDDEN_MANUFACTURER_SPECIFIC_ATTRIBUTE_ID = 0x0000  # TestAttribute


class TC_ManufacturerSpecificCluster(MatterBaseTest):
    """
    Integration test for manufacturer-specific cluster handling.

    Runs against all-clusters-app to validate TestHiddenManufacturerSpecific behavior.
    """

    @async_test_body
    async def test_manufacturer_specific_cluster_handling(self):
        """Test that controller gracefully handles manufacturer-specific clusters without codegen."""

        self.print_step(1, "Verify TestHiddenManufacturerSpecific cluster is NOT in controller's ALL_CLUSTERS")
        asserts.assert_not_in(
            TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID,
            ClusterObjects.ALL_CLUSTERS,
            f"TestHiddenManufacturerSpecific cluster (0x{TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID:08X}) should NOT be in controller's ALL_CLUSTERS"
        )
        print("TestHiddenManufacturerSpecific correctly NOT in controller codegen")

        self.print_step(2, "Verify cluster ID structure per Matter spec")
        vendor_id = (TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID >> 16) & 0xFFFF
        cluster_suffix = TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID & 0xFFFF

        asserts.assert_equal(vendor_id, 0xFFF1, "Expected test vendor ID 0xFFF1")
        asserts.assert_true(
            0xFC00 <= cluster_suffix <= 0xFFFE,
            f"Cluster suffix 0x{cluster_suffix:04X} must be in manufacturer-specific range (0xFC00-0xFFFE)"
        )
        print(f"Cluster ID structure valid: vendor=0x{vendor_id:04X}, suffix=0x{cluster_suffix:04X}")

        self.print_step(3, "Verify cluster ID is correctly classified as kTest")
        cluster_type = global_attribute_ids.cluster_id_type(TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID)
        asserts.assert_equal(
            cluster_type,
            global_attribute_ids.ClusterIdType.kTest,
            f"TestHiddenManufacturerSpecific cluster should be classified as kTest, got {cluster_type}"
        )
        print(f"TestHiddenManufacturerSpecific correctly classified as {cluster_type}")

        self.print_step(4, "Perform wildcard read to get all clusters and attributes")
        read_response = await self.default_controller.Read(self.dut_node_id, [()])
        asserts.assert_is_not_none(read_response, "Read response should not be None")
        asserts.assert_is_not_none(read_response.tlvAttributes, "tlvAttributes should not be None")
        print("Wildcard read successful")

        self.print_step(5, "Verify TestHiddenManufacturerSpecific appears in ServerList")
        server_list = read_response.tlvAttributes[TEST_HIDDEN_MANUFACTURER_SPECIFIC_ENDPOINT][Clusters.Descriptor.id][
            Clusters.Descriptor.Attributes.ServerList.attribute_id
        ]
        asserts.assert_in(
            TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID,
            server_list,
            f"TestHiddenManufacturerSpecific cluster (0x{TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID:08X}) should be in server list on endpoint {TEST_HIDDEN_MANUFACTURER_SPECIFIC_ENDPOINT}"
        )
        print(f"TestHiddenManufacturerSpecific (0x{TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID:08X}) found in ServerList")

        self.print_step(6, "Verify TestHiddenManufacturerSpecific cluster is readable via tlvAttributes")
        asserts.assert_in(
            TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID,
            read_response.tlvAttributes[TEST_HIDDEN_MANUFACTURER_SPECIFIC_ENDPOINT],
            f"TestHiddenManufacturerSpecific cluster (0x{TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID:08X}) should be readable via tlvAttributes"
        )
        print("TestHiddenManufacturerSpecific readable via tlvAttributes")

        self.print_step(7, "Read TestAttribute from TestHiddenManufacturerSpecific using raw AttributePath")
        manufacturer_specific_attribute_path = AttributePath(
            EndpointId=TEST_HIDDEN_MANUFACTURER_SPECIFIC_ENDPOINT,
            ClusterId=TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID,
            AttributeId=TEST_HIDDEN_MANUFACTURER_SPECIFIC_ATTRIBUTE_ID
        )

        specific_read_response = await self.default_controller.Read(self.dut_node_id, [manufacturer_specific_attribute_path])
        raw_value = specific_read_response.tlvAttributes[TEST_HIDDEN_MANUFACTURER_SPECIFIC_ENDPOINT][
            TEST_HIDDEN_MANUFACTURER_SPECIFIC_CLUSTER_ID
        ][TEST_HIDDEN_MANUFACTURER_SPECIFIC_ATTRIBUTE_ID]

        asserts.assert_equal(raw_value, True, "TestAttribute default value should be True")
        print(f"TestAttribute value: {raw_value}")


if __name__ == "__main__":
    default_matter_test_main()
