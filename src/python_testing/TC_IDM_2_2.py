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
import copy
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects as ClusterObjects
from matter.clusters.Attribute import AttributePath
from matter.exceptions import ChipStackError
from matter.interaction_model import InteractionModelError, Status
from matter.testing import global_attribute_ids
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

log = logging.getLogger(__name__)


class TC_IDM_2_2(MatterBaseTest, BasicCompositionTests):
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

    # === Attribute Reading and Verification ===
    async def verify_attribute_read(self, attribute_path: list) -> dict:
        """Read and verify attributes from the device.

        Args:
            attribute_path: List of attribute paths to read (should be AttributePath objects or tuples like (endpoint, cluster, attribute))

        Returns:
            Dictionary containing the read results
        """
        read_response = await self.default_controller.Read(
            self.dut_node_id,
            attribute_path)
        self.verify_attribute_path(read_response, attribute_path[0])
        return read_response

    def verify_all_endpoints_clusters(self, read_response: dict):
        """Verify read response for a full wildcard read (all attributes from all clusters on all endpoints).

        This method performs comprehensive verification that only makes sense for wildcard reads
        where you have complete data for all endpoints, clusters, and attributes.

        Args:
            read_response: The read response from a wildcard read to verify

        Raises:
            AssertionError if verification fails
        """
        # Verify that we got data for all expected endpoints based on EP0's parts list
        # The parts list on EP0 contains all other endpoints, so total endpoints = parts_list + [0]
        if 0 in read_response.tlvAttributes:
            parts_list = read_response.tlvAttributes[0][Clusters.Descriptor.id][
                Clusters.Descriptor.Attributes.PartsList.attribute_id]
            expected_endpoints = sorted(parts_list + [0])  # parts list + endpoint 0 itself
            actual_endpoints = sorted(read_response.tlvAttributes.keys())
            asserts.assert_equal(actual_endpoints, expected_endpoints,
                                 f"Read response endpoints {actual_endpoints} don't match expected {expected_endpoints}")

        for endpoint in read_response.tlvAttributes:
            asserts.assert_in(
                Clusters.Descriptor.Attributes.PartsList.attribute_id,
                read_response.tlvAttributes[endpoint][Clusters.Descriptor.id],
                "PartsList attribute should always be present")

            # Server list matches returned clusters
            returned_clusters = sorted(read_response.tlvAttributes[endpoint].keys())
            server_list = sorted(read_response.tlvAttributes[endpoint][Clusters.Descriptor.id][
                Clusters.Descriptor.Attributes.ServerList.attribute_id])
            asserts.assert_equal(returned_clusters, server_list,
                                 f"Cluster list and server list do not match for endpoint {endpoint}")

            # Attribute lists
            for cluster in read_response.tlvAttributes[endpoint]:
                # Only verify spec-defined clusters to avoid issues with clusters that may have write-only attributes
                # Note: Comprehensive verification of wildcard reads is also performed in IDM-10.1 and ACE tests
                if global_attribute_ids.cluster_id_type(cluster) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                returned_attrs = sorted(read_response.tlvAttributes[endpoint][cluster].keys())
                attr_list = sorted(read_response.tlvAttributes[endpoint][cluster][
                    ClusterObjects.ALL_CLUSTERS[cluster].Attributes.AttributeList.attribute_id])
                asserts.assert_equal(returned_attrs, attr_list,
                                     f"Mismatch for {cluster} at endpoint {endpoint}")

    def verify_attribute_path(self, read_response: dict, path: AttributePath):
        """Verify read response for an attribute path.

        Args:
            read_response: The read response to verify
            path: The attribute path that was read

        Raises:
            AssertionError if verification fails
        """
        endpoint = path.EndpointId
        cluster_id = path.ClusterId
        attribute_id = path.AttributeId

        endpoint_list = [endpoint] if endpoint is not None else list(self.endpoints.keys())

        for ep in endpoint_list:
            asserts.assert_in(ep, read_response.tlvAttributes,
                              f"Endpoint {ep} not found in response")

            if cluster_id is not None:
                asserts.assert_in(cluster_id, read_response.tlvAttributes[ep],
                                  f"Cluster {cluster_id} not found in endpoint {ep}")

                if attribute_id is not None:
                    asserts.assert_in(attribute_id, read_response.tlvAttributes[ep][cluster_id],
                                      f"Attribute {attribute_id} not found in cluster {cluster_id} on endpoint {ep}")
                else:
                    # All attributes from the cluster were requested
                    # Verify AttributeList is present
                    cluster_obj = ClusterObjects.ALL_CLUSTERS.get(cluster_id)
                    if cluster_obj and hasattr(cluster_obj.Attributes, 'AttributeList'):
                        attr_list_id = cluster_obj.Attributes.AttributeList.attribute_id
                        asserts.assert_in(attr_list_id, read_response.tlvAttributes[ep][cluster_id],
                                          f"AttributeList not found in cluster {cluster_id} on endpoint {ep}")

                        # Verify that returned attributes match the AttributeList
                        # Extra assertion to ensure cluster_id exists before accessing (defense in depth)
                        asserts.assert_in(cluster_id, read_response.tlvAttributes[ep],
                                          f"Cluster {cluster_id} not found in endpoint {ep}")
                        returned_attrs = sorted(read_response.tlvAttributes[ep][cluster_id].keys())
                        attr_list = sorted(read_response.tlvAttributes[ep][cluster_id][
                            ClusterObjects.ALL_CLUSTERS[cluster_id].Attributes.AttributeList.attribute_id])
                        asserts.assert_equal(
                            returned_attrs,
                            attr_list,
                            f"Returned attributes don't match AttributeList for cluster {cluster_id} on endpoint {ep}")
            else:
                # For global attributes, we expect them to be present across all clusters
                if attribute_id is not None:
                    # Reading a specific global attribute - verify it exists in all clusters
                    for cluster in read_response.tlvAttributes[ep].values():
                        asserts.assert_in(attribute_id, cluster,
                                          f"Global attribute {attribute_id} not found in cluster on endpoint {ep}")

    async def _read_global_attribute_all_endpoints(self, attribute_id):
        attribute_path = AttributePath(
            EndpointId=None,
            ClusterId=None,
            AttributeId=attribute_id)
        return await self.verify_attribute_read([attribute_path])

    async def _read_cluster_all_endpoints(self, cluster):
        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [cluster])

        # Verify all expected endpoints are returned
        expected_endpoints = list(self.endpoints.keys())
        returned_endpoints = list(read_request.keys())
        asserts.assert_equal(sorted(returned_endpoints), sorted(expected_endpoints),
                             f"Expected endpoints {expected_endpoints} but got {returned_endpoints}")

        for endpoint in read_request:
            asserts.assert_in(cluster, read_request[endpoint].keys(),
                              f"{cluster} cluster not in output")
            asserts.assert_in(cluster.Attributes.AttributeList,
                              read_request[endpoint][cluster],
                              "AttributeList not in output")
            # Verify that returned attributes match the AttributeList
            # DataVersion is excluded as it is metadata and not a real attribute
            if global_attribute_ids.cluster_id_type(cluster.id) == global_attribute_ids.ClusterIdType.kStandard:
                returned_attrs = sorted([x.attribute_id for x in read_request[endpoint][cluster]
                                         if x != Clusters.Attribute.DataVersion])
                attr_list = sorted(read_request[endpoint][cluster][cluster.Attributes.AttributeList])
                asserts.assert_equal(
                    returned_attrs,
                    attr_list,
                    f"Returned attributes don't match AttributeList for cluster {cluster.id} on endpoint {endpoint}")
        return read_request

    async def _read_endpoint_all_clusters(self, endpoint):
        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [endpoint])
        asserts.assert_in(Clusters.Descriptor, read_request[endpoint].keys(), "Descriptor cluster not in output")
        asserts.assert_in(Clusters.Descriptor.Attributes.ServerList,
                          read_request[endpoint][Clusters.Descriptor], "ServerList not in output")

        # Verify that returned clusters match the ServerList
        returned_cluster_ids = sorted([cluster.id for cluster in read_request[endpoint]])
        server_list = sorted(read_request[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList])
        asserts.assert_equal(
            returned_cluster_ids,
            server_list,
            f"Returned cluster IDs {returned_cluster_ids} don't match ServerList {server_list} for endpoint {endpoint}")

        for cluster in read_request[endpoint]:
            attribute_ids = [a.attribute_id for a in read_request[endpoint][cluster]
                             if a != Clusters.Attribute.DataVersion]
            asserts.assert_equal(
                sorted(attribute_ids),
                sorted(read_request[endpoint][cluster][cluster.Attributes.AttributeList]),
                f"Expected attribute list does not match for cluster {cluster}"
            )
        return read_request

    async def _read_unsupported_endpoint(self):
        """
        Find unsupported endpoint and attempt to read from it
        """
        supported_endpoints = set(self.endpoints.keys())
        all_endpoints = set(range(max(supported_endpoints) + 2))
        unsupported = list(all_endpoints - supported_endpoints)
        await self.read_single_attribute_expect_error(endpoint=unsupported[0], cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.FeatureMap, error=Status.UnsupportedEndpoint)

    async def _read_unsupported_cluster(self):
        """
        Find a standard cluster that's not supported on any endpoint and try to read from it.
        """
        # Get all standard clusters supported on all endpoints
        supported_cluster_ids = set()
        for endpoint_clusters in self.endpoints.values():
            supported_cluster_ids.update({
                cluster.id for cluster in endpoint_clusters
                if global_attribute_ids.cluster_id_type(cluster.id) == global_attribute_ids.ClusterIdType.kStandard
            })

        # Get all possible standard clusters
        all_standard_cluster_ids = {
            cluster_id for cluster_id in ClusterObjects.ALL_CLUSTERS
            if global_attribute_ids.cluster_id_type(cluster_id) == global_attribute_ids.ClusterIdType.kStandard
        }

        # Find unsupported clusters
        unsupported_cluster_ids = all_standard_cluster_ids - supported_cluster_ids

        # If no unsupported clusters are found, skip this test step
        if not unsupported_cluster_ids:
            self.skip_step("No unsupported standard clusters found to test")
            return

        # Use the first unsupported cluster
        unsupported_cluster_id = next(iter(unsupported_cluster_ids))
        unsupported_cluster = ClusterObjects.ALL_CLUSTERS[unsupported_cluster_id]

        # Get any attribute from this cluster
        cluster_attributes = ClusterObjects.ALL_ATTRIBUTES[unsupported_cluster_id]
        test_attribute = next(iter(cluster_attributes.values()))

        # Test the unsupported cluster on all available endpoints
        # It should return UnsupportedCluster error from all endpoints
        for endpoint_id in self.endpoints:
            result = await self.read_single_attribute_expect_error(
                endpoint=endpoint_id,
                cluster=unsupported_cluster,
                attribute=test_attribute,
                error=Status.UnsupportedCluster)
            asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                msg=f"Unexpected success reading invalid cluster on endpoint {endpoint_id}")
            log.info(f"Confirmed unsupported cluster {unsupported_cluster_id} returns error on endpoint {endpoint_id}")

    async def _read_unsupported_attribute(self):
        """
        Attempts to read an unsupported attribute from a supported cluster on any endpoint.
        Requires an UNSUPPORTED_ATTRIBUTE error from the DUT.
        """
        for endpoint_id, endpoint in self.endpoints.items():
            for cluster_type, cluster in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                all_attrs = set(ClusterObjects.ALL_ATTRIBUTES[cluster_type.id].keys())
                dut_attrs = set(cluster[cluster_type.Attributes.AttributeList])

                unsupported = [
                    attr_id for attr_id in (all_attrs - dut_attrs)
                    if global_attribute_ids.attribute_id_type(attr_id) == global_attribute_ids.AttributeIdType.kStandardNonGlobal
                ]
                if unsupported:
                    unsupported_attr = ClusterObjects.ALL_ATTRIBUTES[cluster_type.id][unsupported[0]]
                    log.info(
                        f"Testing unsupported attribute: endpoint={endpoint_id}, cluster={cluster_type}, attribute={unsupported_attr}")
                    # Only request this single attribute
                    result = await self.read_single_attribute_expect_error(
                        endpoint=endpoint_id,
                        cluster=cluster_type,
                        attribute=unsupported_attr,
                        error=Status.UnsupportedAttribute
                    )
                    asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                        msg="Unexpected success reading invalid attribute")
                    log.info(f"Confirmed unsupported attribute {unsupported_attr} returns error on endpoint {endpoint_id}")
                    return

        # If we get here, we got problems as there should always be at least one unsupported attribute
        asserts.fail("No unsupported attributes found we must find at least one unsupported attribute")

    async def _read_repeat_attribute(self, endpoint, cluster, attribute, repeat_count):
        results = []
        for i in range(repeat_count):
            path = AttributePath(EndpointId=endpoint, ClusterId=cluster.id, AttributeId=attribute.attribute_id)
            result = await self.verify_attribute_read([path])
            results.append(result)

        # Verify all reads returned consistent values
        if len(results) > 1:
            first_result = results[0]
            for i, result in enumerate(results[1:], 2):
                # Compare the attribute values from each read
                first_attr_value = first_result.tlvAttributes[endpoint][cluster.id][attribute.attribute_id]
                current_attr_value = result.tlvAttributes[endpoint][cluster.id][attribute.attribute_id]
                asserts.assert_equal(first_attr_value, current_attr_value,
                                     f"Read {i} returned different value than first read")

        log.info(f"Successfully completed {repeat_count} consistent reads of {attribute}")
        return results

    async def _read_data_version_filter(self, endpoint, cluster, attribute, test_value=None):
        read_request = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(endpoint, cluster, attribute)])
        data_version = read_request[0][cluster][Clusters.Attribute.DataVersion]
        if test_value is not None:
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(endpoint, attribute(value=test_value))])
        data_version_filter = [(endpoint, cluster, data_version)]
        filtered_read = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(endpoint, cluster, attribute)],
            dataVersionFilters=data_version_filter)
        return read_request, filtered_read

    async def _read_non_global_attribute_across_all_clusters(self, endpoint=None, attribute=None):
        attribute_path = AttributePath(
            EndpointId=endpoint,
            ClusterId=None,
            AttributeId=attribute.attribute_id)
        try:
            await self.default_controller.ReadAttribute(
                self.dut_node_id,
                [attribute_path]
            )
            asserts.fail("Expected INVALID_ACTION error but operation succeeded")
        except ChipStackError as e:  # chipstack-ok
            # Spec lists INVALID_ACTION as 0x80, but the stack surfaces it wrapped as 0x580
            # (General error + 0x80). Asserting 0x580 here to match the actual returned error.
            # Ref: https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/data_model/Interaction-Model.adoc#101-status-code-table
            asserts.assert_equal(e.err, 0x580,
                                 "Incorrect error response for reading non-global attribute on all clusters at endpoint, should have returned GENERAL_ERROR + INVALID_ACTION")
            return

    async def _read_limited_access(self, endpoint, cluster_id):
        """Test reading all attributes from all clusters at an endpoint with limited access.

        Creates a second controller (TH2) with limited access to only one cluster, then reads
        all attributes from all clusters at the endpoint. Verifies that only the allowed cluster
        is returned and no errors are sent for clusters without access.

        Args:
            endpoint: The endpoint to test
            cluster_id: The cluster ID to grant access to
        """
        # Creates a second controller (TH2)
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        TH2_nodeid = self.matter_test_config.controller_node_id + 1
        TH2 = fabric_admin.NewController(nodeId=TH2_nodeid)

        # Read and save the original ACL using the default (admin) controller
        read_acl = await self.default_controller.Read(
            self.dut_node_id,
            [(self.endpoint, Clusters.AccessControl.Attributes.Acl)])
        dut_acl_original = read_acl.attributes[self.endpoint][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

        try:
            # Create an ACE that grants View access to TH2 for only ONE specific cluster
            ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=cluster_id)],
                subjects=[TH2_nodeid])
            dut_acl = copy.deepcopy(dut_acl_original)
            dut_acl.append(ace)

            # Write the modified ACL to grant TH2 limited access
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(endpoint, Clusters.AccessControl.Attributes.Acl(dut_acl))])
            log.info(f"Granted TH2 View access to only cluster {cluster_id}")

            # Use TH2 to read ALL attributes from ALL clusters at the endpoint
            read_request = await TH2.Read(
                self.dut_node_id,
                [(endpoint)])

            # Verify the endpoint is in the response
            asserts.assert_in(endpoint, read_request.attributes,
                              f"Endpoint {endpoint} not found in response - may not exist or have no accessible clusters")

            # Verify only the allowed cluster is returned
            returned_clusters = list(read_request.attributes[endpoint].keys())
            log.info(f"Clusters returned with limited access (TH2): {[c.id for c in returned_clusters]}")

            # The allowed cluster should be present
            allowed_cluster_obj = None
            for cluster_obj in returned_clusters:
                if cluster_obj.id == cluster_id:
                    allowed_cluster_obj = cluster_obj
                    break
            asserts.assert_is_not_none(allowed_cluster_obj,
                                       f"Expected cluster {cluster_id} (allowed) to be present in response")

            for cluster_obj in returned_clusters:
                if cluster_obj.id != cluster_id:
                    asserts.fail(f"Unexpected cluster {cluster_obj.id} returned - should only get allowed cluster {cluster_id}")

            return dut_acl_original, read_request

        finally:
            # Ensure cleanup happens even if an exception occurs
            # Restore original ACL
            try:
                await self.default_controller.WriteAttribute(
                    self.dut_node_id,
                    [(self.endpoint, Clusters.AccessControl.Attributes.Acl(dut_acl_original))])
                log.info("Restored original ACL")
            except Exception as e:
                log.error(f"Failed to restore original ACL: {e}")

            # Removes TH2 controller
            TH2.Shutdown()

    async def _read_all_events_attributes(self):
        return await self.default_controller.Read(nodeId=self.dut_node_id, attributes=[()], events=[()])

    async def _read_data_version_filter_multiple_clusters(self, endpoint, cluster, attribute, other_cluster, other_attribute):
        read_a = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(endpoint, cluster, attribute)])
        data_version_a = read_a[0][cluster][Clusters.Attribute.DataVersion]
        data_version_filter_a = [(endpoint, cluster, data_version_a)]
        read_both = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(endpoint, cluster, attribute),
             (endpoint, other_cluster, other_attribute)],
            dataVersionFilters=data_version_filter_a)
        return read_a, read_both

    async def _read_multiple_data_version_filters(self, endpoint, cluster, attribute, test_value=None):
        # First read to get the old data version
        read_request = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(endpoint, cluster, attribute)])
        data_version_old = read_request[0][cluster][Clusters.Attribute.DataVersion]

        # Write to change the data version
        if test_value is not None:
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(endpoint, attribute(value=test_value))])

        # Second read to get the new (correct) data version after write
        read_after_write = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(endpoint, cluster, attribute)])
        data_version_new = read_after_write[0][cluster][Clusters.Attribute.DataVersion]

        # Create filters with BOTH the correct (new) version AND the older version
        data_version_filters = [
            (endpoint, cluster, data_version_new),  # Correct/current version
            (endpoint, cluster, data_version_old)   # Older version
        ]

        # Read with both filters
        filtered_read = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(endpoint, cluster, attribute)],
            dataVersionFilters=data_version_filters)

        return read_request, filtered_read

    def _verify_empty_wildcard(self, attr_path, read_request):
        """Verify read response for empty tuple path (all attributes from all clusters on all endpoints).

        This method is based on Austin's implementation in PR #34003.

        Args:
            attr_path: The attribute path that was read
            read_request: The read request response to verify

        Raises:
            AssertionError if verification fails
        """
        # Parts list validation
        parts_list_a = read_request.tlvAttributes[0][Clusters.Descriptor.id][Clusters.Descriptor.Attributes.PartsList.attribute_id]
        parts_list_b = self.endpoints[0][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]
        asserts.assert_equal(parts_list_a, parts_list_b, "Parts list is not the expected value")

        # Server list validation
        for endpoint in read_request.tlvAttributes:
            returned_clusters = sorted(read_request.tlvAttributes[endpoint].keys())
            server_list = sorted(read_request.tlvAttributes[endpoint][Clusters.Descriptor.id]
                                 [Clusters.Descriptor.Attributes.ServerList.attribute_id])
            asserts.assert_equal(returned_clusters, server_list)

        # Verify all endpoints and clusters
        self.verify_all_endpoints_clusters(read_request)

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
        self._verify_empty_wildcard([()], read_request)

        self.step(6)
        await self._read_global_attribute_all_endpoints(
            attribute_id=global_attribute_ids.GlobalAttributeIds.ATTRIBUTE_LIST_ID)

        self.step(7)
        await self._read_cluster_all_endpoints(
            cluster=Clusters.Descriptor)

        self.step(8)
        await self._read_endpoint_all_clusters(
            endpoint=self.endpoint)

        self.step(9)
        await self._read_unsupported_endpoint()

        self.step(10)
        await self._read_unsupported_cluster()

        self.step(11)
        await self._read_unsupported_attribute()

        self.step(12)
        await self._read_repeat_attribute(
            endpoint=self.endpoint,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList,
            repeat_count=3)

        self.step(13)
        read_request13, filtered_read13 = await self._read_data_version_filter(
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
            read_request14, filtered_read14 = await self._read_data_version_filter(
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
            read_request16, filtered_read16 = await self._read_multiple_data_version_filters(
                endpoint=self.endpoint,
                cluster=Clusters.BasicInformation,
                attribute=Clusters.BasicInformation.Attributes.NodeLabel,
                test_value="Hello World Again")
            if filtered_read16 and 0 in filtered_read16:
                asserts.assert_equal(filtered_read16[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel],
                                     "Hello World Again", "Data version does not match expected value")

            self.step(17)
            read_a17, read_both17 = await self._read_data_version_filter_multiple_clusters(
                endpoint=self.endpoint,
                cluster=Clusters.Descriptor,
                attribute=Clusters.Descriptor.Attributes.ServerList,
                other_cluster=Clusters.BasicInformation,
                other_attribute=Clusters.BasicInformation.Attributes.NodeLabel)
            asserts.assert_in(0, read_both17, "Endpoint 0 missing in response for step 17")
            asserts.assert_not_in(Clusters.Descriptor, read_both17[0], "Cluster A (Descriptor) should have been filtered out")
            asserts.assert_in(Clusters.BasicInformation, read_both17[0], "Cluster B (BasicInformation) should be present")

        self.step(18)
        await self._read_non_global_attribute_across_all_clusters(
            endpoint=self.endpoint,
            attribute=Clusters.Descriptor.Attributes.ServerList)

        self.step(19)
        await self._read_non_global_attribute_across_all_clusters(
            attribute=Clusters.Descriptor.Attributes.ServerList)

        # Check if BasicInformation cluster exists before running step 20
        # If it doesn't exist (e.g., non-commissionable node), skip this step
        if Clusters.BasicInformation not in self.endpoints[self.endpoint]:
            log.info("BasicInformation cluster not found on endpoint - skipping step 20")
            self.skip_step(20)
        else:
            self.step(20)
            original_acl21, read_request21 = await self._read_limited_access(
                endpoint=self.endpoint,
                cluster_id=Clusters.BasicInformation.id)

            # Verify only BasicInformation cluster is returned (the one we granted access to)
            asserts.assert_true(Clusters.BasicInformation in read_request21.attributes[self.endpoint],
                                "BasicInformation cluster should be present (granted View access)")

            # Verify we got attributes from BasicInformation
            asserts.assert_true(len(read_request21.attributes[self.endpoint][Clusters.BasicInformation]) > 0,
                                "Should have received attributes from BasicInformation cluster")

        self.step(21)
        read_request22 = await self._read_all_events_attributes()
        required_attributes = ["Header", "Status", "Data"]
        for event in read_request22.events:
            for attr in required_attributes:
                asserts.assert_true(hasattr(event, attr), f"{attr} not in event")


if __name__ == "__main__":
    default_matter_test_main()
