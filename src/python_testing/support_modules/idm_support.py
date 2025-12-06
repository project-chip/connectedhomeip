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

"""
Support module for IDM (Interaction Data Model) test modules containing shared functionality.
"""

import copy
import inspect
import logging
from typing import Any, Optional

from mobly import asserts

log = logging.getLogger(__name__)

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters import ClusterObjects as ClusterObjects
from matter.clusters.Attribute import AttributePath, TypedAttributePath
from matter.exceptions import ChipStackError
from matter.interaction_model import InteractionModelError, Status
from matter.testing import global_attribute_ids
from matter.testing.matter_testing import MatterBaseTest

# ============================================================================
# Module-Level Utility Functions
# ============================================================================


def get_all_cmds_for_cluster_id(cid: int) -> list[Clusters.ClusterObjects.ClusterCommand]:
    """Get all commands for a given cluster ID.

    Args:
        cid: Cluster ID to get commands for

    Returns:
        List of command classes for the cluster, or empty list if cluster has no commands

    Note:
        Some clusters don't have a Commands attribute (they have no commands defined).
        Returning an empty list allows callers to safely iterate through all clusters
        without special-casing clusters that don't support commands.
    """
    cluster = Clusters.ClusterObjects.ALL_CLUSTERS[cid]
    if hasattr(cluster, 'Commands'):
        return inspect.getmembers(cluster.Commands, inspect.isclass)
    return []


def client_cmd(cmd_class):
    """Filter to check if a command class is a client command.

    Args:
        cmd_class: Command class to check

    Returns:
        The command class if it's a client command, None otherwise

    Note:
        Inspect returns all classes, not just command classes. Some builtin classes
        won't have the is_client attribute, so we check for it explicitly.
    """
    if hasattr(cmd_class, 'is_client'):
        return cmd_class if cmd_class.is_client else None
    return None


# ============================================================================
# IDMBaseTest - Main Base Class
# ============================================================================

