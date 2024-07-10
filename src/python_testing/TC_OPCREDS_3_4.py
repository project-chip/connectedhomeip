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

import random

import chip.clusters as Clusters
from chip.utils import CommissioningBuildingBlocks
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_OPCREDS_3_4(MatterBaseTest):
    # TODO: Implement

    @async_test_body
    async def test_TC_OPCREDS_3_4(self):
        # TODO: add steps
        self.print_step(0, "Preconditions")

        self.print_step(1, "TH1 fully commissions the DUT")
        opcreds = Clusters.OperationalCredentials
        dev_ctrl = self.default_controller

        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th1_vid = 0xFFF1
        th1_fabricId = 1111
        th1_new_fabric_admin = new_certificate_authority.NewFabricAdmin(
            vendorId=th1_vid, fabricId=th1_fabricId)
        th1_nodeId = self.default_controller.nodeId+1
        th1_dut_node_id = self.dut_node_id+1

        th1_new_fabric_ctrl = th1_new_fabric_admin.NewController(
            nodeId=th1_nodeId)
        success, nocResp, noc_original, rcac_original, icac_original = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=th1_new_fabric_ctrl,
            existingNodeId=self.dut_node_id, newNodeId=th1_dut_node_id
        )

        self.print_step(
            2, "TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a fabric-filtered read")
        nocs = await self.read_single_attribute_check_success(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=True)

        self.print_step(
            3, "TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster")
        trusted_root_original = await self.read_single_attribute_check_success(
            dev_ctrl=th1_new_fabric_ctrl,
            node_id=th1_dut_node_id, cluster=opcreds,
            attribute=opcreds.Attributes.TrustedRootCertificates)
        print("trusted_root_original: ", trusted_root_original)

        self.print_step(
            4, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster with the following fields: NOCValue and ICACValue")
        cmd = opcreds.Commands.UpdateNOC(
            NOCValue=noc_original, ICACValue=icac_original)
        try:
            await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
            asserts.fail("Unexpected error sending UpdateNOC command")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.FailsafeRequired, "Unexpected Failsafe status")

        self.print_step(
            5, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(
            expiryLengthSeconds=900)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        print(resp)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(
            6, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster with the following fields: NOCValue and ICACValue")
        cmd = opcreds.Commands.UpdateNOC(
            NOCValue=noc_original, ICACValue=icac_original)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kMissingCsr,
                             "Failure status returned from UpdateNOC")

        self.print_step(
            7, "TH1 Sends CSRRequest command with the IsForUpdateNOC field set to false")
        cmd = opcreds.Commands.CSRRequest(
            CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        csr_not_updated = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        print("csr_not_updated: ", csr_not_updated)

        self.print_step(
            8, "TH1 generates a new NOC chain with ICAC with the following properties")
        new_noc_chain = th1_new_fabric_ctrl.IssueNOCChain(
            csr_not_updated, th1_dut_node_id)
        noc_not_for_update = csr_not_updated.NOCSRElements
        icac_not_for_update = new_noc_chain.icacBytes
        # TODO: Ask to @Cecille if ICACvalue should be get from new NOCchain or must be get from somewhere else?

        self.print_step(
            9, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster")
        cmd = opcreds.Commands.UpdateNOC(
            NOCValue=noc_not_for_update, ICACValue=icac_not_for_update)
        try:
            resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
            asserts.fail("Unexpected error sending UpdateNOC command")
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status, Status.ConstraintError, "Failure status returned from UpdateNOC")

        self.print_step(
            10, "TH1 Sends CSRRequest command with the IsForUpdateNOC field set to true")
        # resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(300))
        # print("ArmFailSafe 600: ", resp)
        cmd = opcreds.Commands.CSRRequest(
            CSRNonce=random.randbytes(32), isForUpdateNOC=True)
        csr_update = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        # I don't know why but I received an FAILURE error if set the isForUpdateNOC with True instead if I use False I get  ConstraintError

        # cmd = opcreds.Commands.AddTrustedRootCertificate(rcac_original)
        # await self.send_single_cmd(cmd=cmd, dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id)
        # temp_certs = th1_new_fabric_ctrl.IssueNOCChain(csr_update, th1_dut_node_id)

        cmd = opcreds.Commands.UpdateNOC(
            NOCValue=noc_original, ICACValue=icac_original)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)


if __name__ == "__main__":
    default_matter_test_main()
