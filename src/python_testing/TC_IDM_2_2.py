#!/usr/bin/env python3
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
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

from mobly import asserts
from support_modules.idm_support import IDMBaseTest

import matter.clusters as Clusters
from matter.clusters.Attribute import AttributePath
from matter.testing import global_attribute_ids
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

log = logging.getLogger(__name__)


class TC_IDM_2_2(IDMBaseTest, BasicCompositionTests):
    """Test case for IDM-2.2: Report Data Action from DUT to TH.

    This test verifies that the DUT correctly handles read requests and responds
    with appropriate report data actions. It covers various scenarios including:
    - Reading single and multiple attributes
    - Reading global and non-global attributes
    - Handling unsupported endpoints/clusters/attributes
    - Data version filtering
    - Chunked data messages
    - Access control
    """

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.endpoint = 0

    # This test can take a long time to run especially if run in highly
    # congested lab environments since it gathers all
    # attributes/clusters/endpoints from DUT in order to run this.
    @property
    def default_timeout(self) -> int:
        return 600

    def steps_TC_IDM_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "TH sends the Read Request Message to the DUT to read one attribute on a given cluster and endpoint, AttributePath = [[Endpoint = Specific Endpoint, Cluster = Specific ClusterID, Attribute = Specific Attribute]], On receipt of this message, DUT should send a report data action with the attribute value to the DUT.",
                     "On the TH verify the received report data message has the right attribute values.", is_commissioning=True),
            TestStep(2, "TH sends the Read Request Message to the DUT to read all attributes on a given cluster and Endpoint, AttributePath = [[Endpoint = Specific Endpoint, Cluster = Specific ClusterID]], On receipt of this message, DUT should send a report data action with the attribute value to the DUT.",
                     "On the TH verify the received report data message has only the attributes that the TH has privilege to."),
            TestStep(3, "TH sends the Read Request Message to the DUT to read an attribute from a cluster at all Endpoints, AttributePath = [[Cluster = Specific ClusterID, Attribute = Specific Attribute]], On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.",
                     "On the TH verify the received report data message has all the right attribute values."),
            TestStep(4, "TH sends the Read Request Message to the DUT to read a global attribute from all clusters at that Endpoint, AttributePath = [[Endpoint = Specific Endpoint, Attribute = Specific Global Attribute]], On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.",
                     "On the TH verify the received report data message has all the right attribute values."),
            TestStep(5, "TH sends the Read Request Message to the DUT to read all attributes from all clusters on all Endpoints, AttributePath = [[]], On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.",
                     "On the TH verify the received report data message has all the right attribute values."),
            TestStep(6, "TH sends the Read Request Message to the DUT to read a global attribute from all clusters at all Endpoints, AttributePath = [[Attribute = Specific Global Attribute]], On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.",
                     "On the TH verify the received report data message has all the right attribute values."),
            TestStep(7, "TH sends the Read Request Message to the DUT to read all attributes from a cluster at all Endpoints, AttributePath = [[Cluster = Specific ClusterID]], On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.",
                     "On the TH verify the received report data message has all the right attribute values."),
            TestStep(8, "TH sends the Read Request Message to the DUT to read all attributes from all clusters at one Endpoint, AttributePath = [[Endpoint = Specific Endpoint]], On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.",
                     "On the TH verify the received report data message has all the right attribute values."),
            TestStep(9, "TH sends the Read Request Message to the DUT to read any attribute to an unsupported Endpoint, DUT responds with the report data action.",
                     "Verify on the TH that the DUT sends the status code UNSUPPORTED_ENDPOINT"),
            TestStep(10, "TH sends the Read Request Message to the DUT to read any attribute to an unsupported cluster, DUT responds with the report data action.",
                     "Verify on the TH that the DUT sends the status code UNSUPPORTED_CLUSTER"),
            TestStep(11, "TH sends the Read Request Message to the DUT to read an unsupported attribute, DUT responds with the report data action.",
                     "Verify on the TH that the DUT sends the status code UNSUPPORTED_ATTRIBUTE"),
            TestStep(12, "TH sends the Read Request Message to the DUT to read an attribute. Repeat the above steps 3 times.",
                     "On the TH verify the received Report data message has the right attribute values for all the 3 times."),
            TestStep(13, "TH sends a Read Request Message to the DUT to read a particular attribute with the DataVersionFilter Field not set. DUT sends back the attribute value with the DataVersion of the cluster. TH sends a second read request to the same cluster with the DataVersionFilter Field set with the dataversion value received before.",
                     "On the TH verify the received Report data message has the right attribute values."),
            TestStep(14, "TH sends a Read Request Message to the DUT to read a particular attribute with the DataVersionFilter Field not set. DUT sends back the attribute value with the DataVersion of the cluster. TH sends a write request to the same cluster to write to any attribute. TH sends a second read request to read an attribute from the same cluster with the DataVersionFilter Field set with the dataversion value received before.",
                     "DUT should send a report data action with the attribute value to the TH."),
            TestStep(15, "TH sends a Read Request Message to the DUT to read all attributes on a cluster with the DataVersionFilter Field not set. DUT sends back the all the attribute values with the DataVersion of the cluster. TH sends a write request to the same cluster to write to any attribute. TH sends a second read request to read all the attributes from the same cluster with the DataVersionFilter Field set with the dataversion value received before.",
                     "DUT should send a report data action with all, the attribute values to the TH."),
            TestStep(16, "TH sends a Read Request Message to the DUT to read a particular attribute on a particular cluster with the DataVersionFilter Field not set. DUT sends back the attribute value with the DataVersion of the cluster. TH sends a read request to the same cluster to read any attribute with the right DataVersion(received in the previous step) and also an older DataVersion. The Read Request Message should have 2 DataVersionIB filters.",
                     "DUT should send a report data action with the attribute value to the TH."),
            TestStep(17, "TH sends a Read Request Message to the DUT to read any supported attribute/wildcard on a particular cluster say A with the DataVersionFilter Field not set. DUT sends back the attribute value with the DataVersion of the cluster A. TH sends a Read Request Message to read any supported attribute/wildcard on cluster A and any supported attribute/wildcard on another cluster B. DataVersionList field should only contain the DataVersion of cluster A.",
                     "Verify that the DUT sends a report data action with the attribute value from the cluster B to the TH. Verify that the DUT does not send the attribute value from cluster A."),
            TestStep(18, "TH sends a Read Request Message to the DUT to read a non global attribute from all clusters at that Endpoint, AttributePath = [[Endpoint = Specific Endpoint, Attribute = Specific Non Global Attribute]] +",
                     "On the TH verify that the DUT sends an error message and not the value of the attribute."),
            TestStep(19, "TH sends a Read Request Message to the DUT to read a non global attribute from all clusters at all Endpoints, AttributePath = [[Attribute = Specific Non Global Attribute]] +",
                     "On the TH verify that the DUT sends an error message and not the value of the attribute."),
            TestStep(20, "TH should have access to only a single cluster at one Endpoint1. TH sends a Read Request Message to the DUT to read all attributes from all clusters at Endpoint1, AttributePath = [[Endpoint = Specific Endpoint]] +",
                     "Verify that the DUT sends back data of all attributes only from that one cluster to which it has access. Verify that there are no errors sent back for attributes the TH has no access to."),
            TestStep(21, "TH sends a Read Request Message to read all events and attributes from the DUT.",
                     "Verify that the DUT sends back data of all attributes and events that the TH has access to."),
        ]

    # Update the test method to call functions directly with explicit parameters
    @async_test_body
    async def test_TC_IDM_2_2(self):
        # Test Setup with robust endpoint/cluster discovery
        await self.setup_class_helper(allow_pase=False)

        self.step(1)
        # Read a single attribute
        path = AttributePath(EndpointId=self.endpoint, ClusterId=Clusters.Descriptor.id,
                             AttributeId=Clusters.Descriptor.Attributes.ServerList.attribute_id)
        await self.verify_attribute_read([path])

        self.step(2)
        # Read all attributes on a cluster
        path = AttributePath(EndpointId=self.endpoint, ClusterId=Clusters.Descriptor.id, AttributeId=None)
        await self.verify_attribute_read([path])

        self.step(3)
        # Read an attribute from all endpoints
        path = AttributePath(EndpointId=None, ClusterId=Clusters.Descriptor.id,
                             AttributeId=Clusters.Descriptor.Attributes.ServerList.attribute_id)
        await self.verify_attribute_read([path])

        self.step(4)
        # Read a global attribute
        path = AttributePath(
            EndpointId=self.endpoint,
            ClusterId=None,
            AttributeId=global_attribute_ids.GlobalAttributeIds.ATTRIBUTE_LIST_ID)
        await self.verify_attribute_read([path])

        self.step(5)
        read_request = await asyncio.wait_for(
            self.default_controller.Read(self.dut_node_id, [()]),
            timeout=120.0
        )
        self.verify_empty_wildcard([()], read_request)

        self.step(6)
        await self.read_global_attribute_all_endpoints(
            attribute_id=global_attribute_ids.GlobalAttributeIds.ATTRIBUTE_LIST_ID)

        self.step(7)
        await self.read_cluster_all_endpoints(
            cluster=Clusters.Descriptor)

        self.step(8)
        await self.read_endpoint_all_clusters(
            endpoint=self.endpoint)

        self.step(9)
        await self.read_unsupported_endpoint()

        self.step(10)
        await self.read_unsupported_cluster()

        self.step(11)
        await self.read_unsupported_attribute()

        self.step(12)
        await self.read_repeat_attribute(
            endpoint=self.endpoint,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList,
            repeat_count=3)

        self.step(13)
        read_request13, filtered_read13 = await self.read_data_version_filter(
            endpoint=self.endpoint,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList)
        asserts.assert_true(0 in read_request13, "Endpoint 0 missing in first read")
        asserts.assert_true(Clusters.Descriptor in read_request13[0], "Cluster missing in first read")
        asserts.assert_equal(filtered_read13, {}, "Expected empty response with matching data version")

        # Check if BasicInformation cluster exists before running steps 14-17
        # If it doesn't exist (e.g., non-commissionable node), skip these steps
        if Clusters.BasicInformation not in self.endpoints[self.endpoint]:
            log.info("BasicInformation cluster not found on endpoint - skipping steps 14-17")
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)
            self.skip_step(17)
        else:
            self.step(14)
            read_request14, filtered_read14 = await self.read_data_version_filter(
                endpoint=self.endpoint,
                cluster=Clusters.BasicInformation,
                attribute=Clusters.BasicInformation.Attributes.NodeLabel,
                test_value="Hello World")
            if filtered_read14 and 0 in filtered_read14:
                data_version14 = filtered_read14[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
                asserts.assert_equal(filtered_read14[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel],
                                     "Hello World", "Data version does not match expected value")
                asserts.assert_equal((read_request14[0][Clusters.BasicInformation]
                                     [Clusters.Attribute.DataVersion] + 1), data_version14, "DataVersion was not incremented")

            self.step(15)
            # Read all attributes on BasicInformation cluster (no attribute filter)
            read_request15 = await self.default_controller.ReadAttribute(
                self.dut_node_id, [(self.endpoint, Clusters.BasicInformation)])
            data_version15_before = read_request15[self.endpoint][Clusters.BasicInformation][Clusters.Attribute.DataVersion]

            # Write to any attribute to change the data version
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(self.endpoint, Clusters.BasicInformation.Attributes.NodeLabel("Goodbye World"))])

            # Read all attributes again with old data version filter - should return all attributes since version changed
            data_version_filter15 = [(self.endpoint, Clusters.BasicInformation, data_version15_before)]
            filtered_read15 = await self.default_controller.ReadAttribute(
                self.dut_node_id,
                [(self.endpoint, Clusters.BasicInformation)],
                dataVersionFilters=data_version_filter15)

            # Verify we got all attributes back because data version changed
            asserts.assert_in(self.endpoint, filtered_read15, "Endpoint missing in response")
            asserts.assert_in(Clusters.BasicInformation, filtered_read15[self.endpoint], "BasicInformation cluster missing")
            data_version15_after = filtered_read15[self.endpoint][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
            asserts.assert_not_equal(data_version15_before, data_version15_after, "DataVersion should have changed after write")
            asserts.assert_equal(
                filtered_read15[self.endpoint][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel],
                "Goodbye World", "NodeLabel value does not match expected value")

            # Verify that all attributes from the cluster are returned (not just the one we wrote)
            returned_attrs15 = set(filtered_read15[self.endpoint][Clusters.BasicInformation].keys())
            expected_attrs15 = set(read_request15[self.endpoint][Clusters.BasicInformation].keys())
            asserts.assert_equal(returned_attrs15, expected_attrs15,
                                 "All cluster attributes should be returned when data version changed")

            self.step(16)
            read_request16, filtered_read16 = await self.read_multiple_data_version_filters(
                endpoint=self.endpoint,
                cluster=Clusters.BasicInformation,
                attribute=Clusters.BasicInformation.Attributes.NodeLabel,
                test_value="Hello World Again")
            if filtered_read16 and 0 in filtered_read16:
                asserts.assert_equal(filtered_read16[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel],
                                     "Hello World Again", "Data version does not match expected value")

            self.step(17)
            read_a17, read_both17 = await self.read_data_version_filter_multiple_clusters(
                endpoint=self.endpoint,
                cluster=Clusters.Descriptor,
                attribute=Clusters.Descriptor.Attributes.ServerList,
                other_cluster=Clusters.BasicInformation,
                other_attribute=Clusters.BasicInformation.Attributes.NodeLabel)
            asserts.assert_in(0, read_both17, "Endpoint 0 missing in response for step 17")
            asserts.assert_not_in(Clusters.Descriptor, read_both17[0], "Cluster A (Descriptor) should have been filtered out")
            asserts.assert_in(Clusters.BasicInformation, read_both17[0], "Cluster B (BasicInformation) should be present")

        self.step(18)
        await self.read_non_global_attribute_across_all_clusters(
            endpoint=self.endpoint,
            attribute=Clusters.Descriptor.Attributes.ServerList)

        self.step(19)
        await self.read_non_global_attribute_across_all_clusters(
            attribute=Clusters.Descriptor.Attributes.ServerList)

        # Check if BasicInformation cluster exists before running step 20
        # If it doesn't exist (e.g., non-commissionable node), skip this step
        if Clusters.BasicInformation not in self.endpoints[self.endpoint]:
            log.info("BasicInformation cluster not found on endpoint - skipping step 20")
            self.skip_step(20)
        else:
            self.step(20)
            original_acl21, read_request21 = await self.read_limited_access(
                endpoint=self.endpoint,
                cluster_id=Clusters.BasicInformation.id)

            # Verify only BasicInformation cluster is returned (the one we granted access to)
            asserts.assert_true(Clusters.BasicInformation in read_request21.attributes[self.endpoint],
                                "BasicInformation cluster should be present (granted View access)")

            # Verify we got attributes from BasicInformation
            asserts.assert_true(len(read_request21.attributes[self.endpoint][Clusters.BasicInformation]) > 0,
                                "Should have received attributes from BasicInformation cluster")

        self.step(21)
        read_request22 = await self.read_all_events_attributes()
        required_attributes = ["Header", "Status", "Data"]
        for event in read_request22.events:
            for attr in required_attributes:
                asserts.assert_true(hasattr(event, attr), f"{attr} not in event")


if __name__ == "__main__":
    default_matter_test_main()
