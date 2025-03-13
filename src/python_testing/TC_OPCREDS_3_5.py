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
from datetime import timedelta

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.utils import CommissioningBuildingBlocks
from mobly import asserts


class TC_OPCREDS_3_5(MatterBaseTest):

    def pics_TC_OPCREDS_3_5(self):
        return ["OPCREDS.S"]

    def steps_TC_OPCREDS_3_5(self):
        return [TestStep(1, "TH0 adds TH1 over CASE", "Commissioning is successful"),
                TestStep(2, "TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                         "Verify that the returned list has a single entry. Save the NOC field as noc_original and the ICAC field as icac_original"),
                TestStep(3, "TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster",
                         "Verify that at least one of the trusted_root_certificates in the returned list matches the RCAC"),
                TestStep(4, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900",
                         "Verify that DUT sends ArmFailSafeResponse with the ErrorCode set to OK"),
                TestStep(5, "TH1 Sends CSRRequest command with the IsForUpdateNOC field set to true",
                         "Verify that the DUT returns a CSRResponse and save as csr_update1"),
                TestStep(6, """TH1 generates a new NOC and ICAC with the following properties:

                            - new NOC is generated from the NOCSR returned in csr_update1. The subject is set to match noc_original. The NOC is signed by the new ICA. Save as noc_update1

                            - new ICAC must be distinguishable from icac_original. This can be accomplished by changing any of the following: the subject, subject public key or validity. The certificate must be valid. ICAC is signed by the original key for trusted_root_original. Save as icac_update1"""),
                TestStep(7, """TH1 sends the UpdateNOC command to the Node Operational Credentials cluster with the following fields:

                            - NOCValue is set to noc_update1

                            - ICACValue is to set icac_update1""", "Verify that the DUT responds with a NOCResponse having its StatusCode field set to Ok"),
                TestStep(8, "TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                         "Verify that the returned list has a single entry. Verify that the NOC field matches noc_update1 and the ICAC field matches icac_update1"),
                TestStep(9, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 0",
                         "Verify that DUT sends ArmFailSafeResponse with the ErrorCode set to OK"),
                TestStep(10, "TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                         "Verify that the returned list has a single entry. Verify that the NOC field matches noc_original and the ICAC field matches icac_original, due to the explicit expiry of the fail-safe done in the previous step, which is expected to have reverted the updated credentials back to their original value."),
                TestStep(11, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 900",
                         "Verify that DUT sends ArmFailSafeResponse with the ErrorCode set to OK"),
                TestStep(12, "TH1 Sends CSRRequest command with the IsForUpdateNOC field set to true",
                         "Verify that the DUT returns a CSRResponse and save as csr_update2"),
                TestStep(13, """TH1 generates a new NOC and ICAC with the following properties:

                                - new NOC is generated from the NOCSR returned in csr_update2. The subject is set to match noc_original. The NOC is signed by new ICA. Save as noc_update2

                                - new ICAC must be distinguishable from icac_original and icac_update1. This can be accomplished by changing any of the following: the subject, subject public key or validity. The certificate must be valid. ICAC is signed by the original key for trusted_root_original. Save as icac_update2"""),
                TestStep(14, """TH1 sends the UpdateNOC command to the Node Operational Credentials cluster with the following fields:

                                - NOCValue is set to noc_update2

                                - ICACValue is to set icac_update2""", "Verify that the DUT responds with a NOCResponse having its StatusCode field set to Ok"),
                TestStep(15, "TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                         "Verify that the returned list has a single entry. Verify that the NOC field matches noc_update2 and the ICAC field matches icac_update2"),
                TestStep(16, "TH1 sends the CommissioningComplete command to the General Commissioning cluster",
                         "Verify that the DUT returns a CommissioningCompleteResponse with the ErrorCode set to OK"),
                TestStep(17, "TH1 sends ArmFailSafe command to the DUT with the ExpiryLengthSeconds field set to 0 to verify that the CommissioningComplete command successfully persisted the failsafe context.",
                         "Verify that DUT sends ArmFailSafeResponse with the ErrorCode set to OK"),
                TestStep(18, "TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                         "Verify that the returned list has a single entry. Verify that the NOC field matches noc_update2 and the ICAC field matches icac_update2"),
                TestStep(19, "TH1 reads its fabric index from the CurrentFabricIndex attribute and saves as fabric_idx"),
                TestStep(20, "TH0 sends the RemoveFabric command with the fabric index set to fabric_idx")
                ]

    @async_test_body
    async def test_TC_OPCREDS_3_5(self):
        opcreds = Clusters.OperationalCredentials

        self.step(1)

        # Start with 10 years for cert validity
        original_cert_validity = int(timedelta(days=10*365).total_seconds())
        # We're going to force the use of maximally generated certs because this forces the ICAC to be re-generated each time
        th1_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority(
            certificateValidityPeriodSec=original_cert_validity, maximizeCertChains=True)
        th1_fabric_admin = th1_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)
        th1_controller_node_id = self.matter_test_config.controller_node_id+1
        th1 = th1_fabric_admin.NewController(nodeId=th1_controller_node_id)

        success, nocResp, original_cert_chain = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=self.default_controller, newFabricDevCtrl=th1,
            existingNodeId=self.dut_node_id, newNodeId=self.dut_node_id)

        self.step(2)
        nocs = await self.read_single_attribute_check_success(dev_ctrl=th1, node_id=self.dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=True)
        noc_original = nocs[0].noc
        icac_original = nocs[0].icac
        asserts.assert_equal(noc_original, original_cert_chain.nocBytes, "Returned NOC does not match generated NOC")
        asserts.assert_equal(icac_original, original_cert_chain.icacBytes, "Returned ICAC does not match generated ICAC")

        self.step(3)
        rcac_original = original_cert_chain.rcacBytes
        trusted_root_certificates = await self.read_single_attribute_check_success(
            dev_ctrl=th1,
            node_id=self.dut_node_id,
            cluster=opcreds,
            attribute=opcreds.Attributes.TrustedRootCertificates)
        asserts.assert_in(rcac_original, trusted_root_certificates,
                          "RCAC should match one of the TrustedRootCertificates.")

        self.step(4)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        resp = await self.send_single_cmd(dev_ctrl=th1, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(5)
        cmd = opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=True)
        csr_update1 = await self.send_single_cmd(dev_ctrl=th1, node_id=self.dut_node_id, cmd=cmd)

        self.step(6)
        new_cert_validity = int(timedelta(days=9*365).total_seconds())
        th1_certificate_authority.certificateValidityPeriodSec = new_cert_validity
        update1_cert_chain = await th1.IssueNOCChain(csr_update1, self.dut_node_id)
        asserts.assert_not_equal(original_cert_chain.nocBytes, update1_cert_chain.nocBytes,
                                 "Generated matching NOC despite validity changes")
        asserts.assert_not_equal(original_cert_chain.icacBytes, update1_cert_chain.icacBytes,
                                 "Generated matching ICAC despite validity changes")

        self.step(7)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=update1_cert_chain.nocBytes, ICACValue=update1_cert_chain.icacBytes)
        resp = await self.send_single_cmd(dev_ctrl=th1, cmd=cmd, node_id=self.dut_node_id)
        asserts.assert_equal(resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk,
                             "Received unexpected error response from UpdateNOC")
        # Expire the session since hte DUT now has a new cert
        th1.ExpireSessions(self.dut_node_id)

        self.step(8)
        resp = await self.read_single_attribute_check_success(dev_ctrl=th1, node_id=self.dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=True)
        asserts.assert_equal(resp[0].noc, update1_cert_chain.nocBytes, "Returned NOC does not match generated NOC")
        asserts.assert_equal(resp[0].icac, update1_cert_chain.icacBytes, "Returned ICAC does not match generated ICAC")

        self.step(9)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(dev_ctrl=th1, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        # Expire the session since hte DUT now has a new cert
        th1.ExpireSessions(self.dut_node_id)

        self.step(10)
        resp = await self.read_single_attribute_check_success(dev_ctrl=th1, node_id=self.dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=True)
        asserts.assert_equal(resp[0].noc, original_cert_chain.nocBytes, "Returned NOC does not match original NOC")
        asserts.assert_equal(resp[0].icac, original_cert_chain.icacBytes, "Returned ICAC does not match original ICAC")

        self.step(11)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        resp = await self.send_single_cmd(dev_ctrl=th1, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(12)
        cmd = opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=True)
        csr_update2 = await self.send_single_cmd(dev_ctrl=th1, node_id=self.dut_node_id, cmd=cmd)

        self.step(13)
        new_cert_validity = int(timedelta(days=8*365).total_seconds())
        th1_certificate_authority.certificateValidityPeriodSec = new_cert_validity
        update2_cert_chain = await th1.IssueNOCChain(csr_update2, self.dut_node_id)
        asserts.assert_not_equal(update2_cert_chain.nocBytes, update1_cert_chain.nocBytes,
                                 "Generated matching NOC despite validity changes")
        asserts.assert_not_equal(update2_cert_chain.icacBytes, update1_cert_chain.icacBytes,
                                 "Generated matching ICAC despite validity changes")
        asserts.assert_not_equal(update2_cert_chain.nocBytes, original_cert_chain.nocBytes,
                                 "Generated matching NOC despite validity changes")
        asserts.assert_not_equal(update2_cert_chain.icacBytes, original_cert_chain.icacBytes,
                                 "Generated matching ICAC despite validity changes")

        self.step(14)
        cmd = opcreds.Commands.UpdateNOC(NOCValue=update2_cert_chain.nocBytes, ICACValue=update2_cert_chain.icacBytes)
        resp = await self.send_single_cmd(dev_ctrl=th1, cmd=cmd, node_id=self.dut_node_id)
        asserts.assert_equal(resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk,
                             "Received unexpected error response from UpdateNOC")
        # Expire the session since hte DUT now has a new cert
        th1.ExpireSessions(self.dut_node_id)

        self.step(15)
        resp = await self.read_single_attribute_check_success(dev_ctrl=th1, node_id=self.dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=True)
        asserts.assert_equal(resp[0].noc, update2_cert_chain.nocBytes, "Returned NOC does not match generated NOC")
        asserts.assert_equal(resp[0].icac, update2_cert_chain.icacBytes, "Returned ICAC does not match generated ICAC")

        self.step(16)
        cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
        resp = await self.send_single_cmd(dev_ctrl=th1, cmd=cmd, node_id=self.dut_node_id)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Unexpected error returned from CommissioningComplete")

        self.step(17)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(dev_ctrl=th1, node_id=self.dut_node_id, cmd=cmd)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(18)
        resp = await self.read_single_attribute_check_success(dev_ctrl=th1, node_id=self.dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.NOCs, fabric_filtered=True)
        asserts.assert_equal(resp[0].noc, update2_cert_chain.nocBytes, "Returned NOC does not match generated NOC")
        asserts.assert_equal(resp[0].icac, update2_cert_chain.icacBytes, "Returned ICAC does not match generated ICAC")

        self.step(19)
        fabric_idx = resp = await self.read_single_attribute_check_success(dev_ctrl=th1, node_id=self.dut_node_id, cluster=opcreds, attribute=opcreds.Attributes.CurrentFabricIndex)

        self.step(20)
        cmd = Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=fabric_idx)
        await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)


if __name__ == "__main__":
    default_matter_test_main()
