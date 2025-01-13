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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.utils import CommissioningBuildingBlocks
from mobly import asserts
import logging
import random
import binascii
import gc

# Create a logger
logger = logging.getLogger(__name__)


class TC_CGEN_2_2(MatterBaseTest):
    def desc_TC_CGEN_2_2(self) -> str:
        return "[TC-CGEN-2.2] ArmFailSafe command verification [DUT - Server]"

    def pics_TC_CGEN_2_2(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "CGEN.S",      # Pics
        ]
        return pics

    def steps_TC_CGEN_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster 
                     and saves the number of list items as numTrustedRootsOriginal"""),
            TestStep(2, """TH1 reads the BasicCommissioningInfo attribute 
                     and saves the MaxCumulativeFailsafeSeconds as maxFailsafe"""),
            TestStep(3, """TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds 
                     and the Breadcrumb value as 1"""),
            TestStep(4, "TH1 reads the Breadcrumb attributee"),
            TestStep(5, """TH1 sends AddTrustedRootCertificate command to DUT with RootCACertificate set to Root_CA_Certificate_TH1
                     verify SUCCESS"""),
            # TestStep(6, "TH..."),
            # TestStep(7, "TH..."),
            # TestStep(8, "TH..."),
            # TestStep(9, "TH..."),
            # TestStep(10, "TH..."),
            # TestStep(11, "TH..."),
        ]
        return steps

    @async_test_body
    async def test_TC_CGEN_2_2(self):
        cluster_opcreds = Clusters.OperationalCredentials
        cluster_cgen = Clusters.GeneralCommissioning

        self.step(0)

    # Read the Spteps
        self.step(1)
        trusted_root_list_original = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size = len(trusted_root_list_original)
        logger.info(f'The trusted_root_list is {trusted_root_list_original}')
        logger.info(f'The size of the trusted_root_list is {trusted_root_list_original_size}')
        # asserts.assert_equal(len(trusted_root_list_original), 1,
        #                      "Unexpected number of entries in the TrustedRootCertificates table")

        self.step(2)
        basic_commissioning_info = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.BasicCommissioningInfo)
        maxFailsafe = basic_commissioning_info.maxCumulativeFailsafeSeconds
        logger.info(f'The MaxCumulativeFailsafeSeconds is {maxFailsafe}')

        self.step(3)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        logger.info(f'response attributesare: {vars(resp)}')

        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        assert debug_text == '' or len(debug_text) <= 512, "debugText must be empty or have a maximum length of 512 characters"

        self.step(4)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.Breadcrumb)
        logger.info(f'The breadcrumb_attribute is {breadcrumb_info}')

        self.step(5)

        root_cert_hex = "1530010828c376ebc17f21512402013703271401000000cacacaca182604ef171b2726056eb5b94c3706271401000000cacacaca182407012408013009410452c19fd9d329a738fd65722a8309fa68bcaa9ffe87d8114b802c922e5066d0b2f0573b89b38bf98fc9c424ab8ffdabcb18d42e623d82a02d0ca0c062ccadb4bc370a350129011824026030041457934de5405e9a40eacb86ee647e583141ae78f430051457934de5405e9a40eacb86ee647e583141ae78f418300b40a0b0d57bddbc7bcf44480a8b7bd0231d54ccacd68d90efb67b7aa3206adbd268725092992a0388c8e934504178613c5b932d422eed7463f38fd82aaa429b574a18"
        bin_root_cert = binascii.unhexlify(root_cert_hex)

        # 1 Request CSR (Certificate Signing Request) and update NOC (Node Operational Certificate)
        cmd = cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=True)
        csr_update = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        # 2 Create new certificate authority for TH1
        th1_ca_new = self.certificate_authority_manager.NewCertificateAuthority(maximizeCertChains=True)
        th1_fabric_admin_new = th1_ca_new.NewFabricAdmin(vendorId=0xFFF2, fabricId=2)

        # 3 Create the new controller
        th1_new = th1_fabric_admin_new.NewController(nodeId=self.default_controller.nodeId+1)

        # 4 Isue the certificates
        th1_certs_new = await th1_new.IssueNOCChain(csr_update, self.dut_node_id+1)
        new_root_cert = th1_certs_new.rcacBytes
        logger.info(f"RCAC Certificate: {new_root_cert}")
        noc_update_new_root = th1_certs_new.nocBytes
        logger.info(f"NOC Certificate: {noc_update_new_root}")
        icac_update_new_root = th1_certs_new.icacBytes
        logger.info(f"ICAC Certificate: {icac_update_new_root}")

        # 5 Send command to add new trusted root certificate (in binary format)
        cmd = cluster_opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        logger.info(f'AddTrustedRootCertificate command response: {resp}')

        # Verify that the response does not have an error
        asserts.assert_equal(resp.errorCode, Clusters.OperationalCredentials.Enums.OperationalCertStatusEnum.kSuccess,
                             "Failed to add trusted root certificate")
        logger.info("Root certificate added successfull")


        # self.step(7)
        # self.step(8)
        # self.step(9)
        # self.step(10)
        # self.step(11)
if __name__ == "__main__":
    default_matter_test_main()
