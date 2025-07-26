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

import asyncio
import copy
import inspect
import logging
from enum import IntFlag
from typing import Any, Optional

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import AttributePath
from chip.clusters.enum import MatterIntEnum
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from chip.testing import global_attribute_ids
from chip.testing.basic_composition import BasicCompositionTests
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


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
        self.large_operation_timeout_ms = 60000  # 60 second timeout for large operations (like reading all attributes)
        self.setup_timeout_ms = 60000  # 60 second timeout for setup phase
        # Operation type to handler mapping (use function references directly)
        self._operation_map = {
            self._read_single_attribute: self._read_single_attribute,
            self._read_all_cluster_attributes: self._read_all_cluster_attributes,
            self._read_attribute_all_endpoints: self._read_attribute_all_endpoints,
            self._read_global_attribute: self._read_global_attribute,
            self._read_all_attributes: self._read_all_attributes,
            self._read_global_attribute_all_endpoints: self._read_global_attribute_all_endpoints,
            self._read_cluster_all_endpoints: self._read_cluster_all_endpoints,
            self._read_endpoint_all_clusters: self._read_endpoint_all_clusters,
            self._read_unsupported_endpoint: self._read_unsupported_endpoint,
            self._read_unsupported_cluster: self._read_unsupported_cluster,
            self._read_unsupported_attribute: self._read_unsupported_attribute,
            self._read_repeat_attribute: self._read_repeat_attribute,
            self._read_data_version_filter: self._read_data_version_filter,
            self._read_non_global_attribute: self._read_non_global_attribute,
            self._read_limited_access: self._read_limited_access,
            self._read_all_events_attributes: self._read_all_events_attributes,
            self._read_data_version_filter_multiple_clusters: self._read_data_version_filter_multiple_clusters,
            self._read_multiple_data_version_filters: self._read_multiple_data_version_filters,
        }

    # === Attribute Reading and Verification ===

    async def read_attribute(self, attribute_path: list, timeout_ms: int = None) -> dict:
        """Read attributes from the device with timeout handling."""
        if timeout_ms is None:
            timeout_ms = self.timeout_ms

        try:
            return await self.default_controller.Read(
                self.dut_node_id,
                attribute_path)
        except ChipStackError as e:  # chipstack-ok
            if "Timeout" in str(e):
                logging.error(f"Read operation timed out after {timeout_ms}ms")
                logging.error(f"Attribute path: {attribute_path}")
            raise

    async def verify_attribute_read(self, attribute_path: list, expected_error: Status = None) -> dict:
        """Read and verify attributes from the device.

        Args:
            attribute_path: List of attribute paths to read (should be AttributePath or direct values)
            expected_error: Optional expected error status

        Returns:
            Dictionary containing the read results

        Raises:
            AssertionError if verification fails
        """
        if expected_error:
            try:
                endpoint = attribute_path[0][0] if len(attribute_path[0]) > 0 else None
                cluster = attribute_path[0][1] if len(attribute_path[0]) > 1 else None
                attribute = attribute_path[0][2] if len(attribute_path[0]) > 2 else None
                result = await self.read_single_attribute_expect_error(
                    endpoint=endpoint,
                    cluster=cluster,
                    attribute=attribute,
                    error=expected_error)
                return result
            except KeyError:
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
            # Assert PartsList is always present
            asserts.assert_in(
                Clusters.Descriptor.Attributes.PartsList.attribute_id,
                read_request.tlvAttributes[endpoint][Clusters.Descriptor.id],
                "PartsList attribute should always be present")
            parts_list = read_request.tlvAttributes[endpoint][Clusters.Descriptor.id][
                Clusters.Descriptor.Attributes.PartsList.attribute_id]
            if Clusters.Descriptor.Attributes.PartsList in self.endpoints[endpoint][Clusters.Descriptor]:
                asserts.assert_equal(parts_list, self.endpoints[endpoint][Clusters.Descriptor][
                    Clusters.Descriptor.Attributes.PartsList],
                    "Parts list is not the expected value")

            # Server list matches returned clusters
            returned_clusters = sorted(list(read_request.tlvAttributes[endpoint].keys()))
            server_list = sorted(read_request.tlvAttributes[endpoint][Clusters.Descriptor.id][
                Clusters.Descriptor.Attributes.ServerList.attribute_id])
            asserts.assert_equal(returned_clusters, server_list,
                                 f"Cluster list and server list do not match for endpoint {endpoint}")

            # Attribute lists
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
                await self._verify_single_endpoint_path(read_request, ep, attribute)
        else:
            await self._verify_single_endpoint_path(read_request, endpoint, attribute)

    async def _verify_single_endpoint_path(
        self,
        read_request: dict,
        endpoint: int,
        attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor
    ) -> None:
        """Verify read response for a single endpoint path.
        Args:
            read_request: The read request response to verify
            endpoint: The endpoint to verify
            attribute: The specific attribute (if any)
        Raises:
            AssertionError if verification fails
        """
        # This method is only used for Descriptor cluster, so we can use it directly
        server_list = read_request.tlvAttributes[endpoint][Clusters.Descriptor.id][
            Clusters.Descriptor.Attributes.ServerList.attribute_id]
        asserts.assert_equal(sorted(server_list), sorted([x.id for x in self.endpoints[endpoint]]))

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

    async def _read_single_attribute(self, endpoint, attribute):
        # Find the cluster that contains this attribute
        cluster_obj = None
        for cluster_name in dir(Clusters):
            cluster = getattr(Clusters, cluster_name)
            if hasattr(cluster, 'Attributes') and hasattr(cluster.Attributes, attribute.__name__):
                cluster_obj = cluster
                break

        if cluster_obj is None:
            raise ValueError(f"Could not determine cluster for attribute {attribute}")

        return await self.verify_attribute_read([(endpoint, cluster_obj, attribute)])

    async def _read_all_cluster_attributes(self, endpoint, cluster):
        return await self.verify_attribute_read([(endpoint, cluster)])

    async def _read_attribute_all_endpoints(self, cluster, attribute):
        return await self.verify_attribute_read([(None, cluster, attribute)])

    async def _read_global_attribute(self, endpoint, attribute_id):
        attribute_path = AttributePath(
            EndpointId=endpoint,
            ClusterId=None,
            AttributeId=attribute_id)
        return await self.verify_attribute_read([attribute_path])

    async def _read_all_attributes(self):
        read_request = await asyncio.wait_for(
            self.default_controller.Read(self.dut_node_id, [()]),
            timeout=120.0
        )
        await self._verify_empty_tuple([()], read_request)
        return read_request

    async def _read_global_attribute_all_endpoints(self, attribute_id):
        attribute_path = AttributePath(
            EndpointId=None,
            ClusterId=None,
            AttributeId=attribute_id)
        return await self.verify_attribute_read([attribute_path])

    async def _read_cluster_all_endpoints(self, cluster):
        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [cluster])
        for endpoint in read_request:
            asserts.assert_in(cluster, read_request[endpoint].keys(),
                              f"{cluster} cluster not in output")
            asserts.assert_in(cluster.Attributes.AttributeList,
                              read_request[endpoint][cluster],
                              "AttributeList not in output")
        return read_request

    async def _read_endpoint_all_clusters(self, endpoint):
        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [endpoint])
        asserts.assert_in(Clusters.Descriptor, read_request[endpoint].keys(), "Descriptor cluster not in output")
        asserts.assert_in(Clusters.Descriptor.Attributes.ServerList,
                          read_request[endpoint][Clusters.Descriptor], "ServerList not in output")
        for cluster in read_request[endpoint]:
            attribute_ids = [a.attribute_id for a in read_request[endpoint][cluster].keys()
                             if a != Clusters.Attribute.DataVersion]
            asserts.assert_equal(
                sorted(attribute_ids),
                sorted(read_request[endpoint][cluster][cluster.Attributes.AttributeList]),
                f"Expected attribute list does not match for cluster {cluster}"
            )
        return read_request

    async def _read_unsupported_endpoint(self):
        return await self.verify_attribute_read(
            [(0xFFFF, Clusters.Descriptor, Clusters.Descriptor.Attributes.ServerList)],
            expected_error=Status.UnsupportedEndpoint)

    async def _read_unsupported_cluster(self):
        return await self.verify_attribute_read(
            [(0, 0xFFFF, Clusters.Descriptor.Attributes.ServerList)],
            expected_error=Status.UnsupportedCluster)

    async def _read_unsupported_attribute(self):
        """
        Attempts to read an unsupported attribute from a supported cluster on any endpoint.
        Expects an UNSUPPORTED_ATTRIBUTE error from the DUT.
        Fails the test if no such attribute is found, but as a fallback, tries a known invalid attribute ID (0xFFFF).
        """
        for endpoint_id, endpoint in self.endpoints.items():
            for cluster_type, cluster in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                # Skip clusters that do not have AttributeList
                if not hasattr(cluster_type.Attributes, "AttributeList") or cluster_type.Attributes.AttributeList not in cluster:
                    continue

                all_attrs = set(ClusterObjects.ALL_ATTRIBUTES[cluster_type.id].keys())
                dut_attrs = set(cluster[cluster_type.Attributes.AttributeList])

                unsupported = [
                    attr_id for attr_id in (all_attrs - dut_attrs)
                    if global_attribute_ids.attribute_id_type(attr_id) == global_attribute_ids.AttributeIdType.kStandardNonGlobal
                ]
                if unsupported:
                    unsupported_attr = ClusterObjects.ALL_ATTRIBUTES[cluster_type.id][unsupported[0]]
                    logging.info(
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
                    return

        # Fallback: try a known invalid attribute according to the yaml script version of this test
        try:
            result = await self.read_single_attribute_expect_error(
                endpoint=1,
                cluster=Clusters.Thermostat,
                attribute=Clusters.Thermostat.Attributes.OutdoorTemperature,
                error=Status.UnsupportedAttribute
            )
            asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                msg="Unexpected success reading unsupported attribute")
            return
        except AssertionError as e:
            # This means the DUT did not return the expected UNSUPPORTED_ATTRIBUTE error
            logging.error(f"AssertionError during unsupported attribute test: {e}")
        except KeyError as e:
            # This means the cluster or attribute does not exist in the Python cluster definitions
            logging.error(f"KeyError during unsupported attribute test: {e}")

        # If we get here, neither the for-loop nor the fallback succeeded
        asserts.fail("Could not find a cluster/attribute combination to test unsupported attribute (even with fallback)")

    async def _read_repeat_attribute(self, endpoint, cluster, attribute, repeat_count):
        results = []
        for _ in range(repeat_count):
            results.append(await self.verify_attribute_read(
                [(endpoint, cluster, attribute)]))
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

    async def _read_non_global_attribute(self, endpoint=None, attribute=None):
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
            asserts.assert_equal(e.err, 0x580,
                                 "Incorrect error response for reading non-global attribute on all clusters at endpoint")
            return None

    async def _read_limited_access(self, endpoint, cluster_id, subject_id):
        read_request = await self.default_controller.Read(
            self.dut_node_id,
            [(endpoint, Clusters.AccessControl.Attributes.Acl)])
        dut_acl_original = read_request.attributes[endpoint][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]
        ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=cluster_id)],
            subjects=[subject_id])
        dut_acl = copy.deepcopy(dut_acl_original)
        dut_acl.append(ace)
        await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(endpoint, Clusters.AccessControl.Attributes.Acl(dut_acl))])
        read_request = await self.default_controller.Read(
            self.dut_node_id,
            [(endpoint, Clusters.AccessControl.Attributes.Acl)])
        asserts.assert_equal(len(read_request.attributes[endpoint]
                             [Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]), 2)
        await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(endpoint, Clusters.AccessControl.Attributes.Acl(dut_acl_original))])
        read_request = await self.default_controller.Read(
            self.dut_node_id,
            [(endpoint, Clusters.AccessControl.Attributes.Acl)])
        asserts.assert_equal(len(read_request.attributes[endpoint]
                             [Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]), 1)
        return dut_acl_original, read_request

    async def _read_all_events_attributes(self):
        return await self.default_controller.Read(nodeid=self.dut_node_id, attributes=[()], events=[()])

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
        read_request = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(endpoint, cluster, attribute)])
        data_version = read_request[0][cluster][Clusters.Attribute.DataVersion]
        if test_value is not None:
            await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(endpoint, attribute(value=test_value))])
        data_version_filter_1 = [(endpoint, cluster, data_version)]
        filtered_read = await self.default_controller.ReadAttribute(
            self.dut_node_id,
            [(endpoint, cluster, attribute)],
            dataVersionFilters=data_version_filter_1)
        return read_request, filtered_read

    async def _test_read_operation(self, handler, **kwargs) -> dict:
        try:
            if handler is None or handler not in self._operation_map:
                raise ValueError(f"Invalid operation handler: {handler}")
            return await handler(**kwargs)
        except ChipStackError as e:  # chipstack-ok
            logging.error(f"Operation {handler} failed: {str(e)}")
            logging.error(f"Arguments: {kwargs}")
            raise

    async def _verify_empty_tuple(self, attr_path, read_request):
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
            returned_clusters = sorted(list(read_request.tlvAttributes[endpoint].keys()))
            server_list = sorted(read_request.tlvAttributes[endpoint][Clusters.Descriptor.id]
                                 [Clusters.Descriptor.Attributes.ServerList.attribute_id])
            asserts.assert_equal(returned_clusters, server_list)

        # Verify all endpoints and clusters
        await self.verify_all_endpoints_clusters(read_request)

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
            TestStep(21, "TH should have access to only a single cluster at one Endpoint1. TH sends a Read Request Message to the DUT to read all attributes from all clusters at Endpoint1, AttributePath = [[Endpoint = Specific Endpoint]] +",
                     "Verify that the DUT sends back data of all attributes only from that one cluster to which it has access. Verify that there are no errors sent back for attributes the TH has no access to."),
            TestStep(22, "TH sends a Read Request Message to read all events and attributes from the DUT.",
                     "Verify that the DUT sends back data of all attributes and events that the TH has access to."),
        ]
        return steps

    # Update the test method to call functions directly with explicit parameters
    @async_test_body
    async def test_TC_IDM_2_2(self):
        # Test Setup with robust endpoint/cluster discovery
        await self.setup_class_helper(allow_pase=False)

        self.step(1)
        await self._read_single_attribute(
            endpoint=self.endpoint,
            attribute=Clusters.Descriptor.Attributes.ServerList)

        self.step(2)
        await self._read_all_cluster_attributes(
            endpoint=self.endpoint,
            cluster=Clusters.Descriptor)

        self.step(3)
        await self._read_attribute_all_endpoints(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList)

        self.step(4)
        await self._read_global_attribute(
            endpoint=self.endpoint,
            attribute_id=global_attribute_ids.GlobalAttributeIds.ATTRIBUTE_LIST_ID)

        self.step(5)
        await self._read_all_attributes()

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
            asserts.assert_equal((read_request14[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion] + 1), data_version14,
                                 "DataVersion was not incremented")

        self.step(15)
        read_request15, filtered_read15 = await self._read_data_version_filter(
            endpoint=self.endpoint,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.NodeLabel,
            test_value="Goodbye World")
        if filtered_read15 and 0 in filtered_read15:
            data_version15 = filtered_read15[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
            asserts.assert_equal(filtered_read15[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel],
                                 "Goodbye World", "Data version does not match expected value")
            asserts.assert_equal((read_request15[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion] + 1), data_version15,
                                 "DataVersion was not incremented")

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
        try:
            device = await self.default_controller.GetConnectedDevice(
                nodeid=self.dut_node_id,
                allowPASE=False,
                timeoutMs=1000,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD
            )
        except TimeoutError:
            logging.error("Unable to establish a CASE session over TCP to the device")
            raise
        asserts.assert_equal(device.isSessionOverTCPConnection, True,
                             "Session does not have associated TCP connection")
        asserts.assert_equal(device.sessionAllowsLargePayload, True,
                             "Session does not support large payloads")
        await self.default_controller.ReadAttribute(self.dut_node_id, ([]),
                                                    payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)

        self.step(19)
        await self._read_non_global_attribute(
            endpoint=self.endpoint,
            attribute=Clusters.Descriptor.Attributes.ServerList)

        self.step(20)
        await self._read_non_global_attribute(
            attribute=Clusters.Descriptor.Attributes.ServerList)

        self.step(21)
        original_acl21, read_request21 = await self._read_limited_access(
            endpoint=self.endpoint,
            cluster_id=Clusters.BasicInformation.id,
            subject_id=self.matter_test_config.controller_node_id + 1)
        asserts.assert_true(self.endpoint in read_request21.attributes, f"Endpoint {self.endpoint} missing in response")
        asserts.assert_true(Clusters.AccessControl in read_request21.attributes[self.endpoint],
                            "Clusters.AccessControl not in response")
        asserts.assert_true(Clusters.AccessControl.Attributes.Acl in read_request21.attributes[self.endpoint][Clusters.AccessControl],
                            "ACL attribute not in response")

        self.step(22)
        read_request22 = await self._read_all_events_attributes()
        required_attributes = ["Header", "Status", "Data"]
        for event in read_request22.events:
            for attr in required_attributes:
                asserts.assert_true(hasattr(event, attr), f"{attr} not in event")


if __name__ == "__main__":
    default_matter_test_main()