class IDMBaseTest(MatterBaseTest):
    """Base test class for IDM tests with shared functionality."""

    ROOT_NODE_ENDPOINT_ID = 0

    # ========================================================================
    # Descriptor and Cluster Reading Utilities
    # ========================================================================

    async def get_descriptor_server_list(self, ctrl: ChipDeviceCtrl, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Read ServerList attribute from Descriptor cluster.

        Args:
            ctrl: Controller to use for reading
            ep: Endpoint to read from (default: ROOT_NODE_ENDPOINT_ID)

        Returns:
            ServerList attribute value
        """
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
        )

    async def get_descriptor_parts_list(self, ctrl: ChipDeviceCtrl, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Read PartsList attribute from Descriptor cluster.

        Args:
            ctrl: Controller to use for reading
            ep: Endpoint to read from (default: ROOT_NODE_ENDPOINT_ID)

        Returns:
            PartsList attribute value
        """
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList
        )

    async def get_idle_mode_duration_sec(self, ctrl: ChipDeviceCtrl, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Read IdleModeDuration attribute from ICD Management cluster.

        Args:
            ctrl: Controller to use for reading
            ep: Endpoint to read from (default: ROOT_NODE_ENDPOINT_ID)

        Returns:
            IdleModeDuration attribute value in seconds
        """
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.IdleModeDuration
        )

    # ========================================================================
    # ACL Management Functions
    # ========================================================================

    async def get_dut_acl(self, ctrl: ChipDeviceCtrl, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Read the ACL attribute from the DUT.

        Args:
            ctrl: Controller to use for reading
            ep: Endpoint to read from (default: ROOT_NODE_ENDPOINT_ID)

        Returns:
            ACL list from the DUT
        """
        sub = await ctrl.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[(ep, Clusters.AccessControl.Attributes.Acl)],
            keepSubscriptions=False,
            fabricFiltered=True
        )
        return sub[ep][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

    async def write_dut_acl(self, ctrl: ChipDeviceCtrl, acl, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Write an ACL to the DUT.

        Args:
            ctrl: Controller to use for writing
            acl: ACL list to write
            ep: Endpoint to write to (default: ROOT_NODE_ENDPOINT_ID)
        """
        result = await ctrl.WriteAttribute(self.dut_node_id, [(ep, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[ep].Status, Status.Success, "ACL write failed")

    async def add_ace_to_dut_acl(self, ctrl: ChipDeviceCtrl, ace, dut_acl_original):
        """Add an Access Control Entry (ACE) to the DUT's ACL.

        Args:
            ctrl: Controller to use for writing
            ace: Access Control Entry to add
            dut_acl_original: Original ACL list to append to
        """
        dut_acl = copy.deepcopy(dut_acl_original)
        dut_acl.append(ace)
        await self.write_dut_acl(ctrl=ctrl, acl=dut_acl)

    # ========================================================================
    # Attribute Path Utilities
    # ========================================================================

    @staticmethod
    def get_typed_attribute_path(attribute, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Create a TypedAttributePath from an attribute.

        Args:
            attribute: Attribute to create path for
            ep: Endpoint ID (default: ROOT_NODE_ENDPOINT_ID)

        Returns:
            TypedAttributePath object
        """
        return TypedAttributePath(
            Path=AttributePath.from_attribute(
                EndpointId=ep,
                Attribute=attribute
            )
        )

    async def find_timed_write_attribute(
        self, endpoints_data: dict[int, Any]
    ) -> tuple[Optional[int], Optional[type[ClusterObjects.ClusterAttributeDescriptor]]]:
        """
        Find an attribute that requires timed write on the actual device
        Uses the wildcard read data that's already in endpoints_data
        """
        log.info(f"Searching for timed write attributes across {len(endpoints_data)} endpoints")
        for endpoint_id, endpoint in endpoints_data.items():
            for cluster_type, cluster_data in endpoint.items():
                cluster_id = cluster_type.id

                cluster_type_enum = global_attribute_ids.cluster_id_type(cluster_id)
                # If debugging, please uncomment the following line to add Unit Testing clusters to the search and comment out the line below it.

                if cluster_type_enum != global_attribute_ids.ClusterIdType.kStandard and cluster_type_enum != global_attribute_ids.ClusterIdType.kTest:
                    # if cluster_type_enum != global_attribute_ids.ClusterIdType.kStandard:
                    continue
                for attr_type in cluster_data:
                    # Check if this is an attribute descriptor class
                    if (isinstance(attr_type, type) and
                            issubclass(attr_type, ClusterObjects.ClusterAttributeDescriptor)):
                        # Check if this attribute requires timed write using the must_use_timed_write class property
                        if attr_type.must_use_timed_write:
                            log.info(f"Found timed write attribute: {attr_type.__name__} "
                                     f"in cluster {cluster_type.__name__} on endpoint {endpoint_id}")
                            return endpoint_id, attr_type
        log.warning("No timed write attributes found on device")

    # ========================================================================
    # Attribute Verification Functions
    # ========================================================================

    @staticmethod
    def verify_attribute_exists(sub, cluster, attribute, ep: int = ROOT_NODE_ENDPOINT_ID):
        """Verify that an attribute exists in a subscription or read response.

        Args:
            sub: Subscription or read response to check
            cluster: Cluster to check for
            attribute: Attribute to check for
            ep: Endpoint to check (default: ROOT_NODE_ENDPOINT_ID)
        """
        sub_attrs = sub
        if isinstance(sub, Clusters.Attribute.SubscriptionTransaction):
            sub_attrs = sub.GetAttributes()

        asserts.assert_true(ep in sub_attrs, "Must have read endpoint %s data" % ep)
        asserts.assert_true(cluster in sub_attrs[ep], "Must have read %s cluster data" % cluster.__name__)
        asserts.assert_true(attribute in sub_attrs[ep][cluster],
                            "Must have read back attribute %s" % attribute.__name__)

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

    # ========================================================================
    # Attribute Reading Helper Functions
    # ========================================================================

    async def verify_attribute_read(self, attribute_path: list) -> dict:
        """Read and verify attributes from the device.

        Args:
            attribute_path: List of attribute paths to read (should be AttributePath objects or tuples)

        Returns:
            Dictionary containing the read results
        """
        read_response = await self.default_controller.Read(
            self.dut_node_id,
            attribute_path)
        self.verify_attribute_path(read_response, attribute_path[0])
        return read_response

    async def read_global_attribute_all_endpoints(self, attribute_id):
        """Read a global attribute from all endpoints.

        Args:
            attribute_id: Global attribute ID to read

        Returns:
            Read response dictionary
        """
        attribute_path = AttributePath(
            EndpointId=None,
            ClusterId=None,
            AttributeId=attribute_id)
        return await self.verify_attribute_read([attribute_path])

    async def read_cluster_all_endpoints(self, cluster):
        """Read all attributes from a cluster across all endpoints.

        Args:
            cluster: Cluster to read from

        Returns:
            Read response dictionary
        """
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

    async def read_endpoint_all_clusters(self, endpoint):
        """Read all attributes from all clusters on an endpoint.

        Args:
            endpoint: Endpoint to read from

        Returns:
            Read response dictionary
        """
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

    async def read_unsupported_endpoint(self):
        """Find an unsupported endpoint and attempt to read from it.

        Expects an UnsupportedEndpoint error.
        """
        supported_endpoints = set(self.endpoints.keys())
        all_endpoints = set(range(max(supported_endpoints) + 2))
        unsupported = list(all_endpoints - supported_endpoints)
        await self.read_single_attribute_expect_error(
            endpoint=unsupported[0],
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.FeatureMap,
            error=Status.UnsupportedEndpoint)

    async def read_unsupported_cluster(self):
        """Find a standard cluster that's not supported on any endpoint and try to read from it.

        Expects an UnsupportedCluster error.
        """
        # Get all standard clusters supported on all endpoints
        supported_cluster_ids = set()
        for endpoint_clusters in self.endpoints.values():
            supported_cluster_ids.update({cluster.id for cluster in endpoint_clusters.keys(
            ) if global_attribute_ids.cluster_id_type(cluster.id) == global_attribute_ids.ClusterIdType.kStandard})

        # Get all possible standard clusters
        all_standard_cluster_ids = {cluster_id for cluster_id in ClusterObjects.ALL_CLUSTERS.keys(
        ) if global_attribute_ids.cluster_id_type(cluster_id) == global_attribute_ids.ClusterIdType.kStandard}

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

    async def read_unsupported_attribute(self):
        """Attempt to read an unsupported attribute from a supported cluster on any endpoint.

        Expects an UNSUPPORTED_ATTRIBUTE error from the DUT.
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
        asserts.fail("No unsupported attributes found - must find at least one unsupported attribute")

    async def read_repeat_attribute(self, endpoint, cluster, attribute, repeat_count):
        """Read the same attribute multiple times and verify consistency.

        Args:
            endpoint: Endpoint to read from
            cluster: Cluster to read from
            attribute: Attribute to read
            repeat_count: Number of times to repeat the read

        Returns:
            List of read results
        """
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

    async def read_data_version_filter(self, endpoint, cluster, attribute, test_value=None):
        """Read an attribute with data version filtering.

        Args:
            endpoint: Endpoint to read from
            cluster: Cluster to read from
            attribute: Attribute to read
            test_value: Optional value to write before second read

        Returns:
            Tuple of (initial_read_response, filtered_read_response)
        """
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

    async def read_non_global_attribute_across_all_clusters(self, endpoint=None, attribute=None):
        """Attempt to read a non-global attribute across all clusters.

        This should fail with INVALID_ACTION error.

        Args:
            endpoint: Endpoint to read from (None for all endpoints)
            attribute: Non-global attribute to attempt to read
        """
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

    async def read_limited_access(self, endpoint, cluster_id):
        """Test reading all attributes from all clusters at an endpoint with limited access.

        Creates a second controller (TH2) with limited access to only one cluster, then reads
        all attributes from all clusters at the endpoint. Verifies that only the allowed cluster
        is returned and no errors are sent for clusters without access.

        Args:
            endpoint: The endpoint to test
            cluster_id: The cluster ID to grant access to

        Returns:
            Tuple of (original_acl, read_response)
        """
        # Creates a second controller (TH2)
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        TH2_nodeid = self.matter_test_config.controller_node_id + 1
        TH2 = fabric_admin.NewController(nodeId=TH2_nodeid)

        # Read and save the original ACL using the default (admin) controller
        read_acl = await self.default_controller.Read(
            self.dut_node_id,
            [(endpoint, Clusters.AccessControl.Attributes.Acl)])
        dut_acl_original = read_acl.attributes[endpoint][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

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
                    [(endpoint, Clusters.AccessControl.Attributes.Acl(dut_acl_original))])
                log.info("Restored original ACL")
            except Exception as e:
                log.error(f"Failed to restore original ACL: {e}")

            # Removes TH2 controller
            TH2.Shutdown()

    async def read_all_events_attributes(self):
        """Read all events and attributes from the DUT.

        Returns:
            Read response with events and attributes
        """
        return await self.default_controller.Read(nodeId=self.dut_node_id, attributes=[()], events=[()])

    async def read_data_version_filter_multiple_clusters(self, endpoint, cluster, attribute, other_cluster, other_attribute):
        """Read from multiple clusters with data version filter on one cluster.

        Args:
            endpoint: Endpoint to read from
            cluster: First cluster to read (with filter)
            attribute: Attribute from first cluster
            other_cluster: Second cluster to read (without filter)
            other_attribute: Attribute from second cluster

        Returns:
            Tuple of (first_cluster_read, both_clusters_read)
        """
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

    async def read_multiple_data_version_filters(self, endpoint, cluster, attribute, test_value=None):
        """Read with multiple data version filters (old and new).

        Args:
            endpoint: Endpoint to read from
            cluster: Cluster to read from
            attribute: Attribute to read
            test_value: Optional value to write to change data version

        Returns:
            Tuple of (initial_read, filtered_read_with_multiple_filters)
        """
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

    def verify_empty_wildcard(self, attr_path, read_request):
        """Verify read response for empty tuple path (all attributes from all clusters on all endpoints).

        This method is based on implementation in PR #34003.

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
