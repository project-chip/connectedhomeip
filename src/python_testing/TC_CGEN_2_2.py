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
from chip import ChipDeviceCtrl
from chip.exceptions import ChipStackError
import chip.discovery as Discovery
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.utils import CommissioningBuildingBlocks
from mobly import asserts
import logging
import random
import asyncio
import time
from chip.exceptions import ChipStackError
from chip.native import PyChipError

# Create logger
logger = logging.getLogger(__name__)


class TC_CGEN_2_2(MatterBaseTest):
    async def FindAndEstablishPase(self, longDiscriminator: int, setupPinCode: int, nodeid: int, dev_ctrl: ChipDeviceCtrl = None):
        if dev_ctrl is None:
            dev_ctrl = self.default_controller

        devices = await dev_ctrl.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR, filter=longDiscriminator, stopOnFirst=False)
        # For some reason, the devices returned here aren't filtered, so filter ourselves
        device = next(filter(lambda d: d.commissioningMode ==
                      Discovery.FilterType.LONG_DISCRIMINATOR and d.longDiscriminator == longDiscriminator, devices))
        for a in device.addresses:
            try:
                await dev_ctrl.EstablishPASESessionIP(ipaddr=a, setupPinCode=setupPinCode,
                                                      nodeid=nodeid, port=device.port)
                break
            except ChipStackError:
                continue
        try:
            dev_ctrl.GetConnectedDeviceSync(nodeid=nodeid, allowPASE=True, timeoutMs=1000)
        except TimeoutError:
            asserts.fail("Unable to establish a PASE session to the device")

    async def OpenCommissioningWindow(self, dev_ctrl: ChipDeviceCtrl, node_id: int):
        # TODO: abstract this in the base layer? Do we do this a lot?
        longDiscriminator = random.randint(0, 4095)
        try:
            params = await dev_ctrl.OpenCommissioningWindow(
                nodeid=node_id, timeout=600, iteration=10000, discriminator=longDiscriminator, option=ChipDeviceCtrl.ChipDeviceControllerBase.CommissioningWindowPasscode.kTokenWithRandomPin)
        except Exception as e:
            logging.exception('Error running OpenCommissioningWindow %s', e)
            asserts.assert_true(False, 'Failed to open commissioning window')
        return (longDiscriminator, params)

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
                     and the Breadcrumb value as 1. 
                     DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)"""),
            TestStep(4, """TH1 reads the Breadcrumb attribute.
                     Verify the breadcrumb attribute is 1"""),
            TestStep(5, """TH1 generates a new TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1. 
                     TH1 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate.
                     DUT responds with SUCCESS"""),
            TestStep(6, """"TH1 reads the TrustedRootCertificate attribute 
                     and verify that the number of items in the returned list is numTrustedRootsOriginal + 1"""),
            TestStep(7, "TH1 waits for PIXIT.CGEN.FailsafeExpiryLengthSeconds to ensure the failsafe timer has expired"),
            TestStep(8, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster 
                     and verify that the number of items in the returned list is numTrustedRootsOriginal"""),
            TestStep(9, """"TH1 reads the Breadcrumb attribute and verify that the breadcrumb attribute is 0"""),
            TestStep(10, """TH1 repeats steps #3 through #5."""),
            TestStep(11, """TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0"""),
            TestStep(12, """TH1 Repeat steps 8 through 9"""),
            TestStep(13, """TH1 sends the OpenCommissioningWindow command to the Administrator Commissioning cluster"""),
            TestStep(14, """TH1 TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds"""),
            TestStep(15, """TH2 opens a PASE connection to the DUT"""),
            TestStep(16, """TH2 obtains or generates a TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1. 
                     TH2 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate
                     DUT responds with SUCCESS"""),
            TestStep(17, """TH2 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0"""),
            TestStep(18, """TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a non-fabric-filtered read 
                     and saves the returned list as nocs"""),
            TestStep(19, """TH1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read 
                     and saves the returned list as fabrics"""),
            TestStep(20, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster 
                     and saves the returned list as trustedroots"""),
            TestStep(21, """TH2 starts commissioning the DUT. It performs all steps up to establishing a CASE connection, 
                     but DOES NOT send the CommissioningComplete command"""),
            TestStep(22, """TH2 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0"""),
        ]
        return steps

    @async_test_body
    async def test_TC_CGEN_2_2(self):
        cluster_opcreds = Clusters.OperationalCredentials
        cluster_cgen = Clusters.GeneralCommissioning

        maxFailsafe_tmp = 10

        self.step(0)

    # Read the Spteps
        self.step(1)
        trusted_root_list_original = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size = len(trusted_root_list_original)
        logger.info(f'The original trusted_root_list is {trusted_root_list_original}')
        logger.info(f'The original trusted_root_list size is {trusted_root_list_original_size}')

        # Verify that original list has 1 certificate
        asserts.assert_equal(len(trusted_root_list_original), 1,
                             "Unexpected number of entries in the TrustedRootCertificates table")

        self.step(2)
        basic_commissioning_info = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.BasicCommissioningInfo)
        maxFailsafe = basic_commissioning_info.maxCumulativeFailsafeSeconds
        logger.info(f'The MaxCumulativeFailsafeSeconds is {maxFailsafe}')

        self.step(3)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        logger.info(f'response attributes are: {vars(resp)}')

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
        logger.info("Generating a new CSR to update the root certificate...")

        # 5.1 Request CSR (Certificate Signing Request) and update NOC (Node Operational Certificate)
        cmd = cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        csr_update = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        # Create new certificate authority for TH1
        # th1_ca_new = self.certificate_authority_manager.NewCertificateAuthority(maximizeCertChains=True)
        # th1_fabric_admin_new = th1_ca_new.NewFabricAdmin(vendorId=0xFFF2, fabricId=2)

        # Create the new controller
        # th1_new = th1_fabric_admin_new.NewController(nodeId=self.default_controller.nodeId + 1)

        # 5.2 Isue the certificates
        th1_certs_new = await self.default_controller.IssueNOCChain(csr_update, self.dut_node_id)
        new_root_cert = th1_certs_new.rcacBytes
        logger.info(f"RCAC Certificate: {new_root_cert}")

        # 5.3 Send command to add new trusted root certificate
        cmd = cluster_opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        logger.info(f'AddTrustedRootCertificate command response: {resp}')

        self.step(6)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(f'The updated trusted_root_list is {trusted_root_list_original_updated}')
        logger.info(f'The updated trusted_root_list size is {trusted_root_list_original_size_updated}')

        # Verify that the trusted root list size has increased by 1
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 1,
                             "Unexpected number of entries in the TrustedRootCertificates table after update")

        # Optionally, check if the new certificate is in the updated list
        assert new_root_cert in trusted_root_list_original_updated, \
            "New root certificate was not added to the trusted root list."

        self.step(7)
        logger.info(f"Waiting for Failsafe timer to expire for {maxFailsafe_tmp} seconds...")
        start_time = time.time()

        # Wait for the maximum failsafe time
        await asyncio.sleep(12)

        elapsed_time = time.time() - start_time
        logger.info(f"Failsafe timer expired after {elapsed_time} seconds.")

        # Verify that the elapsed time is as expected (within a tolerance margin)
        # assert abs(elapsed_time - maxFailsafe) <= 1, f"Failsafe timer did not expire correctly. Expected {maxFailsafe} seconds but got {elapsed_time} seconds."

        self.step(8)
        trusted_root_list_original_after_wait = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)

        # Get the size of the list after waiting
        trusted_root_list_original_size_after_wait = len(trusted_root_list_original_after_wait)
        logger.info(
            f'The trusted_root_list size after waiting for failsafe timeout is {trusted_root_list_original_size_after_wait}')

        # Verifying if the number of entries in the TrustedRootCertificates table is the same as before
        # assert trusted_root_list_original_size_after_wait == trusted_root_list_original_size_updated, \
        #     f"Expected {trusted_root_list_original_size_updated} TrustedRootCertificates, but found {trusted_root_list_original_size_after_wait}"

        self.step(9)
        breadcrumb_info_after_wait = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.Breadcrumb)

        logger.info(f"The breadcrumb_attribute after waiting for failsafe timeout is: {breadcrumb_info_after_wait}")
        # asserts.assert_equal(breadcrumb_info_after_wait, 0, "Breadcrumb value is not 0 after waiting for failsafe timer")

        self.step(10)
        # self.step(3) ************************************************************************************************
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        logger.info(f'response attributes are: {vars(resp)}')

        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        assert debug_text == '' or len(debug_text) <= 512, "debugText must be empty or have a maximum length of 512 characters"

        # self.step(4) ***************************************************************************************

        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.Breadcrumb)
        logger.info(f'The breadcrumb_attribute (Step #10) is {breadcrumb_info}')

        # self.step(5) **********************************************************************
        logger.info("Generating a new CSR to update the root certificate...")

        # 5.1 Request CSR (Certificate Signing Request) and update NOC (Node Operational Certificate)
        cmd = cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        csr_update = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        # Create new certificate authority for TH1
        # th1_ca_new = self.certificate_authority_manager.NewCertificateAuthority(maximizeCertChains=True)
        # th1_fabric_admin_new = th1_ca_new.NewFabricAdmin(vendorId=0xFFF2, fabricId=2)

        # Create the new controller
        # th1_new = th1_fabric_admin_new.NewController(nodeId=self.default_controller.nodeId + 1)

        # 5.2 Isue the certificates
        th1_certs_new = await self.default_controller.IssueNOCChain(csr_update, self.dut_node_id)
        new_root_cert = th1_certs_new.rcacBytes
        logger.info(f"RCAC Certificate: {new_root_cert}")

        # 5.3 Send command to add new trusted root certificate
        cmd = cluster_opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        logger.info(f'AddTrustedRootCertificate command response (Step #10): {resp}')

        # self.step(6) ******************************************este no va
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(f'The updated trusted_root_list is  (Step #10) {trusted_root_list_original_updated}')
        logger.info(f'The updated trusted_root_list size is  (Step #10) {trusted_root_list_original_size_updated}')

        # Verify that the trusted root list size has increased by 1
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 1,
                             "Unexpected number of entries in the TrustedRootCertificates table after update")

        # Optionally, check if the new certificate is in the updated list
        assert new_root_cert in trusted_root_list_original_updated, \
            "New root certificate was not added to the trusted root list."

        self.step(11)
        # self.step(3)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        logger.info(f'response attributes are: {vars(resp)}')

        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        assert debug_text == '' or len(debug_text) <= 512, "debugText must be empty or have a maximum length of 512 characters"

        self.step(12)
        # self.step(8)
        trusted_root_list_original_after_wait = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)

        # Get the size of the list after waiting
        trusted_root_list_original_size_after_wait = len(trusted_root_list_original_after_wait)
        logger.info(
            f'(Step #12) The trusted_root_list size after waiting for failsafe timeout is {trusted_root_list_original_size_after_wait}')

        # Verifying if the number of entries in the TrustedRootCertificates table is the same as before
        # assert trusted_root_list_original_size_after_wait == trusted_root_list_original_size_updated, \
        #     f"Expected {trusted_root_list_original_size_updated} TrustedRootCertificates, but found {trusted_root_list_original_size_after_wait}"

        # self.step(9)
        breadcrumb_info_after_wait = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.Breadcrumb)

        logger.info(f"(Step #12)The breadcrumb_attribute after waiting for failsafe timeout is: {breadcrumb_info_after_wait}")
        # asserts.assert_equal(breadcrumb_info_after_wait, 0, "Breadcrumb value is not 0 after waiting for failsafe timer")

        self.step(13)

        # Create TH2
        # Maximize cert chains so we can use this below
        TH2_CA_real = self.certificate_authority_manager.NewCertificateAuthority(maximizeCertChains=True)
        TH2_vid = 0xFFF2
        TH2_fabric_admin_real = TH2_CA_real.NewFabricAdmin(vendorId=TH2_vid, fabricId=2)
        TH2_nodeid = self.default_controller.nodeId+1
        TH2 = TH2_fabric_admin_real.NewController(nodeId=TH2_nodeid)

        longDiscriminator, params = await self.OpenCommissioningWindow(self.default_controller, self.dut_node_id)
        # params = await self.OpenCommissioningWindow(self.default_controller, self.dut_node_id)
        setup_pin_code = setup_pin_code = params.setupPinCode

        logger.info(f"step #13 - params with vars: {vars(params)}")
        logger.info(f"step #13 - params: {params}")
        logger.info(f"step #13 - setupPinCode: {setup_pin_code}")
        logger.info(f"step #13 - Attributes of params: {dir(params)}")

        self.step(14)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        logger.info(f'step #14 - response attributes are: {vars(resp)}')
        logger.info(f'step #14 - response attributes are: {resp.errorCode}')

        self.step(15)
        newNodeId = self.dut_node_id + 1
        await self.FindAndEstablishPase(dev_ctrl=TH2, longDiscriminator=longDiscriminator,
                                        setupPinCode=setup_pin_code, nodeid=newNodeId)

        self.step(16)
        logger.info("TH2 Generating a new CSR to update the root certificate...")

        # 5.1 Request CSR (Certificate Signing Request) and update NOC (Node Operational Certificate)
        cmd = cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        th2_csr = await self.send_single_cmd(dev_ctrl=TH2, node_id=newNodeId, cmd=cmd)

        # 5.2 Isue the certificates
        th2_certs_new = await TH2.IssueNOCChain(th2_csr, newNodeId)
        th2_new_root_cert = th2_certs_new.rcacBytes
        logger.info(f"Step #16 - TH2 RCAC Certificate: {th2_new_root_cert}")

        # 5.3 Send command to add new trusted root certificate
        cmd = cluster_opcreds.Commands.AddTrustedRootCertificate(th2_new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=TH2, node_id=newNodeId, cmd=cmd)
        logger.info(f'Step #16 - TH2 AddTrustedRootCertificate command response: {resp}')

        self.step(17)
        logger.info(f'Step #17 - TH2_nodeid : {TH2_nodeid}')
        logger.info(f'Step #17 - newNodeId : {newNodeId}')

        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=TH2,
            node_id=newNodeId,
            cmd=cmd)
        logger.info(f'Step #17 - response attributes are: {vars(resp)}')

        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        assert debug_text == '' or len(debug_text) <= 512, "debugText must be empty or have a maximum length of 512 characters"

        # *****************
        self.step(18)
        logger.info(f'Step #18 - TH1 self.default_controller: {vars(self.default_controller)}')
        logger.info(f'Step #18 - TH2 self.default_controller: {vars(TH2)}')

        nocs = await self.read_single_attribute_check_success(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cluster=cluster_opcreds, attribute=cluster_opcreds.Attributes.NOCs, fabric_filtered=False)
        nocs_original_size = len(nocs)
        logger.info(f'Step #18 - TH1 nocs_original_size: {nocs_original_size}')

        self.step(19)
        fabrics = await self.read_single_attribute_check_success(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cluster=cluster_opcreds, attribute=cluster_opcreds.Attributes.Fabrics, fabric_filtered=False)
        fabrics_original_size = len(fabrics)
        logger.info(f'Step #19 - TH1 fabrics_original_size: {fabrics_original_size}')

        self.step(20)
        trustedroots_list = await self.read_single_attribute_check_success(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cluster=cluster_opcreds, attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trustedroots_list_size = len(trustedroots_list)
        logger.info(f'Step #20 - TH1 trusted_root_list: {trustedroots_list_size}')
        logger.info(f'Step #20 - from #12 - TH1 trusted_root_list: {trusted_root_list_original_size_after_wait}')

        self.step(21)
        logger.info(f"Step #21 - TH2 starts commissioning, establishing a CASE connection, but DOES NOT send the CommissioningComplete")
        # Commissioning stage numbers - we should find a better way to match these to the C++ code
        # CommissioningDelegate.h
        # TODO: https://github.com/project-chip/connectedhomeip/issues/36629
        kFindOperationalForCommissioningComplete = 30
        logger.info(
            f'Step #21 - Commissioning stage SetTestCommissionerPrematureCompleteAfter enum: {kFindOperationalForCommissioningComplete}')

        # reset_com = TH2.ResetTestCommissioner()
        test = TH2.SetTestCommissionerPrematureCompleteAfter(kFindOperationalForCommissioningComplete)
        logger.info(f'Step #21 - test: {test}')
        # Wait for clean up
        await asyncio.sleep(1)

        self.step(22)
        logger.info(f"Step #22 - Sending ArmFailSafe command with expiry set to 0 seconds")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=TH2,
            node_id=newNodeId,
            cmd=cmd
        )
        logger.info(f'Step #22 - response attributes are: {vars(resp)}')

        # TH2_dut_nodeid = self.dut_node_id+2
        # TH2.ExpireSessions(newNodeId)
        # # longDiscriminator, params = await self.OpenCommissioningWindow(self.default_controller, self.dut_node_id)
        # # TH2.ResetTestCommissioner()

        # # await TH2.CommissionOnNetwork(
        # #    nodeId=TH2_dut_nodeid, setupPinCode=setup_pin_code,
        # #    filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=longDiscriminator)
        # # FUNCIONA
        # await self.FindAndEstablishPase(dev_ctrl=TH2, longDiscriminator=longDiscriminator,
        #                                 setupPinCode=setup_pin_code, nodeid=newNodeId)


if __name__ == "__main__":
    default_matter_test_main()
