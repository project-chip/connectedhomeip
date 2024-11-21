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
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import random

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from test_plan_support import commission_if_required, read_attribute, send_command


def verify_noc() -> str:
    return ("- Verify that the returned list has a single entry.\n"
            "- Save the NOC field as noc_original and the ICAC field as icac_original.\n")


def verify_trusted_root_original() -> str:
    return ("Verify that the returned list has a single entry. Save the entry as trusted_root_original")


def verify_failsafe_status() -> str:
    return ("Verify that the DUT responds with FAILSAFE_REQUIRED")


def verify_armfailsafe_response() -> str:
    return ("Verify that DUT sends ArmFailSafeResponse with the ErrorCode set to OK")


def verify_noc_response(status) -> str:
    return (f"Verify that the DUT responds with a NOCResponse with the StatusCode field set to {status}")


def verify_csr_not_update() -> str:
    return ("Verify that the DUT returns a CSRResponse and save as csr_not_update")


def verify_constraint_error() -> str:
    return ("Verify that the DUT responds with CONSTRAINT_ERROR")


def verify_csr_pase() -> str:
    return ("Verify that the DUT returns a CSRResponse and save as csr_pase")


def verify_unsupported_access() -> str:
    return ("Verify that the DUT responds with UNSUPPORTED_ACCESS")


class TC_OPCREDS_3_4(MatterBaseTest):
    def desc_TC_OPCREDS_3_4(self):
        return " UpdateNOC-Error Condition [DUT-Server]"

    def pics_OPCREDS_3_4(self):
        return ['OPCREDS.S']

    def steps_TC_OPCREDS_3_4(self):
        return [TestStep(1, commission_if_required('TH1'), is_commissioning=True),
                TestStep(
                    2, f"TH1 {read_attribute('NOCs')} from the Node Operational Credentials cluster using a fabric-filtered read. Save the NOCs as nocs.", None, verify_noc()),
                TestStep(
                    3, f"TH1 {read_attribute('TrustedRootCertificates')} attribute from the Node Operational Credentials cluster", None, verify_trusted_root_original()),
                TestStep(
                    4, f"TH1 {send_command('UpdateNOC')} to the Node Operational Credentials cluster with the following fields: NOCValue and ICACValue", None, verify_failsafe_status()),
                TestStep(
                    5, f"TH1 {send_command('ArmFailSafe')} to the DUT with the ExpiryLengthSeconds field set to 900", None, verify_armfailsafe_response()),
                TestStep(
                    6, f"TH1 {send_command('UpdateNOC')} to the Node Operational Credentials cluster with the following fields: NOCValue and ICACValue", None, verify_noc_response("MissingCsr")),
                TestStep(
                    7, f"TH1 {send_command('CSRRequest')} with the IsForUpdateNOC field set to false", None, verify_csr_not_update()),
                TestStep(8, "TH1 generates a new NOC chain with ICAC with the following properties"),
                TestStep(
                    9, f"TH1 {send_command('UpdateNOC')} to the Node Operational Credentials cluster", None, verify_constraint_error()),
                TestStep(
                    10, f"TH1 {send_command('CSRequest')} with the IsForUpdateNOC field set to true", None, verify_csr_not_update()),
                TestStep(
                    11, f"TH1 {send_command('UpdateNOC')} to the Node Operational Credentials cluster", None, verify_noc_response("InvalidPublicKey")),
                TestStep(12, "TH1 generates a new Trusted Root Certificate and Private Key and saves as new_root_cert and new_root_key so that TH can generate an NOC for UpdateNOC that doesn’t chain to the original root"),
                TestStep(13, "TH1 generates a new NOC and ICAC"),
                TestStep(
                    14, f"TH1 {send_command('UpdateNOC')} to the Node Operational Credentials cluster", None, verify_noc_response("InvalidNOC")),
                TestStep(15, "TH1 generates a new NOC and ICAC"),
                TestStep(
                    16, f"TH1 {send_command('UpdateNOC')} to the Node Operational Credentials cluster", None, verify_noc_response("InvalidNOC")),
                TestStep(17, "TH1 generates a new NOC and ICAC"),
                TestStep(
                    18, f"TH1 {send_command('UpdateNOC')} to the Node Operational Credentials cluster", None, verify_noc_response("InvalidNOC")),
                TestStep(
                    19, f"TH1 {send_command('AddTrustedRootCertificate')} to DUT again with the RootCACertificate field set to new_root_cert"),
                TestStep(
                    20, f"TH1 {send_command('UpdateNOC')} to the Node Operational Credentials cluster", None, verify_constraint_error()),
                TestStep(
                    21, f"TH1 {send_command('ArmFailSafe')} to the DUT with the ExpiryLengthSeconds field set to 0", None, verify_armfailsafe_response()),
                TestStep(22, f"TH1 {send_command('OpenCommissioningWindow')} to the DUT"),
                TestStep(
                    23, f"TH1 connects to the DUT over PASE and {send_command('ArmFailSafe')} to the DUT with the ExpiryLengthSeconds field set to 900. Steps 24-26 are all performed over the PASE connection.", None, verify_armfailsafe_response()),
                TestStep(
                    24, f"TH1 {send_command('CSRequest')} over PASE with the IsForUpdateNOC field set to true", None, verify_csr_pase()),
                TestStep(25, "TH1 generates a new NOC chain with ICAC with the following properties: new NOC and ICAC using icac_pase"),
                TestStep(26, f"TH1 {send_command('UpdateNOC')} to the Node Operational Credentials cluster over PASE", None, verify_unsupported_access())]

    @async_test_body
    async def test_TC_OPCREDS_3_4(self):
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
            asserts.fail("Unexpected fail reading ICAC Value on NOCs response")

        self.step(3)
        trusted_root_list_original = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id, cluster=opcreds,
            attribute=opcreds.Attributes.TrustedRootCertificates)
        asserts.assert_equal(len(trusted_root_list_original), 1,
                             "Unexpected number of entries in the TrustedRootCertificates table")

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
        csr_not_update = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        self.step(8)
        new_noc_chain = await self.default_controller.IssueNOCChain(csr_not_update, self.dut_node_id)
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
        th1_fabric_admin_new = th1_ca_new.NewFabricAdmin(vendorId=0xFFF2, fabricId=self.matter_test_config.fabric_id)
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
