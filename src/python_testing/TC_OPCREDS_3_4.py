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
import chip.discovery as Discovery
from chip import ChipDeviceCtrl
from chip.exceptions import ChipStackError
from chip.utils import CommissioningBuildingBlocks
from chip.interaction_model import InteractionModelError, Status
from chip.tlv import TLVReader, TLVWriter
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_OPCREDS_3_4(MatterBaseTest):
    # TODO: Implement the function FindAndEstablishPase
    def FindAndEstablishPase(self, longDiscriminator: int, setupPinCode: int, nodeid: int, dev_ctrl: ChipDeviceCtrl = None):
        if dev_ctrl is None:
            dev_ctrl = self.default_controller

        devices = dev_ctrl.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR, filter=longDiscriminator, stopOnFirst=False)
        # For some reason, the devices returned here aren't filtered, so filter ourselves
        device = next(filter(lambda d: d.commissioningMode ==
                        Discovery.FilterType.LONG_DISCRIMINATOR and d.longDiscriminator == longDiscriminator, devices))
        for a in device.addresses:
            try:
                dev_ctrl.EstablishPASESessionIP(ipaddr=a, setupPinCode=setupPinCode,
                                                nodeid=nodeid, port=device.port)
                break
            except ChipStackError:
                continue
        try:
            dev_ctrl.GetConnectedDeviceSync(nodeid=nodeid, allowPASE=True, timeoutMs=1000)
        except TimeoutError:
            asserts.fail("Unable to establish a PASE session to the device")

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

        self.print_step(2, "TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a fabric-filtered read")
        nocs = await self.read_single_attribute_check_success(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=True)

        self.print_step(3, "TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster")
        trusted_root_original = await self.read_single_attribute_check_success(
            dev_ctrl=th1_new_fabric_ctrl,
            node_id=th1_dut_node_id, cluster=opcreds,
            attribute=opcreds.Attributes.TrustedRootCertificates)
        print("trusted_root_original: ", trusted_root_original)

        self.print_step(4, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster with the following fields: NOCValue and ICACValue")
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_original, ICACValue=icac_original)
        try:
            await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
            asserts.fail("Unexpected error sending UpdateNOC command")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.FailsafeRequired, "Unexpected Failsafe status")

        self.print_step(5, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        print(resp)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "Failure status returned from arm failsafe")

        self.print_step(6, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster with the following fields: NOCValue and ICACValue")
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_original, ICACValue=icac_original)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kMissingCsr, "Failure status returned from UpdateNOC")

        self.print_step(7, "TH1 Sends CSRRequest command with the IsForUpdateNOC field set to false")
        cmd = opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        csr_not_updated = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        print("csr_not_updated: ", csr_not_updated)

        self.print_step(8, "TH1 generates a new NOC chain with ICAC with the following properties")
        new_noc_chain = th1_new_fabric_ctrl.IssueNOCChain(csr_not_updated, th1_dut_node_id)
        noc_not_for_update = csr_not_updated.NOCSRElements
        icac_not_for_update = new_noc_chain.icacBytes

        self.print_step(9, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster")
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_not_for_update, ICACValue=icac_not_for_update)
        try:
            resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
            asserts.fail("Unexpected error sending UpdateNOC command")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Failure status returned from UpdateNOC")

        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(0)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "Failure status returned from arm failsafe")

        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(900)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "Failure status returned from arm failsafe")

        self.print_step(10, "TH1 Sends CSRRequest command with the IsForUpdateNOC field set to true")
        cmd = opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=True)
        csr_update = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)

        self.print_step(11, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster")
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_original, ICACValue=icac_original)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidPublicKey, "Unexpected error code on AddNOC with mismatched CSR")

        self.print_step(12, "TH1 generates a new Trusted Root Certificate and Private Key and saves as new_root_cert and new_root_key so that TH can generate an NOC for UpdateNOC that doesn’t chain to the original root")
        new_root_cert = new_noc_chain.rcacBytes
        new_root_key = TLVReader(new_root_cert).get()["Any"][9]
        print("new_root_key: ", new_root_key)

        self.print_step(13, "TH1 generates a new NOC and ICAC")
        new_noc_chain = th1_new_fabric_ctrl.IssueNOCChain(csr_update, th1_dut_node_id)
        noc_update_new_root = csr_update.NOCSRElements
        icac_update_new_root = new_noc_chain.icacBytes

        self.print_step(14, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster")
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_update_new_root, ICACValue=icac_update_new_root)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidNOC, "NOCResponse with the StatusCode InvalidNOC")

        self.print_step(15, "TH1 generates a new NOC and ICAC")
        # new NOC is generated from the NOCSR returned in csr_update with the matter-fabric-id set to a different
        # value than noc_original. The NOC is signed by new ICA. Save as noc_update_bad_fabric_on_noc.
        noc_update_bad_fabric_on_noc = csr_update.NOCSRElements
        # new ICAC is generated with the and matter-fabric-id omitted. ICAC is signed by the original key for
        # trusted_root_original. Save as icac_update_bad_fabric_on_noc
        icac_update_bad_fabric_on_noc = new_noc_chain.icacBytes

        self.print_step(16, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster")
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_update_bad_fabric_on_noc, ICACValue=icac_update_bad_fabric_on_noc)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidNOC, "NOCResponse with the StatusCode InvalidNOC")

        self.print_step(17, "TH1 generates a new NOC and ICAC")
        noc_update_bad_fabric_on_icac = csr_update.NOCSRElements
        icac_update_bad_fabric_on_icac = new_noc_chain.icacBytes

        self.print_step(18, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster")
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_update_bad_fabric_on_icac, ICACValue=icac_update_bad_fabric_on_icac)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidNOC, "NOCResponse with the StatusCode InvalidNOC")

        self.print_step(19, "TH1 sends AddTrustedRootCertificate command to DUT again with the RootCACertificate field set to new_root_cert")
        cmd = opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)

        self.print_step(20, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster")
        # NOCValue is set to noc_update_new_root
        # ICACValue is to set icac_update_new_root
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_update_new_root, ICACValue=icac_update_new_root)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        # 11.17.7.9 Test Step: Verify that the DUT responds with CONSTRAINT_ERROR
        # TODO: Ask why we expected that error if there is not CSRRequest command previously? Also in the step 6 received and MissingCsr error.
        #asserts.assert_equal(e.status, Status.ConstraintError, "Failure status returned from UpdateNOC")
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kMissingCsr, "Failure status returned from UpdateNOC")

        self.print_step(21, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 0")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(0)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "Failure status returned from arm failsafe")

        self.print_step(22, "TH1 sends an OpenCommissioningWindow command to the DUT")
        resp = self.openCommissioningWindow(th1_new_fabric_ctrl, th1_dut_node_id)

        self.print_step(23, "TH1 connects to the DUT over PASE and sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900. Steps 24-26 are all performed over the PASE connection.")
        self.FindAndEstablishPase(dev_ctrl=th1_new_fabric_ctrl, longDiscriminator=resp.randomDiscriminator,
                                  setupPinCode=resp.commissioningParameters.setupPinCode, nodeid=th1_dut_node_id)

        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(900)
        resp = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk, "Error code status returned from arm failsafe")

        self.print_step(24, "TH1 Sends CSRRequest command over PASE with the IsForUpdateNOC field set to true")
        cmd = opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32))
        csr_pase = await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)

        self.print_step(25, "TH1 Sends CSRRequest command over PASE with the IsForUpdateNOC field set to true")
        new_noc_chain = th1_new_fabric_ctrl.IssueNOCChain(csr_pase, th1_dut_node_id)
        noc_pase = csr_pase.NOCSRElements
        icac_pase = new_noc_chain.icacBytes

        self.print_step(26, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster over PASE")
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_pase, ICACValue=icac_pase)
        try:
            await self.send_single_cmd(dev_ctrl=th1_new_fabric_ctrl, node_id=th1_dut_node_id, cmd=cmd)
            asserts.fail("Unexpected error sending UpdateNOC command")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess, "Failure status returned from UpdateNOC")

if __name__ == "__main__":
    default_matter_test_main()
