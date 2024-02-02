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
from chip import ChipDeviceCtrl
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main


class TC_OPCREDS_3_2(MatterBaseTest):
    # All global functions
    @async_test_body
    async def test_TC_OPCREDS_3_2(self):
        self.print_step(1, "Factory Reset DUT")
        # self.th1 = self.default_controller

        # Create and setup TH1
        th1_ca = self.certificate_authority_manager.NewCertificateAuthority()
        th1_fabric_admin = th1_ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=1111)
        th1_nodeId = self.default_controller.nodeId+1
        self.th1 = th1_fabric_admin.NewController(nodeId=th1_nodeId)

        # Commissioning Process
        newNodeId = self.dut_node_id+1
        params = self.openCommissioningWindow(self.default_controller, self.dut_node_id)
        errcode = self.th1.CommissionOnNetwork(
            nodeId=newNodeId, setupPinCode=params.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=params.randomDiscriminator)
        logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
        self.print_step(3, "TH0 commissions DUT")

        # NOCResponse is need it
        opcreds = Clusters.OperationalCredentials
        nonce = random.randbytes(32)
        cmd = opcreds.Commands.CSRRequest(CSRNonce=nonce, isForUpdateNOC=False)
        resp = await self.send_single_cmd(dev_ctrl=self.th1, node_id=newNodeId, cmd=cmd)
        # resp = await self.th1.SendCommand(nodeid=newNodeId, endpoint=0, payload=cmd)
        print("HELLO HERE IS")
        print(resp)
        print("HELLO ABOVE MUST BE THERE")


if __name__ == "__main__":
    default_matter_test_main()
