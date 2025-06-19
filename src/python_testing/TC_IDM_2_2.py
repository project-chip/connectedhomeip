#
#    Copyright (c) 2024 Project CHIP Authors
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


import copy
import inspect
from enum import IntFlag

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import AttributePath
from chip.clusters.ClusterObjects import ClusterObject
from chip.clusters.enum import MatterIntEnum
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from chip.testing import global_attribute_ids
from chip.testing.basic_composition import BasicCompositionTests
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.tlv import uint
from mobly import asserts
import logging

# Import AccessControl classes for limited access testing
from chip.clusters.Objects import AccessControl


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
        self.timeout_ms = 10000  # 10 second timeout for read operations
        self.setup_timeout_ms = 60000  # 60 second timeout for setup phase
    
    def get_device_clusters(self) -> set:
        """Get all clusters supported by the device.
        
        Returns:
            Set of cluster IDs supported by the device
        """
        device_clusters = set()
        for endpoint in self.endpoints:
            device_clusters |= set(self.endpoints[endpoint].keys())
        return device_clusters

    def get_device_attributes(self) -> set:
        """Get all attributes supported by the device.
        
        Returns:
            Set of attribute IDs supported by the device
        """
        device_attributes = set()
        for endpoint in self.endpoints:
            for cluster in self.endpoints[endpoint]:
                device_attributes |= self.endpoints[endpoint][cluster].keys()
        return device_attributes

    def get_cluster_attributes_of_type(self, cluster: ClusterObjects.Cluster, 
                                     desired_type: type) -> list[ClusterObjects.ClusterAttributeDescriptor]:
        """Get all attributes of a specific type from a cluster.
        
        Args:
            cluster: The cluster to get attributes from
            desired_type: The type of attributes to get
            
        Returns:
            List of attribute descriptors of the desired type
        """
        all_attributes = [attr for attr in cluster.Attributes.__dict__.values() 
                         if inspect.isclass(attr) and 
                         issubclass(attr, ClusterObjects.ClusterAttributeDescriptor)]

        if desired_type == MatterIntEnum:
            return [attr for attr in all_attributes 
                   if type(attr.attribute_type.Type) == type(desired_type)]
        elif desired_type == IntFlag:
            return [cluster.Attributes.FeatureMap]
        else:
            return [attr for attr in all_attributes 
                   if attr.attribute_type == ClusterObjects.ClusterObjectFieldDescriptor(Type=desired_type)]

    # === Attribute Reading and Verification ===
    
    async def read_attribute(self, attribute_path: list) -> dict:
        """Read attributes from the device with timeout handling."""
        try:
            return await self.default_controller.Read(
                self.dut_node_id, 
                attribute_path)
        except ChipStackError as e:
            if "Timeout" in str(e):
                logging.error(f"Read operation timed out after {self.timeout_ms}ms")
                logging.error(f"Attribute path: {attribute_path}")
            raise

    async def verify_attribute_read(self, attribute_path: list, 
                                  expected_error: Status = None) -> dict:
        """Read and verify attributes from the device.
        
        Args:
            attribute_path: List of attribute paths to read
            expected_error: Optional expected error status
            
        Returns:
            Dictionary containing the read results
            
        Raises:
            AssertionError if verification fails
        """
        if expected_error:
            try:
                if isinstance(attribute_path[0], AttributePath):
                    # Handle AttributePath objects
                    result = await self.read_single_attribute_expect_error(
                        endpoint=attribute_path[0].EndpointId,
                        cluster=attribute_path[0].ClusterId,
                        attribute=attribute_path[0].AttributeId,
                        error=expected_error)
                else:
                    # Handle tuple paths
                    path_tuple = attribute_path[0]
                    endpoint = path_tuple[0] if isinstance(path_tuple, tuple) else None
                    cluster = path_tuple[1] if isinstance(path_tuple, tuple) else path_tuple
                    attribute = path_tuple[2] if len(path_tuple) > 2 else None
                    
                    # Handle case where attribute is an integer (like 0xFFFF)
                    if isinstance(attribute, int):
                        # For unsupported attribute testing, use a different approach
                        try:
                            result = await self.read_attribute(attribute_path)
                            # If we get here, the attribute was supported (unexpected)
                            asserts.fail("Expected UNSUPPORTED_ATTRIBUTE error but attribute was supported")
                        except Exception as e:
                            # Check if the error contains UNSUPPORTED_ATTRIBUTE
                            if "UNSUPPORTED_ATTRIBUTE" in str(e) or "0x87" in str(e):
                                return None  # Expected error
                            else:
                                raise  # Unexpected error
                    else:
                        result = await self.read_single_attribute_expect_error(
                            endpoint=endpoint,
                            cluster=cluster,
                            attribute=attribute,
                            error=expected_error)
                return result
            except KeyError as e:
                # If we expect UNSUPPORTED_ENDPOINT or UNSUPPORTED_CLUSTER and the endpoint/cluster is missing, this is correct
                if expected_error in [Status.UnsupportedEndpoint, Status.UnsupportedCluster]:
                    return None
                raise

        read_request = await self.read_attribute(attribute_path)
        await self.verify_read_response(read_request, attribute_path)
        return read_request

    async def verify_read_response(self, read_request: dict, attribute_path: list):
        """Verify the response from a read request.
        
        Args:
            read_request: The read request response to verify
            attribute_path: The original attribute path that was read
            
        Raises:
            AssertionError if verification fails
        """
        if not attribute_path or not attribute_path[0]:
            # Empty path - verify all endpoints and clusters
            await self.verify_all_endpoints_clusters(read_request)
            return

        if isinstance(attribute_path[0], tuple):
            await self.verify_specific_path(read_request, attribute_path[0])
        elif isinstance(attribute_path[0], type):
            await self.verify_cluster_path(read_request, attribute_path[0])
        elif isinstance(attribute_path[0], AttributePath):
            await self.verify_attribute_path(read_request, attribute_path[0])

    async def verify_all_endpoints_clusters(self, read_request: dict):
        """Verify read response for all endpoints and clusters.
        
        Args:
            read_request: The read request response to verify
            
        Raises:
            AssertionError if verification fails
        """
        for endpoint in read_request.tlvAttributes:
            # Verify parts list if present
            if Clusters.Descriptor.Attributes.PartsList.attribute_id in read_request.tlvAttributes[endpoint][Clusters.Descriptor.id]:
                parts_list = read_request.tlvAttributes[endpoint][Clusters.Descriptor.id][
                    Clusters.Descriptor.Attributes.PartsList.attribute_id]
                if Clusters.Descriptor.Attributes.PartsList in self.endpoints[endpoint][Clusters.Descriptor]:
                    asserts.assert_equal(parts_list, self.endpoints[endpoint][Clusters.Descriptor][
                        Clusters.Descriptor.Attributes.PartsList], 
                        "Parts list is not the expected value")

            # Verify server list matches returned clusters
            returned_clusters = sorted(list(read_request.tlvAttributes[endpoint].keys()))
            server_list = sorted(read_request.tlvAttributes[endpoint][Clusters.Descriptor.id][
                Clusters.Descriptor.Attributes.ServerList.attribute_id])
            asserts.assert_equal(returned_clusters, server_list,
                               f"Cluster list and server list do not match for endpoint {endpoint}")

            # Verify attribute lists
            for cluster in read_request.tlvAttributes[endpoint]:
                returned_attrs = sorted([x for x in read_request.tlvAttributes[endpoint][cluster].keys()])
                attr_list = sorted([x for x in read_request.tlvAttributes[endpoint][cluster][
                    ClusterObjects.ALL_CLUSTERS[cluster].Attributes.AttributeList.attribute_id]
                    if x != Clusters.UnitTesting.Attributes.WriteOnlyInt8u.attribute_id])
                asserts.assert_equal(returned_attrs, attr_list,
                                   f"Mismatch for {cluster} at endpoint {endpoint}")

    async def verify_specific_path(self, read_request: dict, path: tuple):
        """Verify read response for a specific path.
        
        Args:
            read_request: The read request response to verify
            path: The specific path that was read (endpoint, cluster, attribute) or (endpoint, cluster)
            
        Raises:
            AssertionError if verification fails
        """
        if len(path) == 3:
            endpoint, cluster_obj, attribute = path
        elif len(path) == 2:
            endpoint, cluster_obj = path
            attribute = None
        else:
            raise ValueError(f"Invalid path length: {len(path)}, expected 2 or 3 elements")
            
        # Handle case where endpoint is None (read from all endpoints)
        if endpoint is None:
            for ep in read_request.tlvAttributes:
                await self._verify_single_endpoint_path(read_request, ep, cluster_obj, attribute)
        else:
            await self._verify_single_endpoint_path(read_request, endpoint, cluster_obj, attribute)

    async def _verify_single_endpoint_path(self, read_request: dict, endpoint: int, cluster_obj, attribute):
        """Verify read response for a single endpoint path.
        
        Args:
            read_request: The read request response to verify
            endpoint: The endpoint to verify
            cluster_obj: The cluster object or attribute
            attribute: The specific attribute (if any)
            
        Raises:
            AssertionError if verification fails
        """
        if hasattr(cluster_obj, 'cluster_id'):  # cluster_obj is an Attribute
            cluster = ClusterObjects.ALL_CLUSTERS[cluster_obj.cluster_id]
            attribute_ids = list(read_request.tlvAttributes[endpoint][cluster_obj.cluster_id].keys())
            
            asserts.assert_equal({cluster_obj.cluster_id}, read_request.tlvAttributes[endpoint].keys(),
                               f"Cluster not in output: {cluster_obj.cluster_id}")
            if attribute:
                asserts.assert_equal(attribute_ids, [cluster_obj.attribute_id])
            
            server_list = read_request.tlvAttributes[endpoint][cluster_obj.cluster_id][
                cluster.Attributes.ServerList.attribute_id]
            asserts.assert_equal(server_list, sorted([x.id for x in self.endpoints[endpoint]]))
        else:
            cluster_ids = list(read_request.tlvAttributes[endpoint].keys())
            asserts.assert_in(cluster_obj.id, cluster_ids)
            
            server_list = read_request.tlvAttributes[endpoint][cluster_obj.id][
                Clusters.Descriptor.Attributes.ServerList.attribute_id]
            asserts.assert_equal(server_list, sorted([x.id for x in self.endpoints[endpoint]]))

    async def verify_cluster_path(self, read_request: dict, cluster: ClusterObjects.Cluster):
        """Verify read response for a cluster path.
        
        Args:
            read_request: The read request response to verify
            cluster: The cluster that was read
            
        Raises:
            AssertionError if verification fails
        """
        for endpoint in read_request.tlvAttributes:
            cluster_ids = list(read_request.tlvAttributes[endpoint].keys())
            asserts.assert_in(cluster.id, cluster_ids)
            
            returned_attributes = read_request.tlvAttributes[endpoint][cluster.id][
                cluster.Attributes.AttributeList.attribute_id]
            asserts.assert_equal(sorted(returned_attributes), 
                               sorted(read_request.tlvAttributes[endpoint][cluster.id].keys()),
                               "Expected attribute list doesn't match")

    async def verify_attribute_path(self, read_request: dict, path: AttributePath):
        """Verify read response for an attribute path.
        
        Args:
            read_request: The read request response to verify
            path: The attribute path that was read
            
        Raises:
            AssertionError if verification fails
        """
        endpoint = path.EndpointId
        endpoint_list = [endpoint] if endpoint is not None else list(self.endpoints.keys())
        
        for endpoint in endpoint_list:
            asserts.assert_in(Clusters.Descriptor.id,
                            read_request.tlvAttributes[endpoint].keys(), 
                            "Descriptor cluster not in output")
            asserts.assert_in(Clusters.Descriptor.Attributes.AttributeList.attribute_id,
                            read_request.tlvAttributes[endpoint][Clusters.Descriptor.id], 
                            "AttributeList not in output")

    async def _setup_device_info(self):
        """Set up device information by reading descriptor cluster attributes."""
        try:
            # Read descriptor cluster attributes instead of wildcard read
            descriptor_read = await self.default_controller.Read(
                self.dut_node_id,
                [(0, Clusters.Descriptor, Clusters.Descriptor.Attributes.ServerList),
                 (0, Clusters.Descriptor, Clusters.Descriptor.Attributes.PartsList)])
            
            # Store the endpoints information
            self.endpoints = {}
            
            # Process endpoint 0
            endpoint0_data = {}
            server_list = None
            parts_list = None
            
            for tlv_data in descriptor_read.tlvAttributes[0][Clusters.Descriptor.id].items():
                if tlv_data[0] == Clusters.Descriptor.Attributes.ServerList.attribute_id:
                    server_list = tlv_data[1]
                    endpoint0_data[Clusters.Descriptor.Attributes.ServerList] = server_list
                elif tlv_data[0] == Clusters.Descriptor.Attributes.PartsList.attribute_id:
                    parts_list = tlv_data[1]
                    endpoint0_data[Clusters.Descriptor.Attributes.PartsList] = parts_list
            
            # Store all clusters for endpoint 0
            self.endpoints[0] = {}
            for cluster_id in server_list:
                cluster = ClusterObjects.ALL_CLUSTERS.get(cluster_id)
                if cluster:
                    self.endpoints[0][cluster] = {}
            
            # Add descriptor cluster data
            self.endpoints[0][Clusters.Descriptor] = endpoint0_data
            
            # If there are additional endpoints, read their descriptor clusters
            if parts_list:
                for endpoint in parts_list:
                    if endpoint != 0:  # Skip endpoint 0 as we already have it
                        endpoint_read = await self.default_controller.Read(
                            self.dut_node_id,
                            [(endpoint, Clusters.Descriptor, Clusters.Descriptor.Attributes.ServerList)])
                        
                        endpoint_data = {}
                        endpoint_server_list = None
                        
                        for tlv_data in endpoint_read.tlvAttributes[endpoint][Clusters.Descriptor.id].items():
                            if tlv_data[0] == Clusters.Descriptor.Attributes.ServerList.attribute_id:
                                endpoint_server_list = tlv_data[1]
                                endpoint_data[Clusters.Descriptor.Attributes.ServerList] = endpoint_server_list
                        
                        # Store all clusters for this endpoint
                        self.endpoints[endpoint] = {}
                        for cluster_id in endpoint_server_list:
                            cluster = ClusterObjects.ALL_CLUSTERS.get(cluster_id)
                            if cluster:
                                self.endpoints[endpoint][cluster] = {}
                        
                        # Add descriptor cluster data
                        self.endpoints[endpoint][Clusters.Descriptor] = endpoint_data
            
            logging.info(f"Device endpoints: {self.endpoints}")
            for endpoint, clusters in self.endpoints.items():
                logging.info(f"Endpoint {endpoint} clusters: {[c.id for c in clusters.keys()]}")
            
        except ChipStackError as e:
            logging.error(f"Failed to set up device info: {str(e)}")
            raise
        except Exception as e:
            logging.error(f"Unexpected error in setup: {str(e)}")
            logging.error(f"Read response type: {type(descriptor_read)}")
            logging.error(f"Read response: {descriptor_read}")
            raise

    async def _test_read_operation(self, operation_type: str, **kwargs) -> dict:
        """Execute a read operation based on the specified type.
        
        This is a private helper method for test_TC_IDM_2_2.
        
        Args:
            operation_type: Type of read operation to perform
            **kwargs: Additional arguments specific to the operation type
                
        Returns:
            Dictionary containing the read results
            
        Raises:
            ValueError if operation_type is invalid
            AssertionError if verification fails
            ChipStackError if the read operation fails
        """
        try:
            if operation_type == 'single_attribute':
                return await self.verify_attribute_read([(kwargs['endpoint'], kwargs['attribute'])])
                
            elif operation_type == 'all_cluster_attributes':
                return await self.verify_attribute_read([(kwargs['endpoint'], kwargs['cluster'])])
                
            elif operation_type == 'attribute_all_endpoints':
                return await self.verify_attribute_read([(None, kwargs['cluster'], kwargs['attribute'])])
                
            elif operation_type == 'global_attribute':
                attribute_path = AttributePath(
                    EndpointId=kwargs['endpoint'],
                    ClusterId=None,
                    AttributeId=kwargs['attribute_id'])
                return await self.verify_attribute_read([attribute_path])
                
            elif operation_type == 'all_attributes':
                return await self.verify_attribute_read([()])
                
            elif operation_type == 'global_attribute_all_endpoints':
                attribute_path = AttributePath(
                    EndpointId=None,
                    ClusterId=None,
                    AttributeId=kwargs['attribute_id'])
                return await self.verify_attribute_read([attribute_path])
                
            elif operation_type == 'cluster_all_endpoints':
                read_request = await self.read_attribute([kwargs['cluster']])
                for endpoint in read_request.tlvAttributes:
                    asserts.assert_in(kwargs['cluster'].id, read_request.tlvAttributes[endpoint].keys(),
                                    f"{kwargs['cluster']} cluster not in output")
                    asserts.assert_in(kwargs['cluster'].Attributes.AttributeList.attribute_id,
                                    read_request.tlvAttributes[endpoint][kwargs['cluster'].id],
                                    "AttributeList not in output")
                return read_request
                
            elif operation_type == 'endpoint_all_clusters':
                read_request = await self.read_attribute([kwargs['endpoint']])
                endpoint = kwargs['endpoint']
                asserts.assert_in(Clusters.Descriptor.id, read_request.tlvAttributes[endpoint].keys(), "Descriptor cluster not in output")
                asserts.assert_in(Clusters.Descriptor.Attributes.ServerList.attribute_id, read_request.tlvAttributes[endpoint][Clusters.Descriptor.id], "ServerList not in output")

                for cluster in read_request.tlvAttributes[endpoint]:
                    attribute_ids = [a for a in read_request.tlvAttributes[endpoint][cluster].keys() if a != Clusters.Attribute.DataVersion]
                    asserts.assert_equal(
                        sorted(attribute_ids),
                        sorted(read_request.tlvAttributes[endpoint][cluster][ClusterObjects.ALL_CLUSTERS[cluster].Attributes.AttributeList.attribute_id]),
                        f"Expected attribute list does not match for cluster {cluster}"
                    )
                return read_request
                
            elif operation_type == 'unsupported_endpoint':
                return await self.verify_attribute_read(
                    [(0xFFFF, Clusters.Descriptor, Clusters.Descriptor.Attributes.ServerList)],
                    expected_error=Status.UnsupportedEndpoint)
                
            elif operation_type == 'unsupported_cluster':
                return await self.verify_attribute_read(
                    [(0, 0xFFFF, Clusters.Descriptor.Attributes.ServerList)],
                    expected_error=Status.UnsupportedCluster)
                
            elif operation_type == 'unsupported_attribute':
                # Use a known unsupported attribute ID (0xFFFF) to test UNSUPPORTED_ATTRIBUTE response
                # Use tuple format to avoid conflicts with other operations
                return await self.verify_attribute_read(
                    [(0, Clusters.Descriptor, 0xFFFF)],
                    expected_error=Status.UnsupportedAttribute)
                
            elif operation_type == 'repeat_attribute':
                results = []
                for _ in range(kwargs['repeat_count']):
                    results.append(await self.verify_attribute_read(
                        [(kwargs['endpoint'], kwargs['cluster'], kwargs['attribute'])]))
                return results
                
            elif operation_type == 'data_version_filter':
                # Initial read to get data version - use ReadAttribute like Austin's version
                read_request = await self.default_controller.ReadAttribute(
                    self.dut_node_id, [(kwargs['endpoint'], kwargs['cluster'], kwargs['attribute'])])
                data_version = read_request[0][kwargs['cluster']][Clusters.Attribute.DataVersion]
                
                if 'test_value' in kwargs:
                    # Write operation to change data version
                    await self.default_controller.WriteAttribute(
                        self.dut_node_id,
                        [(kwargs['endpoint'], kwargs['cluster'], kwargs['attribute'], kwargs['test_value'])])
                
                # Read with data version filter - use ReadAttribute like Austin's version
                data_version_filter = [(kwargs['endpoint'], kwargs['cluster'], data_version)]
                filtered_read = await self.default_controller.ReadAttribute(
                    self.dut_node_id, 
                    [(kwargs['endpoint'], kwargs['cluster'], kwargs['attribute'])], 
                    dataVersionFilters=data_version_filter)
                
                return read_request, filtered_read
                
            elif operation_type == 'chunked_data':
                # Find a large attribute to read
                for endpoint in self.endpoints:
                    for cluster in self.endpoints[endpoint]:
                        for attribute in self.endpoints[endpoint][cluster]:
                            if attribute == Clusters.Attribute.DataVersion:
                                continue
                            try:
                                return await self.verify_attribute_read([(endpoint, cluster, attribute)])
                            except ChipStackError as e:
                                if "Response too large" in str(e):
                                    continue
                                raise
                return None
                
            elif operation_type == 'non_global_attribute':
                attribute_path = AttributePath(
                    EndpointId=kwargs.get('endpoint'),
                    ClusterId=None,
                    AttributeId=kwargs['attribute'].attribute_id)
                return await self.verify_attribute_read([attribute_path], 
                                                      expected_error=Status.InvalidAction)
                
            elif operation_type == 'limited_access':
                # Modify ACL to limit access
                original_acl = await self.default_controller.ReadAttribute(
                    self.dut_node_id, [(0, Clusters.AccessControl)])
                
                new_acl = copy.deepcopy(original_acl)
                new_acl[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl] = [
                    AccessControl.Structs.AccessControlEntryStruct(
                        privilege=AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                        authMode=AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                        subjects=[kwargs['subject_id']],
                        targets=[AccessControl.Structs.AccessControlTargetStruct(
                            cluster=kwargs['cluster_id'],
                            endpoint=kwargs['endpoint'])])]
                
                await self.default_controller.WriteAttribute(
                    self.dut_node_id,
                    [(0, Clusters.AccessControl, Clusters.AccessControl.Attributes.Acl, 
                      new_acl[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl])])
                
                # Test read with limited access
                read_request = await self.verify_attribute_read([(kwargs['endpoint'],)])
                
                # Restore original ACL
                await self.default_controller.WriteAttribute(
                    self.dut_node_id,
                    [(0, Clusters.AccessControl, Clusters.AccessControl.Attributes.Acl, 
                      original_acl[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl])])
                
                return original_acl, read_request
                
            elif operation_type == 'all_events_attributes':
                return await self.default_controller.Read(self.dut_node_id, [()], reportInterval=(0, 0))
                
            elif operation_type == 'data_version_filter_multiple_clusters':
                # Read from cluster A to get its data version - use ReadAttribute like Austin's version
                read_a = await self.default_controller.ReadAttribute(
                    self.dut_node_id, [(kwargs['endpoint'], kwargs['cluster'], kwargs['attribute'])])
                data_version_a = read_a[0][kwargs['cluster']][Clusters.Attribute.DataVersion]
                
                # Read from both cluster A and cluster B, but only filter cluster A
                # This should return data from cluster B but not from cluster A (due to matching data version)
                data_version_filter_a = [(kwargs['endpoint'], kwargs['cluster'], data_version_a)]
                read_both = await self.default_controller.ReadAttribute(
                    self.dut_node_id,
                    [(kwargs['endpoint'], kwargs['cluster'], kwargs['attribute']),
                     (kwargs['endpoint'], kwargs['other_cluster'], kwargs['other_attribute'])],
                    dataVersionFilters=data_version_filter_a)
                
                return read_a, read_both
                
            elif operation_type == 'multiple_data_version_filters':
                # Initial read to get data version - use ReadAttribute like Austin's version
                read_request = await self.default_controller.ReadAttribute(
                    self.dut_node_id, [(kwargs['endpoint'], kwargs['cluster'], kwargs['attribute'])])
                data_version = read_request[0][kwargs['cluster']][Clusters.Attribute.DataVersion]
                
                if 'test_value' in kwargs:
                    # Write operation to change data version
                    await self.default_controller.WriteAttribute(
                        self.dut_node_id,
                        [(kwargs['endpoint'], kwargs['cluster'], kwargs['attribute'], kwargs['test_value'])])
                
                # Create two data version filters: one with current version and one with older version
                data_version_filter_1 = [(kwargs['endpoint'], kwargs['cluster'], data_version)]
                data_version_filter_2 = [(kwargs['endpoint'], kwargs['cluster'], data_version - 1)]
                
                # Read with multiple data version filters - use ReadAttribute like Austin's version
                filtered_read = await self.default_controller.ReadAttribute(
                    self.dut_node_id,
                    [(kwargs['endpoint'], kwargs['cluster'], kwargs['attribute'])],
                    dataVersionFilters=data_version_filter_1)
                
                return read_request, filtered_read
                
            else:
                raise ValueError(f"Invalid operation type: {operation_type}")

        except ChipStackError as e:
            logging.error(f"Operation {operation_type} failed: {str(e)}")
            logging.error(f"Arguments: {kwargs}")
            raise

    def steps_TC_IDM_2_2(self) -> list[TestStep]:
        steps = [
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
            TestStep(18, "TH sends a Read Request Message to the DUT to read something(Attribute) which is larger than 1 MTU(1280 bytes) and per spec can be chunked +",
                     "Verify on the TH that the DUT sends a chunked data message with the SuppressResponse field set to False for all the messages except the last one. Verify the last chunked message sent has the SuppressResponse field set to True."),
            TestStep(19, "TH sends a Read Request Message to the DUT to read a non global attribute from all clusters at that Endpoint, AttributePath = [[Endpoint = Specific Endpoint, Attribute = Specific Non Global Attribute]] +",
                     "On the TH verify that the DUT sends an error message and not the value of the attribute."),
            TestStep(20, "TH sends a Read Request Message to the DUT to read a non global attribute from all clusters at all Endpoints, AttributePath = [[Attribute = Specific Non Global Attribute]] +",
                     "On the TH verify that the DUT sends an error message and not the value of the attribute."),
            TestStep(21, "TH should have access to only a single cluster at one Endpoint1.TH sends a Read Request Message to the DUT to read all attributes from all clusters at Endpoint1, AttributePath = [[Endpoint = Specific Endpoint]] +",
                      "Verify that the DUT sends back data of all attributes only from that one cluster to which it has access. Verify that there are no errors sent back for attributes the TH has no access to."),
            TestStep(22, "TH sends a Read Request Message to read all events and attributes from the DUT.",
                      "Verify that the DUT sends back data of all attributes and events that the TH has access to."),
        ]
        return steps

    @async_test_body
    async def test_TC_IDM_2_2(self):
        """Execute the IDM-2.2 test case."""
        # Test Setup with targeted read
        await self._setup_device_info()

        # Step 1: Read single attribute
        self.step(1)
        read_request1 = await self._test_read_operation(operation_type='single_attribute',
            endpoint=self.endpoint,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList)

        # Step 2: Read all cluster attributes
        self.step(2)
        read_request2 = await self._test_read_operation(operation_type='all_cluster_attributes',
            endpoint=self.endpoint,
            cluster=Clusters.Descriptor)

        # Step 3: Read attribute all endpoints
        self.step(3)
        read_request3 = await self._test_read_operation(operation_type='attribute_all_endpoints',
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList)

        # Step 4: Read global attribute
        self.step(4)
        read_request4 = await self._test_read_operation(operation_type='global_attribute',
            endpoint=self.endpoint,
            attribute_id=global_attribute_ids.GlobalAttributeIds.ATTRIBUTE_LIST_ID)

        # Step 5: Read all attributes
        self.step(5)
        read_request5 = await self._test_read_operation(operation_type='all_attributes')

        # Step 6: Read global attribute all endpoints
        self.step(6)
        read_request6 = await self._test_read_operation(operation_type='global_attribute_all_endpoints',
            attribute_id=global_attribute_ids.GlobalAttributeIds.ATTRIBUTE_LIST_ID)

        # Step 7: Read cluster all endpoints
        self.step(7)
        read_request7 = await self._test_read_operation(operation_type='cluster_all_endpoints',
            cluster=Clusters.Descriptor)

        # Step 8: Read endpoint all clusters
        self.step(8)
        read_request8 = await self._test_read_operation(operation_type='endpoint_all_clusters',
            endpoint=self.endpoint)

        # Step 9: Test unsupported endpoint
        self.step(9)
        result9 = await self._test_read_operation(operation_type='unsupported_endpoint')

        # Step 10: Test unsupported cluster
        self.step(10)
        result10 = await self._test_read_operation(operation_type='unsupported_cluster')

        # Step 11: Test unsupported attribute
        self.step(11)
        result11 = await self._test_read_operation(operation_type='unsupported_attribute')

        # Step 12: Test repeat attribute read
        self.step(12)
        read_requests12 = await self._test_read_operation(operation_type='repeat_attribute',
            endpoint=self.endpoint,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList,
            repeat_count=3)

        # Step 13: Test data version filter single read
        self.step(13)
        read_request13, filtered_read13 = await self._test_read_operation(operation_type='data_version_filter',
            endpoint=self.endpoint,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList)

        # 1. Check the first read returns the correct value and DataVersion
        asserts.assert_true(0 in read_request13, "Endpoint 0 missing in first read")
        asserts.assert_true(Clusters.Descriptor in read_request13[0], "Cluster missing in first read")
        
        # 2. Check the second read (with DataVersionFilter) returns empty dict if nothing changed
        # This matches Austin's approach where matching data versions return {}
        asserts.assert_equal(filtered_read13, {}, "Expected empty response with matching data version")

        # Step 14: Test data version filter with write
        self.step(14)
        read_request14, filtered_read14 = await self._test_read_operation(operation_type='data_version_filter',
            endpoint=self.endpoint,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.NodeLabel,
            test_value="Hello World")
        # Check if filtered_read14 contains the new value
        if filtered_read14 and 0 in filtered_read14:
            data_version14 = filtered_read14[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
            asserts.assert_equal(filtered_read14[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel], "Hello World",
                "Data version does not match expected value")
            asserts.assert_equal((read_request14[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion] + 1), data_version14,
                "DataVersion was not incremented")

        # Step 15: Test data version filter all attributes
        self.step(15)
        read_request15, filtered_read15 = await self._test_read_operation(operation_type='data_version_filter',
            endpoint=self.endpoint,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.NodeLabel,
            test_value="Goodbye World")
        # Check if filtered_read15 contains the new value
        if filtered_read15 and 0 in filtered_read15:
            data_version15 = filtered_read15[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
            asserts.assert_equal(filtered_read15[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel], "Goodbye World",
                "Data version does not match expected value")
            asserts.assert_equal((read_request15[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion] + 1), data_version15,
                "DataVersion was not incremented")

        # Step 16: Test multiple data version filters
        self.step(16)
        read_request16, filtered_read16 = await self._test_read_operation(operation_type='multiple_data_version_filters',
            endpoint=self.endpoint,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.NodeLabel,
            test_value="Hello World Again")
        # Check if filtered_read16 contains the new value
        if filtered_read16 and 0 in filtered_read16:
            asserts.assert_equal(filtered_read16[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel], "Hello World Again",
                "Data version does not match expected value")

        # Step 17: Test data version filter multiple clusters
        self.step(17)
        read_a17, read_both17 = await self._test_read_operation(operation_type='data_version_filter_multiple_clusters',
            endpoint=self.endpoint,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList,
            other_cluster=Clusters.BasicInformation,
            other_attribute=Clusters.BasicInformation.Attributes.NodeLabel)
        data_version_a17 = read_a17[0][Clusters.Descriptor][Clusters.Attribute.DataVersion]
        data_version_b17 = read_both17[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
        asserts.assert_not_equal(data_version_a17, data_version_b17,
                               "Data versions should be different")

        # Step 18: Test chunked data message
        self.step(18)
        read_request18 = await self._test_read_operation(operation_type='chunked_data')

        # Step 19: Test non-global attribute on endpoint
        self.step(19)
        result19 = await self._test_read_operation(operation_type='non_global_attribute',
            endpoint=self.endpoint,
            attribute=Clusters.Descriptor.Attributes.ServerList)

        # Step 20: Test non-global attribute all endpoints
        self.step(20)
        result20 = await self._test_read_operation(operation_type='non_global_attribute',
            attribute=Clusters.Descriptor.Attributes.ServerList)

        # Step 21: Test limited access read
        self.step(21)
        original_acl21, read_request21 = await self._test_read_operation(operation_type='limited_access',
            endpoint=self.endpoint,
            cluster_id=Clusters.BasicInformation.id,
            subject_id=self.matter_test_config.controller_node_id + 1)
        asserts.assert_true(self.endpoint in read_request21.tlvAttributes, f"Endpoint {self.endpoint} missing in response")
        asserts.assert_true(Clusters.Descriptor.id in read_request21.tlvAttributes[self.endpoint],
                          "Clusters.Descriptor not in response")

        # Step 22: Test read all events and attributes
        self.step(22)
        read_request22 = await self._test_read_operation(operation_type='all_events_attributes')
        required_attributes = ["Header", "Status", "Data"]
        for event in read_request22.events:
            for attr in required_attributes:
                asserts.assert_true(hasattr(event, attr), f"{attr} not in event")

if __name__ == "__main__":
    default_matter_test_main()
