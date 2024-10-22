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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${TYPE_OF_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import random

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_OPCREDS_3_4(MatterBaseTest):
    def desc_TC_OPCREDS_3_4(self):
        return " UpdateNOC-Error Condition [DUT-Server]"

    def steps_TC_OPCREDS_3_4(self):
        return [TestStep(0, "Preconditions"),
                TestStep(1, "TH1 fully commissions the DUT"),
                TestStep(2, "TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a fabric-filtered read"),
                TestStep(3, "TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster"),
                TestStep(
                    4, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster with the following fields: NOCValue and ICACValue"),
                TestStep(5, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900"),
                TestStep(
                    6, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster with the following fields: NOCValue and ICACValue"),
                TestStep(7, "TH1 Sends CSRRequest command with the IsForUpdateNOC field set to false"),
                TestStep(8, "TH1 generates a new NOC chain with ICAC with the following properties"),
                TestStep(9, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster"),
                TestStep(10, "TH1 Sends CSRRequest command with the IsForUpdateNOC field set to true"),
                TestStep(11, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster"),
                TestStep(12, "TH1 generates a new Trusted Root Certificate and Private Key and saves as new_root_cert and new_root_key so that TH can generate an NOC for UpdateNOC that doesn’t chain to the original root"),
                TestStep(13, "TH1 generates a new NOC and ICAC"),
                TestStep(14, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster"),
                TestStep(15, "TH1 generates a new NOC and ICAC"),
                TestStep(16, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster"),
                TestStep(17, "TH1 generates a new NOC and ICAC"),
                TestStep(18, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster"),
                TestStep(19, "TH1 sends AddTrustedRootCertificate command to DUT again with the RootCACertificate field set to new_root_cert"),
                TestStep(20, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster"),
                TestStep(21, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 0"),
                TestStep(22, "TH1 sends an OpenCommissioningWindow command to the DUT"),
                TestStep(23, "TH1 connects to the DUT over PASE and sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900. Steps 24-26 are all performed over the PASE connection."),
                TestStep(24, "TH1 Sends CSRRequest command over PASE with the IsForUpdateNOC field set to true"),
                TestStep(25, "TH1 generates a new NOC chain with ICAC with the following properties: new NOC and ICAC using icac_pase"),
                TestStep(26, "TH1 sends the UpdateNOC command to the Node Operational Credentials cluster over PASE")]

    @async_test_body
    async def test_TC_OPCREDS_3_4(self):
        self.step(0)

        self.step(1)
        opcreds = Clusters.OperationalCredentials

        self.step(2)
        nocs = await self.read_single_attribute_check_success(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=True)
        if nocs[0].noc:
            noc_original = nocs[0].noc
        else:
            asserts.assert_true(False, "Unexpected fail reading NOC Value on NOCs response")

        if nocs[0].icac:
            icac_original = nocs[0].icac
        else:
            asserts.assert_true(False, "Unexpected fail reading ICAC Value on NOCs response")

        self.step(3)
        trusted_root_list_original = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id, cluster=opcreds,
            attribute=opcreds.Attributes.TrustedRootCertificates)
        asserts.assert_equal(len(trusted_root_list_original), 1, "Unexpected number of entries in the TrustedRootCertificates table")

        self.step(4)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_original, ICACValue=icac_original)
        try:
            await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
            asserts.fail("Unexpected error sending UpdateNOC command")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.FailsafeRequired, "Unexpected Failsafe status")

        self.step(5)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(6)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_original, ICACValue=icac_original)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kMissingCsr,
                             "Failure status returned from UpdateNOC")

        self.step(7)
        cmd = opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        csr_not_updated = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        self.step(8)
        new_noc_chain = await self.default_controller.IssueNOCChain(csr_not_updated, self.dut_node_id)
        noc_not_for_update = new_noc_chain.nocBytes
        icac_not_for_update = new_noc_chain.icacBytes

        self.step(9)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_not_for_update, ICACValue=icac_not_for_update)
        try:
            resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
            asserts.fail("Unexpected error sending UpdateNOC command")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Failure status returned from UpdateNOC")

        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(0)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(900)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(10)
        cmd = opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=True)
        csr_update = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        self.step(11)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_original, ICACValue=icac_original)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidPublicKey,
                             "Unexpected error code on AddNOC with mismatched CSR")

        self.step(12)
        th1_ca_new = self.certificate_authority_manager.NewCertificateAuthority()
        th1_fabric_admin_new = th1_ca_new.NewFabricAdmin(vendorId=0xFFF1, fabricId=1)
        th1_new = th1_fabric_admin_new.NewController(nodeId=self.default_controller.nodeId+1)

        self.step(13)
        th1_certs_new = await th1_new.IssueNOCChain(csr_update, self.dut_node_id+1)
        new_root_cert = th1_certs_new.rcacBytes
        noc_update_new_root = th1_certs_new.nocBytes
        icac_update_new_root = th1_certs_new.icacBytes

        self.step(14)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_update_new_root, ICACValue=icac_update_new_root)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidNOC,
                             "NOCResponse with the StatusCode InvalidNOC")

        self.step(15)
        cmd = opcreds
        # new NOC is generated from the NOCSR returned in csr_update with the matter-fabric-id set to a different
        # value than noc_original. The NOC is signed by new ICA. Save as noc_update_bad_fabric_on_noc.
        noc_update_bad_fabric_on_noc = th1_certs_new.nocBytes
        # new ICAC is generated with the and matter-fabric-id omitted. ICAC is signed by the original key for
        # trusted_root_original. Save as icac_update_bad_fabric_on_noc
        icac_update_bad_fabric_on_noc = new_noc_chain.icacBytes

        self.step(16)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_update_bad_fabric_on_noc, ICACValue=icac_update_bad_fabric_on_noc)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidNOC,
                             "NOCResponse with the StatusCode InvalidNOC")

        self.step(17)
        noc_update_bad_fabric_on_icac = th1_certs_new.nocBytes
        icac_update_bad_fabric_on_icac = new_noc_chain.icacBytes

        self.step(18)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_update_bad_fabric_on_icac, ICACValue=icac_update_bad_fabric_on_icac)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidNOC,
                             "NOCResponse with the StatusCode InvalidNOC")

        self.step(19)
        cmd = opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        self.step(20)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_update_new_root, ICACValue=icac_update_new_root)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kMissingCsr,
                             "Failure status returned from UpdateNOC")

        self.step(21)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(0)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(22)
        resp = await self.openCommissioningWindow(self.default_controller, self.dut_node_id)

        self.step(23)
        # self.FindAndEstablishPase(dev_ctrl=th1_new_fabric_ctrl, longDiscriminator=resp.randomDiscriminator,
        #                          setupPinCode=resp.commissioningParameters.setupPinCode, nodeid=th1_dut_node_id)
        # setupCode = self.matter_test_config.qr_code_content + self.matter_test_config.manual_code
        await self.default_controller.FindOrEstablishPASESession(setupCode=resp.commissioningParameters.setupQRCode, nodeid=self.dut_node_id)

        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(900)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Error code status returned from arm failsafe")

        self.step(24)
        cmd = opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32))
        csr_pase = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        self.step(25)
        new_noc_chain = await self.default_controller.IssueNOCChain(csr_pase, self.dut_node_id)
        # cmd = opcreds.Commands.AddTrustedRootCertificate(trusted_root_original)
        # try:
        #     await self.send_single_cmd(dev_ctrl=th1_new, node_id=self.default_controller.nodeId+1, cmd=cmd)
        #     asserts.fail("Success when adding trusted root certificate")
        # except InteractionModelError as e:
        #     asserts.assert_equal(e.status, Status.ConstraintError, "Unexpected when adding trusted root certificate")
        noc_pase = csr_pase.NOCSRElements
        icac_pase = new_noc_chain.icacBytes

        self.step(26)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=noc_pase, ICACValue=icac_pase)
        try:
            await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
            asserts.fail("Unexpected error sending UpdateNOC command")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess, "Failure status returned from UpdateNOC")


if __name__ == "__main__":
    default_matter_test_main()
