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


import inspect
from enum import IntFlag

import chip.clusters as Clusters
import global_attribute_ids
from basic_composition_support import BasicCompositionTests
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import AttributePath, TypedAttributePath
from chip.clusters.ClusterObjects import ClusterObject
from chip.clusters.enum import MatterIntEnum
from chip.interaction_model import InteractionModelError, Status
from chip.tlv import uint
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_IDM_2_2(MatterBaseTest, BasicCompositionTests):

    ROOT_NODE_ENDPOINT_ID = 0

    @staticmethod
    def get_typed_attribute_path(attribute: Clusters, ep: int = ROOT_NODE_ENDPOINT_ID):
        return TypedAttributePath(
            Path=AttributePath(
                EndpointId=ep,
                Attribute=attribute
            )
        )

    def all_type_attributes_for_cluster(self, cluster: ClusterObjects.Cluster, desired_type: type) -> list[ClusterObjects.ClusterAttributeDescriptor]:
        all_attributes = [attribute for attribute in cluster.Attributes.__dict__.values() if inspect.isclass(
            attribute) and issubclass(attribute, ClusterObjects.ClusterAttributeDescriptor)]

        # Hackish way to get enums to return properly -- the default behavior (under else block) returns a BLANK LIST without this workaround
        # If type(attribute.attribute_type.Type) or type(ClusterObjects.ClusterObjectFieldDescriptor(Type=desired_type).Type are enums, they return <class 'aenum._enum.EnumType'>, which are equal!
        if desired_type == MatterIntEnum:
            all_attributes_of_type = [attribute for attribute in all_attributes if type(
                attribute.attribute_type.Type) == type(ClusterObjects.ClusterObjectFieldDescriptor(Type=desired_type).Type)]
        else:
            all_attributes_of_type = [attribute for attribute in all_attributes if attribute.attribute_type ==
                                      ClusterObjects.ClusterObjectFieldDescriptor(Type=desired_type)]

        return all_attributes_of_type

    def all_attributes_for_cluster(self, cluster: ClusterObjects.Cluster, inverted: bool = False) -> list[ClusterObjects.ClusterAttributeDescriptor]:
        if not inverted:
            all_attributes = [(attribute, attribute.attribute_type) for attribute in cluster.Attributes.__dict__.values(
            ) if inspect.isclass(attribute) and issubclass(attribute, ClusterObjects.ClusterAttributeDescriptor)]
        else:
            all_attributes = [(attribute, attribute.attribute_type) for attribute in cluster.Attributes.__dict__.values(
            ) if inspect.isclass(attribute) and not (issubclass(attribute, ClusterObjects.ClusterAttributeDescriptor))]
        return all_attributes

    def all_attributes(self, inverted: bool = False) -> list[ClusterObjects.ClusterAttributeDescriptor]:
        cluster_ids = Clusters.ClusterObjects.ALL_ATTRIBUTES.keys()
        attribute_list = []
        for cluster_id in cluster_ids:

            cluster = Clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]
            cluster_attributes = self.all_attributes_for_cluster(cluster, inverted=inverted)
            attribute_list.extend(cluster_attributes)
        return attribute_list

    async def get_cluster_from_type(self, cluster_type: type) -> None:
        for cluster in self.all_supported_clusters:
            all_types = self.all_type_attributes_for_cluster(cluster, cluster_type)
            if all_types:
                chosen_cluster = all_types[0]
                break
        else:
            print(f"Attribute not found on device: {cluster_type}")
            chosen_cluster = None

        if chosen_cluster:
            read_request = await self.default_controller.ReadAttribute(self.dut_node_id, [chosen_cluster])
            for endpoint in read_request:
                nested_output = read_request[endpoint]
                for cluster in nested_output:
                    attributes = nested_output[cluster]
                    asserts.assert_true(isinstance(attributes[chosen_cluster], cluster_type),
                                        f"{chosen_cluster} is not a {cluster_type}")
        return

    @async_test_body
    async def test_TC_IDM_2_2(self):

        self.user_params["use_pase_only"] = False
        super().setup_class()
        await self.setup_class_helper()
        # Test Setup

        all_clusters = [cluster for cluster in Clusters.ClusterObjects.ALL_ATTRIBUTES]

        server_list_attr = Clusters.Objects.Descriptor.Attributes.ServerList
        server_list_attr_path = [(0, server_list_attr)]
        descriptor_obj = Clusters.Objects.Descriptor
        descriptor_obj_path = [(0, descriptor_obj)]
        attribute_list = Clusters.Objects.Descriptor.Attributes.AttributeList
        attribute_list_path = [0, attribute_list]
        self.all_supported_clusters = [cluster for cluster in Clusters.__dict__.values(
        ) if inspect.isclass(cluster) and issubclass(cluster, ClusterObjects.Cluster)]

        # # For str test
        # power_source = Clusters.Objects.PowerSource

        # # For uint test
        # microwave_oven_mode = Clusters.Objects.MicrowaveOvenMode

        # # For int test
        # device_energy_mgmt = Clusters.Objects.DeviceEnergyManagement

        # # For float test
        # unit_testing = Clusters.Objects.UnitTesting

        # # For list test
        # energy_evse_mode = Clusters.Objects.EnergyEvseMode

        # # For bitmap test
        # occupancy_sensing = Clusters.Objects.OccupancySensing

        self.print_step(0, "Commissioning - already done")

        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)])
        endpoints = list(wildcard_descriptor.keys())

        endpoints.sort()
        # non_existent_endpoint = next(i for i, e in enumerate(endpoints + [None]) if i != e)

        # Step 1

        # TH sends the Read Request Message to the DUT to read one attribute on a given cluster and endpoint.
        # AttributePath = [[Endpoint = Specific Endpoint, Cluster = Specific ClusterID, Attribute = Specific Attribute]]
        # On receipt of this message, DUT should send a report data action with the attribute value to the DUT

        self.print_step(1, "Send Request Message to read one attribute on a given cluster and endpoint")

        # endpoint = [(0, Clusters.Objects.Descriptor.Attributes.ServerList)]
        read_request_1 = await self.default_controller.ReadAttribute(self.dut_node_id, server_list_attr_path)
        returned_endpoints = read_request_1[0].keys()
        # returned_endpoints = dict_keys([<class 'chip.clusters.Objects.Descriptor'>])
        # Check if chip.clusters.Objects.Descriptor is in output
        asserts.assert_in(descriptor_obj, returned_endpoints, "Descriptor cluster not in output")
        # Check if ServerList is in nested output
        asserts.assert_in(server_list_attr, read_request_1[0][descriptor_obj], "ServerList not in output")

        # Step 2
        # TH sends the Read Request Message to the DUT to read all attributes on a given cluster and Endpoint
        # AttributePath = [[Endpoint = Specific Endpoint, Cluster = Specific ClusterID]]
        # On receipt of this message, DUT should send a report data action with the attribute value to the DUT.

        self.print_step(2, "Send Request Message to read all attributes on a given cluster and endpoint")
        # endpoint = [(0, Clusters.Objects.Descriptor)]
        read_request_2 = await self.default_controller.ReadAttribute(self.dut_node_id, descriptor_obj_path)
        returned_endpoints = read_request_2[0].keys()
        # returned_endpoints = dict_keys([<class 'chip.clusters.Objects.Descriptor'>])
        # Check if chip.clusters.Objects.Descriptor is in output
        asserts.assert_in(descriptor_obj, returned_endpoints, "Descriptor cluster not in output")

        # Step 3

        # TH sends the Read Request Message to the DUT to read an attribute from a cluster at all Endpoints
        # AttributePath = [[Cluster = Specific ClusterID, Attribute = Specific Attribute]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(3, "Send Request Message to read one attribute on a given cluster at all endpoints")
        # endpoint = [Clusters.Objects.Descriptor.Attributes.ServerList]
        read_request_3 = await self.default_controller.ReadAttribute(self.dut_node_id, [server_list_attr])
        for i in range(3):
            returned_endpoints = read_request_3[i].keys()

            # returned_endpoints = dict_keys([<class 'chip.clusters.Objects.Descriptor'>])
            # Check if chip.clusters.Objects.Descriptor is in output
            asserts.assert_in(descriptor_obj, returned_endpoints, "Descriptor cluster not in output")
            # Check if ServerList is in nested output
            asserts.assert_in(server_list_attr, read_request_3[i][descriptor_obj], "ServerList not in output")

        # Step 4

        # TH sends the Read Request Message to the DUT to read a global attribute from all clusters at that Endpoint
        # AttributePath = [[Endpoint = Specific Endpoint, Attribute = Specific Global Attribute]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        self.print_step(4, "Send Request Message to read one global attribute from all clusters at that endpoint")

        # endpoint = [0, Clusters.Objects.Descriptor.Attributes.AttributeList] # Is this a global attribute? Trial and error, but looks like it
        read_request_4 = await self.default_controller.ReadAttribute(self.dut_node_id, attribute_list_path)
        for i in range(3):
            returned_endpoints = read_request_4[i].keys()
            # Check if chip.clusters.Objects.Descriptor is in output
            asserts.assert_in(descriptor_obj, returned_endpoints, "Descriptor cluster not in output")
            # Check if AttributeList is in nested output
            asserts.assert_in(attribute_list, read_request_4[i][descriptor_obj], "AttributeList not in output")

        # Step 5
        # TH sends the Read Request Message to the DUT to read all attributes from all clusters on all Endpoints
        ### AttributePath = [[]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        self.print_step(5, "Send Request Message to read all attributes from all clusters on all endpoints")
        read_request_5 = await self.default_controller.ReadAttribute(self.dut_node_id, [()])

        for i in range(3):
            returned_endpoints = read_request_5[i].keys()
            # Check if chip.clusters.Objects.Descriptor is in output
            asserts.assert_in(descriptor_obj, returned_endpoints, "Descriptor cluster not in output")
            # Check if AttributeList is in nested output
            asserts.assert_in(attribute_list, read_request_5[i][descriptor_obj], "AttributeList not in output")

        # Step 6
        # TH sends the Read Request Message to the DUT to read a global attribute from all clusters at all Endpoints
        # AttributePath = [[Attribute = Specific Global Attribute]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        self.print_step(6, "Send Request Message to read one global attribute from all clusters on all endpoints")
        # endpoint = [Clusters.Objects.Descriptor.Attributes.AttributeList]
        read_request_6 = await self.default_controller.ReadAttribute(self.dut_node_id, [attribute_list])
        returned_endpoints = read_request_6[0].keys()
        for i in range(3):
            returned_endpoints = read_request_6[i].keys()
            # Check if chip.clusters.Objects.Descriptor is in output
            asserts.assert_in(descriptor_obj, returned_endpoints, "Descriptor cluster not in output")
            # Check if AttributeList is in nested output
            asserts.assert_in(attribute_list, read_request_6[i][descriptor_obj], "AttributeList not in output")

        # Step 7
        # TH sends the Read Request Message to the DUT to read all attributes from a cluster at all Endpoints
        # AttributePath = [[Cluster = Specific ClusterID]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(7, "Send Request Message to read all attributes from one cluster at all endpoints")
        # endpoint = [Clusters.Objects.Descriptor]
        read_request_7 = await self.default_controller.ReadAttribute(self.dut_node_id, [descriptor_obj])
        returned_endpoints = read_request_7[0].keys()
        for i in range(3):
            returned_endpoints = read_request_7[i].keys()
            # Check if chip.clusters.Objects.Descriptor is in output
            asserts.assert_in(descriptor_obj, returned_endpoints, "Descriptor cluster not in output")
            # Check if AttributeList is in nested output
            asserts.assert_in(attribute_list, read_request_7[i][descriptor_obj], "AttributeList not in output")

        # Step 8
        # TH sends the Read Request Message to the DUT to read all attributes from all clusters at one Endpoint
        # AttributePath = [[Endpoint = Specific Endpoint]]
        # On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(8, "Send Request Message to read all attributes from all clusters at one endpoint")
        read_request_8 = await self.default_controller.ReadAttribute(self.dut_node_id, [0])
        returned_endpoints = read_request_8[0].keys()

        # Check if chip.clusters.Objects.Descriptor is in output
        asserts.assert_in(descriptor_obj, returned_endpoints, "Descriptor cluster not in output")
        # Check if ServerList is in nested output
        asserts.assert_in(server_list_attr, read_request_1[0][descriptor_obj], "ServerList not in output")

        # Step 9
        # TH sends the Read Request Message to the DUT to read an attribute of data type bool.
        # If the device does not have an attribute of data type bool, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.

        self.print_step(9, "Read a read request of attribute type bool")
        await self.get_cluster_from_type(bool)

        # Step 10
        # TH sends the Read Request Message to the DUT to read an attribute of data type string.
        # If the device does not have an attribute of data type string, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(10, "Read a read request of attribute type string")
        await self.get_cluster_from_type(str)

        # Step 11
        # TH sends the Read Request Message to the DUT to read an attribute of data type unsigned integer.
        # If the device does not have an attribute of data type unsigned integer, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(11, "Read a read request of attribute type unsigned integer")
        await self.get_cluster_from_type(uint)

        # Step 12
        # TH sends the Read Request Message to the DUT to read an attribute of data type signed integer.
        # If the device does not have an attribute of data type signed integer, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(12, "Read a read request of attribute type signed integer")
        await self.get_cluster_from_type(int)

        # Step 13
        # TH sends the Read Request Message to the DUT to read an attribute of data type floating point.
        # If the device does not have an attribute of data type floating point, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(13, "Read a read request of attribute type floating point")
        await self.get_cluster_from_type(float)

        # Step 14
        # TH sends the Read Request Message to the DUT to read an attribute of data type Octet String.
        # If the device does not have an attribute of data type octet string, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(14, "Read a read request of attribute type octet string")
        await self.get_cluster_from_type(bytes)

        # Step 15
        # TH sends the Read Request Message to the DUT to read an attribute of data type Struct.
        # If the device does not have an attribute of data type struct, skip this step.

        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(15, "Read a read request of attribute type struct")
        await self.get_cluster_from_type(ClusterObject)

        # Step 16
        # TH sends the Read Request Message to the DUT to read an attribute of data type List.
        # If the device does not have an attribute of data type list, skip this step.
        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(16, "Read a read request of attribute type list")
        await self.get_cluster_from_type(list)

        # Step 17
        # TH sends the Read Request Message to the DUT to read an attribute of data type enum.
        # If the device does not have an attribute of data type enum, skip this step.
        # Verify on the TH that the DUT returns data from the expected attribute path.
        self.print_step(17, "Read a read request of attribute type enum")
        await self.get_cluster_from_type(MatterIntEnum)

        # Step 18
        # TH sends the Read Request Message to the DUT to read an attribute of data type bitmap.
        # If the device does not have an attribute of data type bitmap, skip this step.
        self.print_step(18, "Read a read request of attribute type bitmap")
        await self.get_cluster_from_type(IntFlag)

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
        asserts.assert_true(isinstance(result.Reason, InteractionModelError), msg="Unexpected success reading invalid endpoint")

        # Seems to return only {} -- Is this a failure or the unintended behavior?
        # read_request_19 = await self.default_controller.ReadAttribute(self.dut_node_id, [non_existent_endpoint])

        # Step 20
        # TH sends the Read Request Message to the DUT to read any attribute to an unsupported cluster.
        # DUT responds with the report data action.

        # Verify on the TH that the DUT sends the status code UNSUPPORTED_CLUSTER
        self.print_step(20, "Send the Read Request Message to the DUT to read any attribute to an unsupported cluster")

        for endpoint_id, endpoint in self.endpoints.items():
            print(endpoint_id, endpoint)
            for cluster_type, cluster in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

                all_clusters = set(list(ClusterObjects.ALL_CLUSTERS.keys()))
                dut_clusters = set(list(x.id for x in endpoint.keys()))

                unsupported = [id for id in list(all_clusters - dut_clusters) if global_attribute_ids.attribute_id_type(id)
                               == global_attribute_ids.AttributeIdType.kStandardNonGlobal]

                unsupported_attribute = (ClusterObjects.ALL_ATTRIBUTES[unsupported[0]])[0]

                if unsupported:
                    result = await self.read_single_attribute_expect_error(endpoint=endpoint_id, cluster=ClusterObjects.ALL_CLUSTERS[unsupported[0]], attribute=unsupported_attribute, error=Status.UnsupportedCluster)
                    asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                        msg="Unexpected success reading invalid cluster")

        # Step 21
        # TH sends the Read Request Message to the DUT to read an unsupported attribute
        # DUT responds with the report data action.
        self.print_step(21, "Send the Read Request Message to the DUT to read any attribute to an unsupported attribute")

        for endpoint_id, endpoint in self.endpoints.items():
            for cluster_type, cluster in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

            all_attrs = set(list(ClusterObjects.ALL_ATTRIBUTES[cluster_type.id].keys()))
            dut_attrs = set(cluster[cluster_type.Attributes.AttributeList])

            unsupported = [id for id in list(all_attrs - dut_attrs) if global_attribute_ids.attribute_id_type(id)
                           == global_attribute_ids.AttributeIdType.kStandardNonGlobal]

            if unsupported:
                result = await self.read_single_attribute_expect_error(endpoint=endpoint_id, cluster=cluster_type, attribute=ClusterObjects.ALL_ATTRIBUTES[cluster_type.id][unsupported[0]], error=Status.UnsupportedAttribute)
                asserts.assert_true(isinstance(result.Reason, InteractionModelError),
                                    msg="Unexpected success reading invalid attribute")

        # all_attributes = self.all_attributes()
        # all_attributes_set = set(x[0] for x in all_attributes)

        # supported_attributes = []
        # for cluster in read_request_5.values():
        #     cluster_attributes = [list(attr.keys()) for attr in cluster.values()]
        #     for cluster_obj in cluster_attributes:
        #         supported_attributes.extend(cluster_obj)

        # supported_attributes_set = set(supported_attributes)
        # all_attributes_set - supported_attributes_set
        # unsupported_attributes_set = all_attributes_set - supported_attributes_set
        # unsupported_attribute = list(unsupported_attributes_set)[0]

        # Seems to return only {} (like unsupported endpoints and clusters)
        # read_request_21 = await self.default_controller.ReadAttribute(self.dut_node_id, [unsupported_attribute])

        # Verify on the TH that the DUT sends the status code UNSUPPORTED_ATTRIBUTE

        # Step 22
        # TH sends the Read Request Message to the DUT to read an attribute
        # Repeat the above steps 3 times

        # On the TH verify the received Report data message has the right attribute values for all the 3 times.
        self.print_step(22, "Send the Read Request Message to the DUT 3 timesand check if they are correct each time")
        read_request_22_1 = await self.default_controller.ReadAttribute(self.dut_node_id, [server_list_attr])
        read_request_22_2 = await self.default_controller.ReadAttribute(self.dut_node_id, [server_list_attr])
        read_request_22_3 = await self.default_controller.ReadAttribute(self.dut_node_id, [server_list_attr])
        asserts.assert_equal(read_request_22_1, read_request_22_2)
        asserts.assert_equal(read_request_22_2, read_request_22_3)

        # Step 23
        # TH sends a Read Request Message to the DUT to read a particular attribute with the DataVersionFilter Field not set.
        # DUT sends back the attribute value with the DataVersion of the cluster.
        # TH sends a second read request to the same cluster with the DataVersionFilter Field set with the dataversion value received before.
        self.print_step(
            23, "Send the Read Request Message to the DUT to read a particular attribute with the DataVersionFilter Field not set")
        # Temporarily commented to avoid linter errors -- will revert once output value is known so that assertion can be used correctly
        # read_request_23 = await self.default_controller.ReadAttribute(self.dut_node_id, server_list_attr_path)
        # data_version = read_request_23[0][Clusters.Descriptor][Clusters.Attribute.DataVersion]
        # data_version_filter = [(0, Clusters.Descriptor, data_version)]

        # read_request_23_2 = await self.default_controller.ReadAttribute(self.dut_node_id, server_list_attr_path, dataVersionFilters=data_version_filter)
        # Seems to return {}?

        # DUT should not send a report data action with the attribute value to the TH if the data version is same as that requested.

        # Step 24
        # TH sends a Read Request Message to the DUT to read a particular attribute with the DataVersionFilter Field not set.
        # DUT sends back the attribute value with the DataVersion of the cluster.
        # TH sends a write request to the same cluster to write to any attribute.
        # TH sends a second read request to read an attribute from the same cluster with the DataVersionFilter Field set with the dataversion value received before.

        self.print_step(
            24, "Send the Read Request Message to the DUT to read a particular attribute with the DataVersionFilter Field not set")
        # Temporarily commented to avoid linter errors -- will revert once output value is known so that assertion can be used correctly
        # read_request_24 = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.Objects.Descriptor.Attributes.FeatureMap)])
        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.Objects.Descriptor.Attributes.FeatureMap(value=123456))])

        # data_version = read_request_24[0][Clusters.Descriptor][Clusters.Attribute.DataVersion]
        # data_version_filter = [(0, Clusters.Descriptor, data_version)]
        # read_request_24_1 = await self.default_controller.ReadAttribute(
        #     self.dut_node_id, [(0, Clusters.Objects.Descriptor.Attributes.FeatureMap)], dataVersionFilters=data_version_filter)
        # Seems to return {}?

        # DUT should send a report data action with the attribute value to the TH.

        # Step 25

        # TH sends a Read Request Message to the DUT to read all attributes on a cluster with the DataVersionFilter Field not set.
        # DUT sends back the all the attribute values with the DataVersion of the cluster.
        # TH sends a write request to the same cluster to write to any attribute.
        # TH sends a second read request to read all the attributes from the same cluster with the DataVersionFilter Field set with the dataversion value received before.

        # DUT should send a report data action with all the attribute values to the TH.
        self.print_step(
            25, "Send the Read Request Message to the DUT to read all attributes on a cluster with the DataVersionFilter Field not set")
        # Temporarily commented to avoid linter errors -- will revert once output value is known so that assertion can be used correctly
        # read_request_25 = await self.default_controller.ReadAttribute(self.dut_node_id, descriptor_obj_path)
        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.Objects.Descriptor.Attributes.FeatureMap(value=654321))])
        # data_version = read_request_25[0][Clusters.Descriptor][Clusters.Attribute.DataVersion]
        # data_version_filter = [(0, Clusters.Descriptor, data_version)]

        # read_request_25_2 = await self.default_controller.ReadAttribute(
        #     self.dut_node_id, [(0, Clusters.Objects.Descriptor.Attributes.FeatureMap)], dataVersionFilters=data_version_filter)
        # Seems to return {}?

        # Step 26
        # TH sends a Read Request Message to the DUT to read a particular attribute on a particular cluster with the DataVersionFilter Field not set.
        # DUT sends back the attribute value with the DataVersion of the cluster.
        # TH sends a read request to the same cluster to read any attribute with the right DataVersion(received in the previous step) and also an older DataVersion.
        # The Read Request Message should have 2 DataVersionIB filters.

        # DUT should send a report data action with the attribute value to the TH.
        self.print_step(
            26, "Send the Read Request Message to read a particular attribute on a particular cluster with the DataVersionFilter Field not set")
        # Temporarily commented to avoid linter errors
        # read_request_26 = await self.default_controller.ReadAttribute(self.dut_node_id, [server_list_attr])
        await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.Objects.Descriptor.Attributes.FeatureMap(value=999))])
        # data_version = read_request_26[0][Clusters.Descriptor][Clusters.Attribute.DataVersion]
        # Temporarily commented to avoid linter errors -- will revert once output value is known so that assertion can be used correctly
        # data_version_filter = [(0, Clusters.Descriptor, data_version)]
        # read_request_26_2 = await self.default_controller.ReadAttribute(
        #     self.dut_node_id, [(0, Clusters.Objects.Descriptor.Attributes.FeatureMap)], dataVersionFilters=data_version_filter)
        # data_version_filter_2 = [(0, Clusters.Descriptor, data_version-1)]
        # read_request_26_3 = await self.default_controller.ReadAttribute(
        #     self.dut_node_id, [(0, Clusters.Objects.Descriptor.Attributes.FeatureMap)], dataVersionFilters=data_version_filter_2)

        # Step 27

        # TH sends a Read Request Message to the DUT to read any supported attribute/wildcard on a particular cluster say A with the DataVersionFilter Field not set.
        # DUT sends back the attribute value with the DataVersion of the cluster A.
        # TH sends a Read Request Message to read any supported attribute/wildcard on cluster A and any supported attribute/wildcard on another cluster B.
        # DataVersionList field should only contain the DataVersion of cluster A.

        # Verify that the DUT sends a report data action with the attribute value from the cluster B to the TH.
        # Verify that the DUT does not send the attribute value from cluster A.
        self.print_step(
            27, "Send the Read Request Message to read any supported attribute/wildcard on a particular cluster say A with the DataVersionFilter Field not set")
        read_request_27_1_1 = await self.default_controller.ReadAttribute(self.dut_node_id, [server_list_attr])

        data_version_1 = read_request_27_1_1[0][Clusters.Descriptor][Clusters.Attribute.DataVersion]

        # data_version_filter_1 = [(0, Clusters.Descriptor, data_version_1)]

        read_request_27_2_1 = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.BasicInformation.Attributes.NodeLabel)])
        data_version_2 = read_request_27_2_1[0][Clusters.BasicInformation][Clusters.Attribute.DataVersion]
        # data_version_filter_2 = [(0, Clusters.Descriptor, data_version_2)]

        asserts.assert_not_equal(data_version_1, data_version_2)

        # Step 28

        # TH sends a Read Request Message to the DUT to read something(Attribute) which is larger than 1 MTU(1280 bytes) and per spec can be chunked +

        # Verify on the TH that the DUT sends a chunked data message with the SuppressResponse field set to False for all the messages except the last one.
        # Verify the last chunked message sent has the SuppressResponse field set to True.

        # self.print_step(28, "Send the Read Request Message to read something(Attribute) which is larger than 1 MTU(1280 bytes) and per spec can be chunked +")
        # This apparently already exists in TCP tests -- remove?

        # Step 29

        # TH sends a Read Request Message to the DUT to read a non global attribute from all clusters at that Endpoint
        # AttributePath = [[Endpoint = Specific Endpoint, Attribute = Specific Non Global Attribute]] +

        # On the TH verify that the DUT sends an error message and not the value of the attribute.
        self.print_step(29, "Send the Read Request Message to the DUT to read a non global attribute from all clusters at that Endpoint")
        found_non_global = False
        for endpoint_id, endpoint in self.endpoints.items():
            if not found_non_global:
                # global_attribute_ids.AttributeIdType.kStandardNonGlobal seems to be non-existent in chip-all-clusters-app
                # But kTest does exist -> Clusters.Objects.UnitTesting
                for cluster_type, cluster in endpoint.items():

                    if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                        # if global_attribute_ids.cluster_id_type(cluster_type.id) == global_attribute_ids.AttributeIdType.kStandardNonGlobal:
                        found_non_global = True

                        non_global_attr = list(ClusterObjects.ALL_ATTRIBUTES[cluster_type.id].values())[0]
                        output = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, non_global_attr)])
                        asserts.assert_true(isinstance(output[0][cluster_type][non_global_attr].Reason, InteractionModelError),
                                            msg="Unexpected success reading non-global attribute")
                        continue

        # Step 30

        # TH sends a Read Request Message to the DUT to read a non global attribute from all clusters at all Endpoints
        # AttributePath = [[Attribute = Specific Non Global Attribute]] +

        # On the TH verify that the DUT sends an error message and not the value of the attribute.
        self.print_step(30, "Send the Read Request Message to the DUT to read a non global attribute from all clusters at all Endpoints")
        found_non_global = False
        for endpoint_id, endpoint in self.endpoints.items():
            if not found_non_global:
                # global_attribute_ids.AttributeIdType.kStandardNonGlobal seems to be non-existent in chip-all-clusters-app
                # But kTest does exist -> Clusters.Objects.UnitTesting
                for cluster_type, cluster in endpoint.items():

                    if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                        # if global_attribute_ids.cluster_id_type(cluster_type.id) == global_attribute_ids.AttributeIdType.kStandardNonGlobal:
                        found_non_global = True
                        continue
                    # Temporarily commented to avoid linter errors -- will revert once output value is known so that assertion can be used correctly
                    # non_global_cluster = cluster_type
                    # read_request_30 = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, non_global_cluster)])
                    # Seems to return {}?
        # Step 31

        # TH should have access to only a single cluster at one Endpoint1.
        # TH sends a Read Request Message to the DUT to read all attributes from all clusters at Endpoint1
        # AttributePath = [[Endpoint = Specific Endpoint]] +

        # Verify that the DUT sends back data of all attributes only from that one cluster to which it has access.
        # Verify that there are no errors sent back for attributes the TH has no access to.
        self.print_step(31, "Send the Read Request Message to the DUT to read all attributes from all clusters at Endpoint1")
        read_request_31 = await self.default_controller.ReadAttribute(self.dut_node_id, [(1, descriptor_obj)])
        asserts.assert_true(1 in read_request_31, "Endpoint 1 missing in response")
        asserts.assert_true(descriptor_obj in read_request_31[1], "Clusters.Objects.Descriptor not in response")

        # Step 32

        # TH sends a Read Request Message to read all events and attributes from the DUT.

        # Verify that the DUT sends back data of all attributes and events that the TH has access to.

        # Clusters.Descriptor.Events doesn't seem to exist, despite https://project-chip.github.io/connectedhomeip-doc/testing/python.html#events suggesting it should


if __name__ == "__main__":
    default_matter_test_main()
