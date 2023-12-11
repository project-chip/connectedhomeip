#
#    Copyright (c) 2022 Project CHIP Authors
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

import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

class TC_ACE_1_5(MatterBaseTest):

    async def read_currentfabricindex_attribute(self, th, nodeid):
        #cluster = Clusters.Objects.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        await self.read_single_attribute(th, nodeid, 0, attribute)

    async def write_acl(self, acl):
        # This returns an attribute status
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")

    @async_test_body
    async def test_TC_ACE_1_5(self):
        self.print_step(1, "Comissioning, already done ")

        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]

        TH1_nodeid = self.matter_test_config.controller_node_id + 1
        TH2_nodeid = self.matter_test_config.controller_node_id + 2

        TH1 = fabric_admin.NewController(nodeId=TH1_nodeid,
                                         paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))
        TH2 = fabric_admin.NewController(nodeId=TH2_nodeid,
                                         paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))

        TH2FabricIndex = await self.read_currentfabricindex_attribute(TH2, TH2_nodeid)

        self.print_step(2, "TH1 writes DUT endpoint AccessControlEntryStruct")
        admin_acl = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex = 1,
            privilege = Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode = Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects = [],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.AccessControl.id)])
        all_view = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex = 1,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(endpoint=0, cluster=Clusters.Descriptor.id)])
        acl = [admin_acl, all_view]
        await self.write_acl(acl)

if __name__ == "__main__":
    default_matter_test_main()
