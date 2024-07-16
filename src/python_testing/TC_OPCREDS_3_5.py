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
from chip.utils import CommissioningBuildingBlocks
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

class TC_OPCREDS_3_5(MatterBaseTest):

    @async_test_body
    async def test_TC_OPCREDS_3_5(self):
        opcreds = Clusters.OperationalCredentials
        
        self.print_step(0, "Preconditions")

        self.print_step(1, "TH1 fully commissions the DUT")
        dev_ctrl = self.default_controller

        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th1_vid = 0xFFF1
        th1_fabricId = 1111
        th1_new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=th1_vid, fabricId=th1_fabricId)
        th1_nodeId = self.default_controller.nodeId+1
        th1_dut_node_id = self.dut_node_id+1

        th1_new_fabric_ctrl = th1_new_fabric_admin.NewController(nodeId=th1_nodeId)
        success, nocResp, rcacResp = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=th1_new_fabric_ctrl,
            existingNodeId=self.dut_node_id, newNodeId=th1_dut_node_id)

        self.print_step(2, "TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a fabric-filtered read")
        nocs = await self.read_single_attribute_check_success(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=True)


if __name__ == "__main__":
    default_matter_test_main()
