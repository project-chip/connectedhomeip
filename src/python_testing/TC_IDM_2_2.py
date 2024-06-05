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
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

class TC_IDM_2_2(MatterBaseTest):

    def steps_TC_IDM_2_2(self) -> list[TestStep]:
        steps = [TestStep(1, "Send Request Message to read one attribute on a given cluster and endpoint")] # Expand later once this is working
        return steps


    @async_test_body
    async def steps_TC_IDM_2_2(self):
        self.print_step(0, "Commissioning - already done")

        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)])

        ### TH sends the Read Request Message to the DUT to read one attribute on a given cluster and endpoint.
        ### AttributePath = [[Endpoint = Specific Endpoint, Cluster = Specific ClusterID, Attribute = Specific Attribute]]
        ### On receipt of this message, DUT should send a report data action with the attribute value to the DUT.
        self.print_step(1, "Send Request Message to read one attribute on a given cluster and endpoint")
        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)]) # Placeholder for now

        ### TH sends the Read Request Message to the DUT to read all attributes on a given cluster and Endpoint
        ### AttributePath = [[Endpoint = Specific Endpoint, Cluster = Specific ClusterID]]
        ###  On receipt of this message, DUT should send a report data action with the attribute value to the DUT.
        self.print_step(2, "Send Request Message to read all attributes on a given cluster and endpoint")
        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)]) # Placeholder for now

        ### TH sends the Read Request Message to the DUT to read an attribute from a cluster at all Endpoints
        ### AttributePath = [[Cluster = Specific ClusterID, Attribute = Specific Attribute]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(3, "Send Request Message to read one attribute on a given cluster at all endpoints")
        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)]) # Placeholder for now

        ### TH sends the Read Request Message to the DUT to read a global attribute from all clusters at that Endpoint
        ### AttributePath = [[Endpoint = Specific Endpoint, Attribute = Specific Global Attribute]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        self.print_step(4, "Send Request Message to read one global attribute from all clusters at that endpoint")
        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)]) # Placeholder for now

        ### TH sends the Read Request Message to the DUT to read all attributes from all clusters on all Endpoints 
        ### AttributePath = [[]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        self.print_step(5, "Send Request Message to read all attributes from all clusters on all endpoints")
        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)]) # Placeholder for now

        ### TH sends the Read Request Message to the DUT to read a global attribute from all clusters at all Endpoints
        ### AttributePath = [[Attribute = Specific Global Attribute]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the clusters to the DUT.
        self.print_step(6, "Send Request Message to read one global attribute from all clusters on all endpoints")
        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)]) # Placeholder for now

        ### TH sends the Read Request Message to the DUT to read all attributes from a cluster at all Endpoints
        ### AttributePath = [[Cluster = Specific ClusterID]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(7, "Send Request Message to read all attributes from one cluster at all endpoints")
        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)]) # Placeholder for now

        ### TH sends the Read Request Message to the DUT to read all attributes from all clusters at one Endpoint
        ### AttributePath = [[Endpoint = Specific Endpoint]]
        ### On receipt of this message, DUT should send a report data action with the attribute value from all the Endpoints to the DUT.
        self.print_step(8, "Send Request Message to read all attributes from all clusters at one endpoint")
        wildcard_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.Descriptor)]) # Placeholder for now


if __name__ == "__main__":
    default_matter_test_main()