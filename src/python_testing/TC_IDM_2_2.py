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
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

class TC_IDM_2_2(MatterBaseTest):

    ROOT_NODE_ENDPOINT_ID = 0

    @staticmethod
    def get_typed_attribute_path(attribute, ep=ROOT_NODE_ENDPOINT_ID):
        return TypedAttributePath(
            Path=AttributePath(
                EndpointId=ep,
                Attribute=attribute
            )
        )

    @async_test_body
    async def test_TC_IDM_2_2(self):
        # Test Setup
        cluster_rev_attr = Clusters.BasicInformation.Attributes.ClusterRevision # Global attribute
        cluster_rev_attr_path = [(cluster_rev_attr)]
        cluster_rev_attr_typed_path = self.get_typed_attribute_path(cluster_rev_attr)
        node_label_attr = Clusters.BasicInformation.Attributes.NodeLabel # Borrowed from TC_IDM_4_2
        node_label_attr_path = [(0, node_label_attr)]
        node_label_attr_typed_path = self.get_typed_attribute_path(node_label_attr)
        node_label_attr_all = Clusters.BasicInformation
        node_label_attr_all_path = [(0, node_label_attr_all)]
        data_version_attr = Clusters.Attribute.DataVersion
        basic_info_attr = Clusters.Objects.BasicInformation
        attribute_list_attr = Clusters.Objects.BasicInformation.Attributes.AttributeList
        cluster_revision_attr = Clusters.Objects.BasicInformation.Attributes.ClusterRevision
        network_diagnostics_attr = Clusters.Objects.ThreadNetworkDiagnostics
        unit_testing_attr = Clusters.Objects.UnitTesting
        
        self.print_step(0, "Commissioning - already done")
        self.print_step(1, "Send Request Message to read one attribute on a given cluster and endpoint")
        
        # TH sends the Read Request Message to the DUT to read one attribute on a given cluster and endpoint.
        # AttributePath = [[Endpoint = Specific Endpoint, Cluster = Specific ClusterID, Attribute = Specific Attribute]]
        # On receipt of this message, DUT should send a report data action with the attribute value to the DUT
        
        read_request_1 = await self.default_controller.ReadAttribute(self.dut_node_id, node_label_attr_path) # [(0, Clusters.BasicInformation.Attributes.NodeLabel)]
        attributes = read_request_1[0]
        basic_information = attributes[basic_info_attr]
        data_version = basic_information[data_version_attr]
    
        asserts.assert_in(node_label_attr, basic_information, "NodeLabel not in BasicInformation")
        asserts.assert_in(data_version_attr, basic_information, "DataVersion not in BasicInformation")

        ### TH sends the Read Request Message to the DUT to read all attributes on a given cluster and Endpoint
        ### AttributePath = [[Endpoint = Specific Endpoint, Cluster = Specific ClusterID]]
        ### On receipt of this message, DUT should send a report data action with the attribute value to the DUT.

        self.print_step(2, "Send Request Message to read all attributes on a given cluster and endpoint")
        asserts.assert_in(node_label_attr, basic_information, "AttributeList not in attributes")
        read_request_2 = await self.default_controller.ReadAttribute(self.dut_node_id, node_label_attr_all_path) # [(0, Clusters.BasicInformation)]
        attributes = read_request_2[0]
        basic_information = attributes[basic_info_attr]
        data_version = basic_information[data_version_attr]
        asserts.assert_in(node_label_attr, basic_information, "NodeLabel not in BasicInformation")
        asserts.assert_in(data_version_attr, basic_information, "DataVersion not in BasicInformation")
        # The output from this command gets many more values from basic_info_attr compared to the first test -- get attribute list, expand test if needed
        asserts.assert_in(attribute_list_attr, basic_information, "AttributeList not in BasicInformation")
        attribute_list = basic_information[attribute_list_attr]

        ### TH sends the Read Request Message to the DUT to read an attribute from a cluster at all Endpoints
        ### AttributePath = [[Cluster = Specific ClusterID, Attribute = Specific Attribute]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(3, "Send Request Message to read one attribute on a given cluster at all endpoints")
        read_request_3 = await self.default_controller.ReadAttribute(self.dut_node_id, [node_label_attr]) # [Clusters.BasicInformation.Attributes.NodeLabel]
        attributes = read_request_3[0]
        asserts.assert_in(node_label_attr, basic_information, "NodeLabel not in BasicInformation")

        ### TH sends the Read Request Message to the DUT to read a global attribute from all clusters at that Endpoint
        ### AttributePath = [[Endpoint = Specific Endpoint, Attribute = Specific Global Attribute]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        self.print_step(4, "Send Request Message to read one global attribute from all clusters at that endpoint")
        # The output from this command gets many more values from unit_testing_attr compared to the some other tests
        read_request_4 = await self.default_controller.ReadAttribute(self.dut_node_id, cluster_rev_attr_path) # [(Clusters.BasicInformation.Attributes.ClusterRevision)]
        attributes = read_request_4[0]
        basic_information = attributes[basic_info_attr]
        asserts.assert_true(basic_information[cluster_revision_attr], 3)

        ### TH sends the Read Request Message to the DUT to read all attributes from all clusters on all Endpoints 
        ### AttributePath = [[]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        self.print_step(5, "Send Request Message to read all attributes from all clusters on all endpoints")
        read_request_5 = await self.default_controller.ReadAttribute(self.dut_node_id, [()]) # This returns a dataclass key of 1, unlike 0 for the earlier ones
        attributes = read_request_5[1]
        unit_testing = attributes[unit_testing_attr]
        data_version = unit_testing[data_version_attr]

        # asserts.assert_in(node_label_attr, unit_testing, "NodeLabel not in UnitTesting")
        asserts.assert_in(data_version_attr, unit_testing, "DataVersion not in UnitTesting")
        ### TH sends the Read Request Message to the DUT to read a global attribute from all clusters at all Endpoints
        ### AttributePath = [[Attribute = Specific Global Attribute]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        self.print_step(6, "Send Request Message to read one global attribute from all clusters on all endpoints")
        read_request_6 = await self.default_controller.ReadAttribute(self.dut_node_id, [cluster_rev_attr]) # Clusters.BasicInformation.Attributes.ClusterRevision
        
        attributes = read_request_6[0]
        basic_information = attributes[basic_info_attr]
        data_version = basic_information[data_version_attr]
        # asserts.assert_in(node_label_attr, basic_information, "NodeLabel not in BasicInformation")
        asserts.assert_in(data_version_attr, basic_information, "DataVersion not in BasicInformation")
        asserts.assert_true(basic_information[cluster_revision_attr], 3)

        ### TH sends the Read Request Message to the DUT to read all attributes from a cluster at all Endpoints
        ### AttributePath = [[Cluster = Specific ClusterID]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(7, "Send Request Message to read all attributes from one cluster at all endpoints")
        read_request_7 = await self.default_controller.ReadAttribute(self.dut_node_id, [node_label_attr_all]) # [Clusters.BasicInformation]
        attributes = read_request_7[0]
        basic_information = attributes[basic_info_attr]
        data_version = basic_information[data_version_attr]
        asserts.assert_in(node_label_attr, basic_information, "NodeLabel not in BasicInformation")
        asserts.assert_in(data_version_attr, basic_information, "DataVersion not in BasicInformation")
        # The output from this command gets many more values from basic_info_attr compared to the first test -- get attribute list, expand test if needed
        asserts.assert_in(attribute_list_attr, basic_information, "AttributeList not in BasicInformation")

        ### TH sends the Read Request Message to the DUT to read all attributes from all clusters at one Endpoint
        ### AttributePath = [[Endpoint = Specific Endpoint]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(8, "Send Request Message to read all attributes from all clusters at one endpoint")
        read_request_8 = await self.default_controller.ReadAttribute(self.dut_node_id, [0])
        attributes = read_request_8[0]
        network_diagnostics = attributes[network_diagnostics_attr]
        data_version = network_diagnostics[data_version_attr]
        
        asserts.assert_in(node_label_attr, basic_information, "NodeLabel not in BasicInformation")
        asserts.assert_in(data_version_attr, network_diagnostics, "DataVersion not in ThreadNetworkDiagnostics")
        asserts.assert_in(attribute_list_attr, basic_information, "AttributeList not in BasicInformation")
    
if __name__ == "__main__":
    default_matter_test_main()