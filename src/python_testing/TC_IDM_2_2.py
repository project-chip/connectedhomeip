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


import chip.clusters as Clusters
from chip.clusters.Attribute import AttributePath, TypedAttributePath
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_IDM_2_2(MatterBaseTest):

    ROOT_NODE_ENDPOINT_ID = 0

    @staticmethod
    def get_typed_attribute_path(attribute: Clusters, ep: int=ROOT_NODE_ENDPOINT_ID):
        return TypedAttributePath(
            Path=AttributePath(
                EndpointId=ep,
                Attribute=attribute
            )
        )

    @async_test_body
    async def test_TC_IDM_2_2(self):
        # Test Setup
        cluster_rev_attr = Clusters.BasicInformation.Attributes.ClusterRevision  # Global attribute

        server_list_attr = Clusters.Objects.Descriptor.Attributes.ServerList
        server_list_attr_path = [(0, server_list_attr)]
        descriptor_obj = Clusters.Objects.Descriptor
        descriptor_obj_path = [(0, descriptor_obj)]
        attribute_list = Clusters.Objects.Descriptor.Attributes.AttributeList
        attribute_list_path = [0, attribute_list]
        
        self.print_step(0, "Commissioning - already done")

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
        attributes = read_request_7[0]
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

if __name__ == "__main__":
    default_matter_test_main()
