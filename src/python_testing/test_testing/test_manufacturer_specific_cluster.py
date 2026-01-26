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
Integration test for manufacturer-specific cluster handling without controller-side implementation.

This test validates that the SDK can gracefully handle clusters that:
1. Exist on the device (in ServerList) - TestHiddenMei on all-clusters-app
2. Are NOT in the controller's ClusterObjects.ALL_CLUSTERS
3. Can still be read via tlvAttributes (raw TLV access)
4. Are correctly classified by cluster_id_type()

This test uses the REAL TestHiddenMei cluster (0xFFF1FC21) implemented in:
- src/app/zap-templates/zcl/data-model/chip/test-hidden-mei-cluster.xml
- examples/all-clusters-app/all-clusters-common/all-clusters-app.zap

This addresses the scenario described in:
https://csa-matter.slack.com/archives/C03MA7WR7Q8/p1766543020012939?thread_ts=1766472182.025259
"""

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects
from matter.clusters.Attribute import AttributePath
from matter.testing import global_attribute_ids
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

# TestHiddenMei cluster that exists on device but NOT in controller codegen
TEST_HIDDEN_MEI_CLUSTER_ID = 0xFFF1FC21
TEST_HIDDEN_MEI_ENDPOINT = 1
TEST_HIDDEN_MEI_ATTRIBUTE_ID = 0x0000  # TestAttribute


class TC_ManufacturerSpecificCluster(MatterBaseTest):
    """
    Integration test for manufacturer-specific cluster handling.

    Runs against all-clusters-app to validate real TestHiddenMei cluster behavior.
    """

    @async_test_body
    async def test_manufacturer_specific_cluster_handling(self):
        """Test that controller gracefully handles manufacturer-specific clusters without codegen."""

        # Test 1: Verify TestHiddenMei is NOT in controller's codegen
        self.print_step(1, "Verify TestHiddenMei cluster is NOT in controller's ALL_CLUSTERS")
        asserts.assert_not_in(
            TEST_HIDDEN_MEI_CLUSTER_ID,
            ClusterObjects.ALL_CLUSTERS,
            f"TestHiddenMei cluster (0x{TEST_HIDDEN_MEI_CLUSTER_ID:08X}) should NOT be in controller's ALL_CLUSTERS"
        )
        print("TestHiddenMei correctly NOT in controller codegen")

        # Test 2: Verify cluster ID structure is spec-compliant
        self.print_step(2, "Verify cluster ID structure per Matter spec")
        vendor_id = (TEST_HIDDEN_MEI_CLUSTER_ID >> 16) & 0xFFFF
        cluster_suffix = TEST_HIDDEN_MEI_CLUSTER_ID & 0xFFFF

        asserts.assert_equal(vendor_id, 0xFFF1, "Expected test vendor ID 0xFFF1")
        asserts.assert_true(
            0xFC00 <= cluster_suffix <= 0xFFFE,
            f"Cluster suffix 0x{cluster_suffix:04X} must be in manufacturer-specific range (0xFC00-0xFFFE)"
        )
        print(f"Cluster ID structure valid: vendor=0x{vendor_id:04X}, suffix=0x{cluster_suffix:04X}")

        # Test 3: Verify cluster is correctly classified
        self.print_step(3, "Verify cluster ID is correctly classified as kTest")
        cluster_type = global_attribute_ids.cluster_id_type(TEST_HIDDEN_MEI_CLUSTER_ID)
        asserts.assert_equal(
            cluster_type,
            global_attribute_ids.ClusterIdType.kTest,
            f"TestHiddenMei cluster should be classified as kTest, got {cluster_type}"
        )
        print(f"TestHiddenMei correctly classified as {cluster_type}")

        # Test 4: Read all attributes from device (wildcard read)
        self.print_step(4, "Perform wildcard read to get all clusters and attributes")
        read_response = await self.default_controller.Read(self.dut_node_id, [()])
        asserts.assert_is_not_none(read_response, "Read response should not be None")
        asserts.assert_is_not_none(read_response.tlvAttributes, "tlvAttributes should not be None")
        print("Wildcard read successful")

        # Test 5: Verify TestHiddenMei is in ServerList
        self.print_step(5, "Verify TestHiddenMei appears in ServerList")
        server_list = read_response.tlvAttributes[TEST_HIDDEN_MEI_ENDPOINT][Clusters.Descriptor.id][
            Clusters.Descriptor.Attributes.ServerList.attribute_id
        ]
        asserts.assert_in(
            TEST_HIDDEN_MEI_CLUSTER_ID,
            server_list,
            f"TestHiddenMei cluster (0x{TEST_HIDDEN_MEI_CLUSTER_ID:08X}) should be in server list on endpoint {TEST_HIDDEN_MEI_ENDPOINT}"
        )
        print(f"TestHiddenMei (0x{TEST_HIDDEN_MEI_CLUSTER_ID:08X}) found in ServerList")

        # Test 6: Verify cluster is readable via tlvAttributes
        self.print_step(6, "Verify TestHiddenMei cluster is readable via tlvAttributes")
        asserts.assert_in(
            TEST_HIDDEN_MEI_CLUSTER_ID,
            read_response.tlvAttributes[TEST_HIDDEN_MEI_ENDPOINT],
            f"TestHiddenMei cluster (0x{TEST_HIDDEN_MEI_CLUSTER_ID:08X}) should be readable via tlvAttributes"
        )
        print("TestHiddenMei readable via tlvAttributes")

        # Test 7: Read specific attribute from manufacturer-specific cluster using raw path
        self.print_step(7, "Read TestAttribute from TestHiddenMei using raw AttributePath")
        mei_attribute_path = AttributePath(
            EndpointId=TEST_HIDDEN_MEI_ENDPOINT,
            ClusterId=TEST_HIDDEN_MEI_CLUSTER_ID,
            AttributeId=TEST_HIDDEN_MEI_ATTRIBUTE_ID
        )

        specific_read_response = await self.default_controller.Read(self.dut_node_id, [mei_attribute_path])
        raw_value = specific_read_response.tlvAttributes[TEST_HIDDEN_MEI_ENDPOINT][TEST_HIDDEN_MEI_CLUSTER_ID][TEST_HIDDEN_MEI_ATTRIBUTE_ID]

        # Verify the default value from XML definition
        asserts.assert_equal(raw_value, True, "TestAttribute default value should be True")
        print(f"TestAttribute value: {raw_value}")


if __name__ == "__main__":
    default_matter_test_main()
