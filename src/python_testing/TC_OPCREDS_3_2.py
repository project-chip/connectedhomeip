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

import logging
import random

import chip.clusters as Clusters
from chip.utils import CommissioningBuildingBlocks
from chip import ChipDeviceCtrl
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_OPCREDS_3_2(MatterBaseTest):
    # All global functions
    @async_test_body
    async def test_TC_OPCREDS_3_2(self):

        

        # It is not necessary implement factory reset to DUT
        self.print_step(1, "Step 1: Factory Reset DUT")

        dev_ctrl = self.default_controller

        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th1_new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2222)
        th1_nodeid = self.default_controller.nodeId+1
        th1_dut_nodeid = self.dut_node_id+1

        th1_new_admin_ctrl = th1_new_fabric_admin.NewController(nodeId=th1_nodeid)
        resp = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=th1_new_admin_ctrl,
            existingNodeId=self.dut_node_id, newNodeId=th1_dut_nodeid
        )

        self.print_step(2, "Commission DUT to TH1's Fabric")

        # CommissioningBuildingBlocks just return a boolean. I need to get the NOC Response
        # so I just I will add a new step for now update the fabric label just to get the NOC response.
        opcreds = Clusters.OperationalCredentials


        nocResponse = await self.read_single_attribute_check_success(
            dev_ctrl=th1_new_admin_ctrl, node_id=th1_dut_nodeid, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=False)
        )

        print("Here is NOCResponse below")
        print(nocResponse)
        print("Here is NOCResponse")

        cmd = opcreds.Commands.UpdateFabricLabel(label="Label1")
        resp = await self.send_single_cmd(cmd=cmd, dev_ctrl=th1_new_admin_ctrl, node_id=th1_dut_nodeid)
        self.print_step(3, "When DUT sends NOC response save FabricIndex as FabricIndex_TH1")
        FabricIndex_TH1 = resp.fabricIndex

        self.print_step(4, "Save TH1's Fabric ID as FabricID1")
        TH1_CurrentFabricIndex = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.CurrentFabricIndex, dev_ctrl=th1_new_admin_ctrl, node_id=th1_dut_nodeid)

        self.print_step(5, "From TH1 read the CurrentFabricIndex")
        print(TH1_CurrentFabricIndex)

if __name__ == "__main__":
    default_matter_test_main()
