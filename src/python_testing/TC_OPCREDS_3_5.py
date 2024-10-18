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
import copy
import random
#from cryptography import x509
#from cryptography.hazmat.backends import default_backend

import chip.clusters as Clusters
from chip.tlv import TLVReader, TLVWriter
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
        success, nocResp, nocBytes, rcacBytes, icacBytes = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=th1_new_fabric_ctrl,
            existingNodeId=self.dut_node_id, newNodeId=th1_dut_node_id)

        self.print_step(2, "TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a fabric-filtered read")
        nocs = await self.read_single_attribute_check_success(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=True)
        noc_original = nocs[0].noc
        icac_original = nocs[0].icac

        self.print_step(3, "TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster")
        trusted_root_original = await self.read_single_attribute_check_success(
            dev_ctrl=th1_new_fabric_ctrl,
            node_id=th1_dut_node_id,
            cluster=opcreds,
            attribute=opcreds.Attributes.TrustedRootCertificates)

        self.print_step(4, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.print_step(5, "TH1 Sends CSRRequest command with the IsForUpdateNOC field set to true")
        cmd = opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=True)
        csr_update1 = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        print("csr_not_updated: ", csr_update1)

        self.print_step(6, "TH1 generates a new NOC and ICAC")
        new_noc_chain = await th1_new_fabric_ctrl.IssueNOCChain(csr_update1, th1_dut_node_id)
        noc_update1 = csr_update1.NOCSRElements
        print("new_noc_chain: ", new_noc_chain)
        icac_update1 = new_noc_chain.icacBytes

        self.print_step(7, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster")
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_original, ICACValue=icac_original)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        print("step7: ", resp)

        # new ICAC must be distinguishable from icac_original. This can be accomplished by changing any
        # of the following: the subject, subject public key or validity. The certificate must be valid.
        # ICAC is signed by the original key for trusted_root_original. Save as icac_update1

        # print(TLVReader(icac_original).get()["Any"])
        # print(TLVReader(icac_original).get()["Any"][6]) #Subject
        # icac_original_subject_decoded = TLVReader(icac_original).get()["Any"][6]
        # writer = TLVWriter(bytearray())
        # writer.startStructure(None)
        # for tag, value in icac_original_decoded:
        #     icac_fake = writer.put(0, 0)
        #     print("icac_fake after writer: ", icac_fake)
        # writer.endContainer()
        # icac_fake = writer.encoding
        # print("icac_fake: ", icac_fake)
        # print("TLVReader icac fake: ", TLVReader(icac_fake).get()["Any"])

        # icac_fake_subject = copy.deepcopy(icac_original)
        # print(TLVReader(icac_fake_subject).get()["Any"][6])
        # fake_subject_int = int.from_bytes(icac_fake_subject[6], 'big') + 1
        # fake_subject = fake_subject_int.to_bytes(len(icac_fake_subject[6]), big)
        # print(TLVReader(fake_subject).get()["Any"][6])

        # writer = TLVWriter()
        # writer.startStructure(None)
        # writer.put(10, 20)
        # writer.endContainer()
        # print("read writer: ", TLVReader(writer).get()["Any"])

        #cert_real = cert = x509.load_der_x509_certificate(icac_original, default_backend())
        #print(cert_real)


if __name__ == "__main__":
    default_matter_test_main()