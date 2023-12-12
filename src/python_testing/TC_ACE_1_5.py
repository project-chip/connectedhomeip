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
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

class TC_ACE_1_5(MatterBaseTest):

    def OpenCommissioningWindow(self) -> CommissioningParameters:
        try:
            params = self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id, timeout=600, iteration=10000, discriminator=self.matter_test_config.discriminators[0], option=1)
            time.sleep(5)
            return params

        except Exception as e:
            logging.exception('Error running OpenCommissioningWindow %s', e)
            asserts.assert_true(False, 'Failed to open commissioning window')
    
    # async def CommissionToStageSendCompleteAndCleanup(
    #         self, stage: int, expectedErrorPart: chip.native.ErrorSDKPart, expectedErrCode: int):

    #     logging.info("-----------------Fail on step {}-------------------------".format(stage))
    #     params = self.OpenCommissioningWindow()
    #     self.th2.ResetTestCommissioner()
    #     # This will run the commissioning up to the point where stage x is run and the
    #     # response is sent before the test commissioner simulates a failure
    #     self.th2.SetTestCommissionerPrematureCompleteAfter(stage)
    #     errcode = self.th2.CommissionOnNetwork(
    #         nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
    #         filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.matter_test_config.discriminators[0])
    #     logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
    #     asserts.assert_false(errcode.is_success, 'Commissioning complete did not error as expected')
    #     asserts.assert_true(errcode.sdk_part == expectedErrorPart, 'Unexpected error type returned from CommissioningComplete')
    #     asserts.assert_true(errcode.sdk_code == expectedErrCode, 'Unexpected error code returned from CommissioningComplete')
    #     revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
    #     await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
    #     # The failsafe cleanup is scheduled after the command completes, so give it a bit of time to do that
    #     time.sleep(1)

    async def read_currentfabricindex_expected_success(self, th) -> str:
        cluster = Clusters.Objects.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        current_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
        print("Current Fabric Index: %s" % current_fabric_index)
        return "100"

    @async_test_body
    async def test_TC_ACE_1_5(self):
        self.print_step(1, "Comissioning, already done ")
        self.th1 = self.default_controller       

        #new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        #new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)
        #TH2 = new_fabric_admin.NewController(nodeId = 112233)

        self.print_step(2, "TH1 opens the commissioning window on the DUT")
        params = self.OpenCommissioningWindow()
        #dev_ctrl = self.default_controller
        #params = dev_ctrl.OpenComissioningWindow(nodeid=self.dut_node_id, timeout=900, iteration=1000, discriminator=1234, option=1)


        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)
        TH2_nodeid = self.matter_test_config.controller_node_id + 2
        self.th2 = new_fabric_admin.NewController(nodeId=TH2_nodeid,
                                         paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))

        errcode = self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.matter_test_config.discriminators[0])
        logging.info('Commissioning complete done. Successful? {}, errorcode = {}'.format(errcode.is_success, errcode))
        self.print_step(3, "TH2 commissions DUT using admin node ID N2")
        
        self.print_step(4, "TH2 Reads CurrentFabricIndex ")
        th2FabricIndex = await self.read_currentfabricindex_expected_success(self.th2)
        print(th2FabricIndex)

if __name__ == "__main__":
    default_matter_test_main()
