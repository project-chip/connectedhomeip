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
#     script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factoryreset: True
#     quiet: True
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
from chip.tlv import uint
from matter_testing_infrastructure.chip.testing.basic_composition import BasicCompositionTests
from matter_testing_infrastructure.chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_IDM_2_2(MatterBaseTest, BasicCompositionTests):

    async def all_type_attributes_for_cluster(self, cluster: ClusterObjects.Cluster, desired_type: type) -> list[ClusterObjects.ClusterAttributeDescriptor]:
        all_attributes = [attribute for attribute in cluster.Attributes.__dict__.values() if inspect.isclass(
            attribute) and issubclass(attribute, ClusterObjects.ClusterAttributeDescriptor)]

        # Enums class is derived from MatterIntEnum, so this is checked separately
        if desired_type == MatterIntEnum:
            all_attributes_of_type = [attribute for attribute in all_attributes if type(
                attribute.attribute_type.Type) == type(desired_type)]

        elif desired_type == IntFlag:
            attributes_class = getattr(cluster, 'Attributes')
            all_attributes_of_type = [attributes_class.FeatureMap]
        else:
            all_attributes_of_type = [attribute for attribute in all_attributes if attribute.attribute_type ==
                                      ClusterObjects.ClusterObjectFieldDescriptor(Type=desired_type)]

        return all_attributes_of_type

    def all_device_clusters(self) -> set:
        device_clusters = set()
        for endpoint in self.endpoints:
            device_clusters |= set(self.endpoints[endpoint].keys())
        return device_clusters

    def all_device_attributes(self) -> set:
        device_attributes = set()
        for endpoint in self.endpoints:
            for cluster in self.endpoints[endpoint]:
                device_attributes |= self.endpoints[endpoint][cluster].keys()
        return device_attributes

    def verify_attribute_list_cluster(self, read_request: dict, endpoint: int, cluster: ClusterObjects.Cluster):
        cluster_ids = [c.id for c in read_request[endpoint].keys()]
        asserts.assert_equal(
            sorted(read_request[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList]),
            sorted(cluster_ids),
            f"ServerList doesn't match the expected server list for endpoint {endpoint}"
        )

    async def check_attribute_read_for_type(self, desired_attribute_type: type) -> bool:
        # Get all clusters from device
        for cluster in self.device_clusters:
            output = await self.default_controller.Read(self.dut_node_id, [cluster])
            cluster_attributes = []
            for endpoint in output.attributes:
                cluster_attributes.extend(list(output.attributes[endpoint][cluster].keys()))

            all_types = await self.all_type_attributes_for_cluster(cluster, desired_attribute_type)

            all_types = list(set(all_types) & set(cluster_attributes))
            if all_types:
                chosen_attributes = all_types
                cluster = Clusters.ClusterObjects.ALL_CLUSTERS[chosen_attributes[0].cluster_id]
                break
        else:
            print(f"Attribute not found on device: {desired_attribute_type}")
            cluster = None

        endpoint = None
        for endpoint in self.endpoints:
            for cluster, clusterdata in self.endpoints[endpoint].items():
                all_types = await self.all_type_attributes_for_cluster(cluster, desired_attribute_type)
                attributes_of_type = set(all_types)
                attributes_of_type_on_device = attributes_of_type.intersection(set(clusterdata.keys()))
                if attributes_of_type_on_device:
                    return True
        return False

    async def read_unsupported_attribute(self):
        found_unsupported = False
        for endpoint_id, endpoint in self.endpoints.items():
            if found_unsupported:
                break
            for cluster_type, cluster in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                all_attrs = set(list(ClusterObjects.ALL_ATTRIBUTES[cluster_type.id].keys()))
                dut_attrs = set(cluster[cluster_type.Attributes.AttributeList])

                unsupported = [id for id in list(all_attrs - dut_attrs) if global_attribute_ids.attribute_id_type(id)
                               == global_attribute_ids.AttributeIdType.kStandardNonGlobal]
                if unsupported:
                    result = await self.read_single_attribute_expect_error(
                        endpoint=endpoint_id,
                        cluster=cluster_type,
                        attribute=ClusterObjects.ALL_ATTRIBUTES[cluster_type.id][unsupported[0]], error=Status.UnsupportedAttribute)
                    asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                        msg="Unexpected success reading invalid attribute")
                    found_unsupported = True
                    return

    async def read_attribute_request(self, attribute_path_list):
        """
        Helper function to send read attribute requests.
        """
        return await self.default_controller.Read(self.dut_node_id, attribute_path_list)

    async def print_read_attributes_step(self, step_num: int, step_msg: str, attr_path):
        self.print_step(step_num, step_msg)
        return await self.verify_attributes(attr_path)

    async def verify_attributes(self, attr_path: ClusterObject):
        read_request = await self.read_attribute_request(attr_path)

        if isinstance(attr_path[0], tuple):
            return await self._verify_attributes_with_tuple(attr_path, read_request)
        elif isinstance(attr_path, list):
            return await self._verify_attributes_with_list(attr_path, read_request)

        return read_request

    async def _verify_attributes_with_tuple(self, attr_path, read_request):
        if len(attr_path[0]) == 0:
            # NOTE: This is checked in its entirety in IDM-10.1
            return await self._verify_empty_tuple(attr_path, read_request)

        if len(attr_path[0]) == 2:
            return await self._verify_non_empty_tuple(attr_path, read_request)

    async def _verify_empty_tuple(self, attr_path, read_request):
        # Parts list validation
        parts_list_a = read_request.tlvAttributes[0][Clusters.Descriptor.id][Clusters.Descriptor.Attributes.PartsList.attribute_id]
        parts_list_b = self.endpoints[0][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]
        asserts.assert_equal(parts_list_a, parts_list_b, "Parts list is not the expected value")

        # Server list validation
        for endpoint in read_request.tlvAttributes:
            returned_clusters = sorted(list(read_request.tlvAttributes[endpoint].keys()))
            server_list = sorted(read_request.attributes[endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList])
            asserts.assert_equal(returned_clusters, server_list)

        self.verify_read_request(read_request)

    async def _verify_non_empty_tuple(self, attr_path, read_request):
        endpoint, cluster_obj = attr_path[0]
        cluster_ids = list(read_request.tlvAttributes[endpoint].keys())

        if hasattr(cluster_obj, 'cluster_id'):  # cluster_obj is an Attribute
            return await self._verify_attribute_cluster(read_request, endpoint, cluster_obj)

        return await self._verify_cluster_without_attribute(read_request, endpoint, cluster_obj)

    async def _verify_attribute_cluster(self, read_request, endpoint, cluster_obj):
        cluster = ClusterObjects.ALL_CLUSTERS[cluster_obj.cluster_id]
        attribute_ids = list(read_request.tlvAttributes[endpoint][cluster_obj.cluster_id].keys())

        asserts.assert_equal({cluster_obj.cluster_id}, read_request.tlvAttributes[endpoint].keys(),
                             f"Cluster not in output: {cluster_obj.cluster_id}")
        asserts.assert_equal(attribute_ids, [cluster_obj.attribute_id])
        asserts.assert_equal([cluster_obj.cluster_id], list(read_request.tlvAttributes[endpoint].keys()))

        server_list_attribute_ids = (read_request.tlvAttributes[endpoint][cluster_obj.cluster_id]
                                     [cluster.Attributes.ServerList.attribute_id])
        asserts.assert_equal(server_list_attribute_ids, sorted([x.id for x in self.endpoints[endpoint]]))

    async def _verify_cluster_without_attribute(self, read_request, endpoint, cluster_obj):
        cluster_ids = list(read_request.tlvAttributes[endpoint].keys())
        asserts.assert_in(cluster_obj.id, cluster_ids)

        server_list_attribute_ids = (read_request.tlvAttributes[endpoint][cluster_obj.id]
                                     [Clusters.Descriptor.Attributes.ServerList.attribute_id])
        asserts.assert_equal(server_list_attribute_ids, sorted([x.id for x in self.endpoints[endpoint]]))

    async def _verify_attributes_with_list(self, attr_path, read_request):
        if isinstance(attr_path[0], type):  # E.g. [Clusters.Descriptor]
            return await self._verify_cluster_list(attr_path, read_request)

        if isinstance(attr_path[0], AttributePath):
            return await self._verify_attribute_path_list(attr_path, read_request)

    async def _verify_attribute_path_list(self, attr_path, read_request):
        endpoint = attr_path[0].EndpointId
        endpoint_list = [endpoint] if endpoint is not None else list(self.endpoints.keys())

        for endpoint in endpoint_list:
            asserts.assert_in(Clusters.Descriptor.id,
                              read_request.tlvAttributes[endpoint].keys(), "Descriptor cluster not in output")
            asserts.assert_in(Clusters.Descriptor.Attributes.AttributeList.attribute_id,
                              read_request.tlvAttributes[endpoint][Clusters.Descriptor.id], "AttributeList not in output")

    async def _verify_cluster_list(self, attr_path, read_request):
        cluster_obj = attr_path[0]
        if hasattr(cluster_obj, 'cluster_id'):  # cluster_obj is an Attribute
            return
        for endpoint in read_request.tlvAttributes:
            cluster_ids = list(read_request.tlvAttributes[endpoint].keys())
            asserts.assert_in(cluster_obj.id, cluster_ids)

            returned_attributes = read_request.tlvAttributes[endpoint][cluster_obj.id][cluster_obj.Attributes.AttributeList.attribute_id]
            asserts.assert_equal(sorted(returned_attributes), sorted(read_request.tlvAttributes[endpoint][cluster_obj.id].keys()),
                                 "Expected attribute list doesn't match")

    async def verify_cluster_and_server_lists(self, read_request):
        for endpoint, endpoint_data in read_request.tlvAttributes.items():
            cluster_list = sorted(endpoint_data.keys())
            server_list = sorted(endpoint_data.get(Clusters.Descriptor.id, {})
                                .get(Clusters.Descriptor.Attributes.ServerList.attribute_id, []))
            asserts.assert_equal(cluster_list, server_list, f"Cluster list and server list do not match for endpoint {endpoint}")

    def verify_read_request(self, read_request):
        for endpoint in read_request.tlvAttributes:
            for cluster in read_request.tlvAttributes[endpoint]:
                returned_attrs = sorted([x for x in read_request.tlvAttributes[endpoint][cluster].keys()])
                attr_list = sorted([x for x in read_request.tlvAttributes[endpoint][cluster]
                                    [ClusterObjects.ALL_CLUSTERS[cluster].Attributes.AttributeList.attribute_id]
                                    if x != Clusters.UnitTesting.Attributes.WriteOnlyInt8u.attribute_id])
                asserts.assert_equal(returned_attrs, attr_list,
                                     f"Mismatch for {cluster} ({ClusterObjects.ALL_CLUSTERS[cluster]}) at endpoint {endpoint}")

    @async_test_body
    async def test_TC_IDM_2_2(self):
        # Test Setup
        await self.setup_class_helper(allow_pase=False)

        all_clusters = [cluster for cluster in Clusters.ClusterObjects.ALL_ATTRIBUTES]

        self.device_clusters = self.all_device_clusters()
        self.device_attributes = self.all_device_attributes()
        self.all_supported_clusters = [cluster for cluster in Clusters.__dict__.values(
        ) if inspect.isclass(cluster) and issubclass(cluster, ClusterObjects.Cluster)]

        self.print_step(0, "Commissioning - already done")

        # Step 1

        # TH sends the Read Request Message to the DUT to read one attribute on a given cluster and endpoint.
        # AttributePath = [[Endpoint = Specific Endpoint, Cluster = Specific ClusterID, Attribute = Specific Attribute]]
        # On receipt of this message, DUT should send a report data action with the attribute value to the DUT

        read_request = await self.print_read_attributes_step(1, "Send Request Message to read one attribute on a given cluster and endpoint", [(0, Clusters.Descriptor.Attributes.ServerList)])

        # Step 2
        # TH sends the Read Request Message to the DUT to read all attributes on a given cluster and Endpoint
        # AttributePath = [[Endpoint = Specific Endpoint, Cluster = Specific ClusterID]]
        # On receipt of this message, DUT should send a report data action with the attribute value to the DUT.

        read_request = await self.print_read_attributes_step(2, "Send Request Message to read all attributes on a given cluster and endpoint", [(0, Clusters.Descriptor)])

        # Step 3

        # TH sends the Read Request Message to the DUT to read an attribute from a cluster at all Endpoints
        # AttributePath = [[Cluster = Specific ClusterID, Attribute = Specific Attribute]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.

        read_request = await self.print_read_attributes_step(3, "Send Request Message to read one attribute on a given cluster at all endpoints", [Clusters.Descriptor])

        # Step 4

        # TH sends the Read Request Message to the DUT to read a global attribute from all clusters at that Endpoint
        # AttributePath = [[Endpoint = Specific Endpoint, Attribute = Specific Global Attribute]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        attribute_path = AttributePath(
            EndpointId=0,
            ClusterId=None,
            AttributeId=global_attribute_ids.GlobalAttributeIds.ATTRIBUTE_LIST_ID)

        read_request = await self.print_read_attributes_step(4, "Send Request Message to read one global attribute from all clusters at that endpoint", [attribute_path])

        # Step 5
        # TH sends the Read Request Message to the DUT to read all attributes from all clusters on all Endpoints
        ### AttributePath = [[]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        # NOTE: This is checked in its entirety in IDM-10.1
        read_request = await self.print_read_attributes_step(5, "Send Request Message to read all attributes from all clusters on all endpoints", [()])

        # Step 6
        # TH sends the Read Request Message to the DUT to read a global attribute from all clusters at all Endpoints
        # AttributePath = [[Attribute = Specific Global Attribute]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.

        self.print_step(6, "Send Request Message to read one global attribute from all clusters on all endpoints")

        attribute_path = AttributePath(
            EndpointId=None,
            ClusterId=None,
            AttributeId=global_attribute_ids.GlobalAttributeIds.ATTRIBUTE_LIST_ID)

        read_request = await self.print_read_attributes_step(6, "Send Request Message to read one global attribute from all clusters on all endpoints", [attribute_path])

        # Step 7
        # TH sends the Read Request Message to the DUT to read all attributes from a cluster at all Endpoints
        # AttributePath = [[Cluster = Specific ClusterID]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(7, "Send Request Message to read all attributes from one cluster at all endpoints")
        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [Clusters.Descriptor])

        for endpoint in read_request:
            asserts.assert_in(Clusters.Descriptor, read_request[endpoint].keys(), "Descriptor cluster not in output")
            asserts.assert_in(Clusters.Descriptor.Attributes.AttributeList,
                              read_request[endpoint][Clusters.Descriptor], "AttributeList not in output")
            # Is this an actual failure? Only cluster in every endpoint returned is Descriptor, but server lists in each endpoint suggest there should be more clusters
            # Commented for now
            # self.verify_attribute_list_cluster(read_request, endpoint, Clusters.Descriptor)

        # Step 8
        # TH sends the Read Request Message to the DUT to read all attributes from all clusters at one Endpoint
        # AttributePath = [[Endpoint = Specific Endpoint]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(8, "Send Request Message to read all attributes from all clusters at one endpoint")
        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [0])

        asserts.assert_in(Clusters.Descriptor, read_request[0].keys(), "Descriptor cluster not in output")
        asserts.assert_in(Clusters.Descriptor.Attributes.ServerList,
                          read_request[0][Clusters.Descriptor], "ServerList not in output")

        for cluster in read_request[0]:
            attribute_ids = [a.attribute_id for a in read_request[0]
                             [cluster].keys() if a != Clusters.Attribute.DataVersion]
            asserts.assert_equal(sorted(attribute_ids),
                                 sorted(read_request[0][cluster][cluster.Attributes.AttributeList]),
                                 "Expected attribute list does not match actual list for cluster {cluster} on endpoint 0")
            self.verify_attribute_list_cluster(read_request, 0, Clusters.Descriptor)

        # Step 9
        # TH sends the Read Request Message to the DUT to read an attribute of data type bool.
        # If the device does not have an attribute of data type bool, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(9, "Read a read request of attribute type bool")
        await self.check_attribute_read_for_type(bool)

        # Step 10
        # TH sends the Read Request Message to the DUT to read an attribute of data type string.
        # If the device does not have an attribute of data type string, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(10, "Read a read request of attribute type string")
        await self.check_attribute_read_for_type(str)

        # Step 11
        # TH sends the Read Request Message to the DUT to read an attribute of data type unsigned integer.
        # If the device does not have an attribute of data type unsigned integer, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(11, "Read a read request of attribute type unsigned integer")
        await self.check_attribute_read_for_type(uint)

        # Step 12
        # TH sends the Read Request Message to the DUT to read an attribute of data type signed integer.
        # If the device does not have an attribute of data type signed integer, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(12, "Read a read request of attribute type signed integer")
        await self.check_attribute_read_for_type(int)

        # Step 13
        # TH sends the Read Request Message to the DUT to read an attribute of data type floating point.
        # If the device does not have an attribute of data type floating point, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(13, "Read a read request of attribute type floating point")
        await self.check_attribute_read_for_type(float)

        # Step 14
        # TH sends the Read Request Message to the DUT to read an attribute of data type Octet String.
        # If the device does not have an attribute of data type octet string, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(14, "Read a read request of attribute type octet string")
        await self.check_attribute_read_for_type(bytes)

        # Step 15
        # TH sends the Read Request Message to the DUT to read an attribute of data type Struct.
        # If the device does not have an attribute of data type struct, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(15, "Read a read request of attribute type struct")
        await self.check_attribute_read_for_type(ClusterObject)

        # Step 16
        # TH sends the Read Request Message to the DUT to read an attribute of data type List.
        # If the device does not have an attribute of data type list, skip this step.
        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(16, "Read a read request of attribute type list")
        await self.check_attribute_read_for_type(list)

        # Step 17
        # TH sends the Read Request Message to the DUT to read an attribute of data type enum.
        # If the device does not have an attribute of data type enum, skip this step.
        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(17, "Read a read request of attribute type enum")
        await self.check_attribute_read_for_type(MatterIntEnum)

        # Step 18
        # TH sends the Read Request Message to the DUT to read an attribute of data type bitmap.
        # If the device does not have an attribute of data type bitmap, skip this step.
        self.print_step(18, "Read a read request of attribute type bitmap")

        await self.check_attribute_read_for_type(IntFlag)

        # Step 19
        # TH sends the Read Request Message to the DUT to read any attribute to an unsupported Endpoint.
        # DUT responds with the report data action.
        # Verify on the TH that the DUT sends the status code UNSUPPORTED_ENDPOINT

        self.print_step(19, "Send the Read Request Message to the DUT to read any attribute to an unsupported Endpoint")
        supported_endpoints = set(self.endpoints.keys())
        all_endpoints = set(range(max(supported_endpoints)+2))
        unsupported = list(all_endpoints - supported_endpoints)
        # Read descriptor
        result = await self.read_single_attribute_expect_error(endpoint=unsupported[0], cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.FeatureMap, error=Status.UnsupportedEndpoint)

        # Step 20
        # TH sends the Read Request Message to the DUT to read any attribute to an unsupported cluster.
        # DUT responds with the report data action.

        # Verify on the TH that the DUT sends the status code UNSUPPORTED_CLUSTER
        self.print_step(20, "Send the Read Request Message to the DUT to read any attribute to an unsupported cluster")
        all_clusters = set(list(ClusterObjects.ALL_CLUSTERS.keys()))

        for endpoint_id, endpoint in self.endpoints.items():
            dut_standard_clusters = set([x.id for x in endpoint.keys() if global_attribute_ids.cluster_id_type(
                x.id) == global_attribute_ids.ClusterIdType.kStandard])
            unsupported = [id for id in list(all_clusters - dut_standard_clusters) if global_attribute_ids.attribute_id_type(id)
                           == global_attribute_ids.AttributeIdType.kStandardNonGlobal]
            if unsupported:
                unsupported_attribute = (ClusterObjects.ALL_ATTRIBUTES[unsupported[0]])[0]

                result = await self.read_single_attribute_expect_error(endpoint=endpoint_id, cluster=ClusterObjects.ALL_CLUSTERS[unsupported[0]], attribute=unsupported_attribute, error=Status.UnsupportedCluster)
                asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                    msg="Unexpected success reading invalid cluster")
            else:
                print("This step is skipped because there are no unsupported clusters")

        # Step 21
        # TH sends the Read Request Message to the DUT to read an unsupported attribute
        # DUT responds with the report data action.
        # Verify on the TH that the DUT sends the status code UNSUPPORTED_ATTRIBUTE
        self.print_step(21, "Send the Read Request Message to the DUT to read any attribute to an unsupported attribute")
        await self.read_unsupported_attribute()

        # Step 22
        # TH sends the Read Request Message to the DUT to read an attribute
        # Repeat the above steps 3 times

        # On the TH verify the received Report data message has the right attribute values for all the 3 times.
        self.print_step(22, "Send the Read Request Message to the DUT 3 times and check if they are correct each time")
        read_request_1 = await self.default_controller.ReadAttribute(self.dut_node_id, [Clusters.Descriptor.Attributes.ServerList])
        read_request_2 = await self.default_controller.ReadAttribute(self.dut_node_id, [Clusters.Descriptor.Attributes.ServerList])
        read_request_3 = await self.default_controller.ReadAttribute(self.dut_node_id, [Clusters.Descriptor.Attributes.ServerList])
        asserts.assert_equal(read_request_1, read_request_2)
        asserts.assert_equal(read_request_2, read_request_3)

        # Step 23
        # TH sends a Read Request Message to the DUT to read a particular attribute with the DataVersionFilter Field not set.
        # DUT sends back the attribute value with the DataVersion of the cluster.
        # TH sends a second read request to the same cluster with the DataVersionFilter Field set with the dataversion value received before.
        # DUT should not send a report data action with the attribute value to the TH if the data version is same as that requested.
        self.print_step(
            23, "Send the Read Request Message to the DUT to read a particular attribute with the DataVersionFilter Field not set")
        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.Descriptor.Attributes.ServerList)])
        data_version = read_request[0][Clusters.Descriptor][Clusters.Attribute.DataVersion]
        data_version_filter = [(0, Clusters.Descriptor, data_version)]

        read_request_2 = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.Descriptor.Attributes.ServerList)], dataVersionFilters=data_version_filter)
        asserts.assert_equal(read_request_2, {})

        # Step 24
        # TH sends a Read Request Message to the DUT to read a particular attribute with the DataVersionFilter Field not set.
        # DUT sends back the attribute value with the DataVersion of the cluster.
        # TH sends a write request to the same cluster to write to any attribute.
        # TH sends a second read request to read an attribute from the same cluster with the DataVersionFilter Field set with the dataversion value received before.
        # DUT should send a report data action with the attribute value to the TH.

        self.print_step(
            24, "Send the Read Request Message to the DUT to read a particular attribute with the DataVersionFilter Field not set")

        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.BasicInformation.Attributes.NodeLabel)])
        data_version_1 = read_request[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
        node_label_value = "Hello World"
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.BasicInformation.Attributes.NodeLabel(value=node_label_value))])

        data_version = read_request[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
        data_version_filter = [(0, Clusters.BasicInformation, data_version)]
        read_request = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(0, Clusters.BasicInformation.Attributes.NodeLabel)], dataVersionFilters=data_version_filter)
        data_version_2 = read_request[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]

        asserts.assert_equal(read_request[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel],
                             node_label_value, f"Data version does not equal {node_label_value}")
        asserts.assert_equal((data_version_1 + 1), data_version_2, "DataVersion was not incremented")

        # Step 25

        # TH sends a Read Request Message to the DUT to read all attributes on a cluster with the DataVersionFilter Field not set.
        # DUT sends back the all the attribute values with the DataVersion of the cluster.
        # TH sends a write request to the same cluster to write to any attribute.
        # TH sends a second read request to read all the attributes from the same cluster with the DataVersionFilter Field set with the dataversion value received before.

        # DUT should send a report data action with all the attribute values to the TH.

        self.print_step(
            25, "Send the Read Request Message to the DUT to read all attributes on a cluster with the DataVersionFilter Field not set")

        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.BasicInformation)])
        data_version_1 = read_request[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
        node_label_value = "Goodbye World"
        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.BasicInformation.Attributes.NodeLabel(value=node_label_value))])
        data_version = read_request[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
        data_version_filter = [(0, Clusters.BasicInformation, data_version)]

        read_request = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(0, Clusters.BasicInformation)], dataVersionFilters=data_version_filter)
        data_version_2 = read_request[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
        asserts.assert_equal(read_request[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel],
                             node_label_value, f"Data version does not equal {node_label_value}")
        asserts.assert_equal((data_version_1 + 1), data_version_2, "DataVersion was not incremented")

        # Step 26
        # TH sends a Read Request Message to the DUT to read a particular attribute on a particular cluster with the DataVersionFilter Field not set.
        # DUT sends back the attribute value with the DataVersion of the cluster.
        # TH sends a read request to the same cluster to read any attribute with the right DataVersion(received in the previous step) and also an older DataVersion.
        # The Read Request Message should have 2 DataVersionIB filters.

        # DUT should send a report data action with the attribute value to the TH.
        self.print_step(
            26, "Send the Read Request Message to read a particular attribute on a particular cluster with the DataVersionFilter Field not set")
        node_label_value = "Hello World Again"
        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [Clusters.BasicInformation.Attributes.DataModelRevision])

        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.BasicInformation.Attributes.NodeLabel(value=node_label_value))])
        data_version = read_request[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
        data_version_filter = [(0, Clusters.BasicInformation, data_version)]
        read_request_2 = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(0, Clusters.BasicInformation.Attributes.NodeLabel)], dataVersionFilters=data_version_filter)
        data_version_filter_2 = [(0, Clusters.BasicInformation, data_version-1)]
        read_request_3 = await self.default_controller.ReadAttribute(
            self.dut_node_id, [(0, Clusters.BasicInformation.Attributes.NodeLabel)], dataVersionFilters=data_version_filter_2)
        asserts.assert_equal(read_request_3[0][Clusters.BasicInformation][Clusters.BasicInformation.Attributes.NodeLabel],
                             node_label_value, f"Data version does not equal {node_label_value}")

        # Step 27

        # TH sends a Read Request Message to the DUT to read any supported attribute/wildcard on a particular cluster say A with the DataVersionFilter Field not set.
        # DUT sends back the attribute value with the DataVersion of the cluster A.
        # TH sends a Read Request Message to read any supported attribute/wildcard on cluster A and any supported attribute/wildcard on another cluster B.
        # DataVersionList field should only contain the DataVersion of cluster A.

        # Verify that the DUT sends a report data action with the attribute value from the cluster B to the TH.
        # Verify that the DUT does not send the attribute value from cluster A.

        self.print_step(
            27, "Send the Read Request Message to read any supported attribute/wildcard on a particular cluster say A with the DataVersionFilter Field not set")
        read_request_27_1_1 = await self.default_controller.ReadAttribute(self.dut_node_id, [Clusters.Descriptor.Attributes.ServerList])

        data_version_1 = read_request_27_1_1[0][Clusters.Descriptor][Clusters.Attribute.DataVersion]
        read_request_27_2_1 = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.BasicInformation.Attributes.NodeLabel)])
        data_version_2 = read_request_27_2_1[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]

        asserts.assert_not_equal(data_version_1, data_version_2)

        # Step 28

        # TH sends a Read Request Message to the DUT to read something(Attribute) which is larger than 1 MTU(1280 bytes) and per spec can be chunked +

        # Verify on the TH that the DUT sends a chunked data message with the SuppressResponse field set to False for all the messages except the last one.
        # Verify the last chunked message sent has the SuppressResponse field set to True.

        self.print_step(
            28, "Send the Read Request Message to read something(Attribute) which is larger than 1 MTU(1280 bytes) and per spec can be chunked +")
        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, ([]))

        # Step 29

        # TH sends a Read Request Message to the DUT to read a non global attribute from all clusters at that Endpoint
        # AttributePath = [[Endpoint = Specific Endpoint, Attribute = Specific Non Global Attribute]] +

        # On the TH verify that the DUT sends an error message and not the value of the attribute.
        self.print_step(29, "Send the Read Request Message to the DUT to read a non global attribute from all clusters at that Endpoint")

        attribute_path = AttributePath(
            EndpointId=0,
            ClusterId=None,
            AttributeId=Clusters.Descriptor.Attributes.ServerList.attribute_id)
        # AttributeId=global_attribute_ids.GlobalAttributeIds.ATTRIBUTE_LIST_ID)
        try:
            read_request = await self.default_controller.ReadAttribute(
                self.dut_node_id,
                [attribute_path]
            )
        except ChipStackError as e:
            asserts.assert_equal(e.err, 0x580,
                                 "Incorrect error response for reading non-global attribute on all clusters at endpoint 0")

        # Step 30

        # TH sends a Read Request Message to the DUT to read a non global attribute from all clusters at all Endpoints
        # AttributePath = [[Attribute = Specific Non Global Attribute]] +

        # On the TH verify that the DUT sends an error message and not the value of the attribute.
        self.print_step(30, "Send the Read Request Message to the DUT to read a non global attribute from all clusters at all Endpoints")

        endpoint = 0
        read_request = await self.default_controller.Read(self.dut_node_id,
                                                          [(endpoint, Clusters.AccessControl.Attributes.Acl)],
                                                          )

        dut_acl_original = read_request.attributes[endpoint][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

        # Limited ACE for controller 2 with single cluster access
        ace = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(cluster=Clusters.BasicInformation.id)],
            subjects=[self.matter_test_config.controller_node_id + 1])

        dut_acl = copy.deepcopy(dut_acl_original)
        dut_acl.append(ace)

        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.AccessControl.Attributes.Acl(dut_acl))])
        read_request = await self.default_controller.Read(self.dut_node_id,
                                                          [(endpoint, Clusters.AccessControl.Attributes.Acl)],
                                                          )

        asserts.assert_equal(len(read_request.attributes[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]), 2)

        asserts.assert_equal(type(read_request.attributes[0][Clusters.AccessControl]
                             [Clusters.AccessControl.Attributes.Acl][0].targets), Clusters.Types.Nullable)
        asserts.assert_not_equal(type(read_request.attributes[0][Clusters.AccessControl]
                                 [Clusters.AccessControl.Attributes.Acl][1].targets), Clusters.Types.Nullable)
        asserts.assert_equal(len(read_request.tlvAttributes[0][31][0]), 2)

        # attribute_path = AttributePath(
        #     EndpointId=0,
        #     ClusterId=None,
        #     AttributeId=Clusters.Descriptor.Attributes.ServerList.attribute_id)

        # read_request = await self.default_controller.ReadAttribute(
        #     self.dut_node_id,
        #     [attribute_path]
        # )

        # Cleanup
        await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.AccessControl.Attributes.Acl(dut_acl_original))])
        read_request = await self.default_controller.Read(self.dut_node_id,
                                                          [(endpoint, Clusters.AccessControl.Attributes.Acl)],
                                                          )
        asserts.assert_equal(len(read_request.tlvAttributes[0][31][0]), 1)
        asserts.assert_equal(len(read_request.attributes[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]), 1)
        asserts.assert_equal(type(read_request.attributes[0][Clusters.AccessControl]
                             [Clusters.AccessControl.Attributes.Acl][0].targets), Clusters.Types.Nullable)

        # Step 31

        # TH should have access to only a single cluster at one Endpoint1.
        # TH sends a Read Request Message to the DUT to read all attributes from all clusters at Endpoint1
        # AttributePath = [[Endpoint = Specific Endpoint]] +

        # Verify that the DUT sends back data of all attributes only from that one cluster to which it has access.
        # Verify that there are no errors sent back for attributes the TH has no access to.
        self.print_step(31, "Send the Read Request Message to the DUT to read all attributes from all clusters at Endpoint1")

        read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [(1, Clusters.Descriptor)])
        asserts.assert_true(1 in read_request, "Endpoint 1 missing in response")
        asserts.assert_true(Clusters.Descriptor in read_request[1], "Clusters.Descriptor not in response")

        # Step 32

        # TH sends a Read Request Message to read all events and attributes from the DUT.

        # Verify that the DUT sends back data of all attributes and events that the TH has access to.
        read_request = await self.default_controller.Read(nodeid=self.dut_node_id, attributes=[()], events=[()])
        asserts.assert_true(hasattr(read_request, 'attributes'), 'attributes not in read_request')
        asserts.assert_true(hasattr(read_request, 'events'), 'events not in read_request')
        asserts.assert_true(hasattr(read_request, 'tlvAttributes'), 'tlvAttributes not in read_request')
        await self.verify_cluster_and_server_lists(read_request)
        required_attributes = ["Header", "Status", "Data"]

        for event in read_request.events:
            for attr in required_attributes:
                asserts.assert_true(hasattr(event, attr), f"{attr} not in event")


if __name__ == "__main__":
    default_matter_test_main()
