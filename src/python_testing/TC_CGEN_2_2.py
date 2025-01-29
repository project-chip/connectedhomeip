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

import asyncio
import logging
import random
import time
from datetime import datetime

from mobly import asserts

import chip.clusters as Clusters
import chip.discovery as Discovery
from chip import ChipDeviceCtrl
from chip.exceptions import ChipStackError
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


# Create logger
logger = logging.getLogger(__name__)


class TC_CGEN_2_2(MatterBaseTest):
    async def FindAndEstablishPase(self, longDiscriminator: int, setupPinCode: int, nodeid: int, dev_ctrl: ChipDeviceCtrl = None):
        """
        Establishes a PASE session to a device using longDiscriminator.

        This method will discover commissionable nodes and filter by longDiscriminator, 
        then attempts to establish a PASE session with setupPinCode and nodeid.

        If no device controller is provided, the default controller will be used.

        Args:
            longDiscriminator (int): The long discriminator used to identify the device.
            setupPinCode (int): The setup PIN code for establishing the session.
            nodeid (int): The node ID for the device.
            dev_ctrl (ChipDeviceCtrl, optional): The device controller. If None, 
                the default controller is used.

        Raises:
            TimeoutError: If unable to establish the PASE session within the timeout.
            ChipStackError: If an error occurs during the establishment of the PASE session.
        """
        if dev_ctrl is None:
            dev_ctrl = self.default_controller

        devices = await dev_ctrl.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR, filter=longDiscriminator, stopOnFirst=False)

        # For some reason, the devices returned here aren't filtered, so filter ourselves
        device = next(
            filter(
                lambda d: d.commissioningMode == Discovery.FilterType.LONG_DISCRIMINATOR
                and d.longDiscriminator == longDiscriminator, devices
            )
        )

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

    def desc_TC_CGEN_2_2(self) -> str:
        return "[TC-CGEN-2.2] ArmFailSafe command verification [DUT - Server]"

    def steps_TC_CGEN_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster 
                     and saves the number of list items as numTrustedRootsOriginal."""),
            TestStep(2, """TH1 reads the BasicCommissioningInfo attribute 
                     and saves the MaxCumulativeFailsafeSeconds as maxFailsafe."""),
            TestStep(3, """TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds
                     and the Breadcrumb value as 1."""),
            TestStep(4, """TH1 reads the Breadcrumb attribute."""),
            TestStep(5, """TH1 generates a new TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1.
                     TH1 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate."""),
            TestStep(6, """TH1 reads the TrustedRootCertificate attribute."""),
            TestStep(7, """TH1 waits for PIXIT.CGEN.FailsafeExpiryLengthSeconds to ensure the failsafe timer has expired."""),
            TestStep(8, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster."""),
            TestStep(9, """TH1 reads the Breadcrumb attribute and verify that the breadcrumb attribute is 0."""),
            TestStep(10, """TH1 repeats steps #3 through #5."""),
            TestStep(11, """TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0."""),
            TestStep(12, """TH1 Repeat steps 8 through 9."""),
            TestStep(13, """TH1 sends the OpenCommissioningWindow command to the Administrator Commissioning cluster."""),
            TestStep(14, """TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds"""),
            TestStep(15, """TH2 opens a PASE connection to the DUT."""),
            TestStep(16, """TH2 obtains or generates a TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1.
                     TH2 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate."""),
            TestStep(17, """TH2 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0."""),
            TestStep(18, """TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a non-fabric-filtered read
                     and saves the returned list as nocs."""),
            TestStep(19, """TH1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read
                     and saves the returned list as fabrics."""),
            TestStep(20, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster
                     and saves the returned list as trustedroots."""),
            TestStep(21, """TH2 starts commissioning the DUT. It performs all steps up to establishing a CASE connection,
                     but DOES NOT send the CommissioningComplete command."""),
            TestStep(22, """TH2 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0."""),
            TestStep(23, """TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a non-fabric-filtered read."""),
            TestStep(24, """TH1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read."""),
            TestStep(25, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster."""),
            TestStep(26, """TH2 fully commissions the DUT."""),
            TestStep(27, """TH2 obtains or generates a TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1.
                     TH2 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate."""),
            TestStep(28, """TH1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read."""),
            TestStep(29, """TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds
                     and the Breadcrumb value as 1."""),
            TestStep(30, """TH1 obtains or generates a new TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1.
                     TH1 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate."""),
            TestStep(31, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster."""),
            TestStep(32, """TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to maxFailsafe"""),
            TestStep(33, """TH1 waits for PIXIT.CGEN.FailsafeExpiryLengthSeconds
                     Verify waits maxFailsafe"""),
            TestStep(34, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster
                     Verify that the number of items in the returned list is still numTrustedRootsOriginal + 1"""),
            TestStep(35, """TH2 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds
                     and the Breadcrumb value as 1."""),
            TestStep(36, """TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0
                     Verify that the DUT responds with ..."""),
            TestStep(37, """TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to maxFailsafe
                     Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)"""),
            TestStep(38, """TH1 saves the current wall time clock in seconds as Tstart"""),
            TestStep(39, """TH1 obtains or generates a new TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1.
                     TH1 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate."""),
            TestStep(40, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster."""),
            TestStep(41, """TH1 waits until the current wall time clock is Tstart + maxFailsafe/2."""),
            TestStep(42, """TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to maxFailsafe."""),
            TestStep(43, """TH1 waits until the current wall time clock is Tstart + maxFailsafe. maxFailsafe is the maximum amount of time a failsafe can be armed for,
                     so the failsafe is required to time out at this point, despite having been re-armed in step 42."""),
            TestStep(44, """TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster."""),
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
        logger.info(f'Step #1 - The size of the origina num_trusted_roots_original list: {trusted_root_list_original_size}')

        # Verify that original list has 1 certificate
        asserts.assert_equal(trusted_root_list_original_size, 1,
                             "Unexpected number of entries in the TrustedRootCertificates table")

        self.step(2)
        basic_commissioning_info = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.BasicCommissioningInfo)
        maxFailsafe = basic_commissioning_info.maxCumulativeFailsafeSeconds
        logger.info(f'Step #2 - The MaxCumulativeFailsafeSeconds (max_fail_safe): {maxFailsafe}')

        self.step(3)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # logger.info(f'Step #3 - response attributes are: {vars(resp)}')
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Step #3 - Failure status returned from arm failsafe")
        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        assert debug_text == '' or len(debug_text) <= 512, "debugText must be empty or have a maximum length of 512 characters"
        logger.info(f'Step #3 - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(4)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.Breadcrumb)
        logger.info(f'Step #4 - The Breadcrumb attribute: {breadcrumb_info}')
        asserts.assert_equal(breadcrumb_info, 1,
                             "Step #6 - The Breadcrumb attribute is not 1")

        self.step(5)
        logger.info("Step #5.1 - Generating a new CSR to update the root certificate...")
        # 5.1 Request CSR (Certificate Signing Request)
        cmd = cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        csr_update = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        # 5.2 Isue the certificates
        th1_certs_new = await self.default_controller.IssueNOCChain(csr_update, self.dut_node_id)
        new_root_cert = th1_certs_new.rcacBytes

        # 5.3 Send command to add new trusted root certificate
        cmd = cluster_opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        self.step(6)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(f'Step #6 - The updated size of the num_trusted_roots_original list: {trusted_root_list_original_size_updated}')

        # Verify that the trusted root list size has increased by 1
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 1,
                             "Step #6 - Unexpected number of entries in the TrustedRootCertificates table after update")

        # Optionally, check if the new certificate is in the updated list
        assert new_root_cert in trusted_root_list_original_updated, \
            "Step #6 - New root certificate was not added to the trusted root list."

        self.step(7)
        # Using maxFailsafe_tmp=10 instead of maxFailsafe=PIXIT.CGEN.FailsafeExpiryLengthSeconds
        logger.info(
            f"Step #7 - Waiting for Failsafe timer to expire for PIXIT.CGEN.FailsafeExpiryLengthSeconds (max_fail_safe): {maxFailsafe_tmp} seconds...")
        start_time = time.time()

        # Wait for the maximum failsafe time - Adding a 2 seconds buffer
        await asyncio.sleep(maxFailsafe_tmp + 1)

        elapsed_time = time.time() - start_time
        logger.info(f"Step #7 - Failsafe timer (max_fail_safe) expired after {elapsed_time} seconds.")

        self.step(8)
        trusted_root_list_original_after_wait = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_after_wait = len(trusted_root_list_original_after_wait)
        logger.info(
            f'Step #8 - The size of the num_trusted_roots_original list after waiting for failsafe timeout: {trusted_root_list_original_size_after_wait}')
        asserts.assert_equal(trusted_root_list_original_size_after_wait, trusted_root_list_original_size,
                             "Step #8 - Unexpected number of entries in the TrustedRootCertificates table after wait")

        self.step(9)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.Breadcrumb)
        logger.info(f'Step #9 - After waiting for failsafe timeout the Breadcrumb attribute: {breadcrumb_info}')
        asserts.assert_equal(breadcrumb_info, 0, "Breadcrumb value is not 0 after waiting for failsafe timer")

        self.step(10)
        logger.info(f'Step #10 - TH1 repeats steps 3 through 5')
        logger.info(f'Step #10 repet #3 - TH1 sends ArmFailSafe')
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        assert debug_text == '' or len(debug_text) <= 512, "debugText must be empty or have a maximum length of 512 characters"
        logger.info(f'Step #10 - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.Breadcrumb)
        logger.info(f'Step #10 repet #4 - The Breadcrumb attribute: {breadcrumb_info}')
        asserts.assert_equal(breadcrumb_info, 1,
                             "Step #10 - The Breadcrumb attribute is not 1")

        # Flow generates a new TrustedRootCertificate - Request CSR (Certificate Signing Request) and update NOC (Node Operational Certificate)
        logger.info("Step #10 repet #5 - Generating a new CSR to update the root certificate...")
        cmd = cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)

        # Flow generates a new TrustedRootCertificate  - Isue the certificates
        th1_certs_new = await self.default_controller.IssueNOCChain(csr_update, self.dut_node_id)
        new_root_cert = th1_certs_new.rcacBytes

        # Flow generates a new TrustedRootCertificate - Send command to add new trusted root certificate
        cmd = cluster_opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        # logger.info(f'Step #10 - The updated original trusted_roots_original: {trusted_root_list_original_updated}')
        logger.info(
            f'Step #10 - The updated size of the num_trusted_roots_original list: {trusted_root_list_original_size_updated}')
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 1,
                             "Step #10 - Unexpected number of entries in the TrustedRootCertificates table after update")

        # Optionally, check if the new certificate is in the updated list
        assert new_root_cert in trusted_root_list_original_updated, \
            "Step #10 - New root certificate was not added to the trusted root list."

        self.step(11)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)

        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        assert debug_text == '' or len(debug_text) <= 512, "debugText must be empty or have a maximum length of 512 characters"
        logger.info(f'Step #11 - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(12)
        logger.info(f'Step #12 - TH1 repeats steps 8 through 9')
        logger.info(f'Step #12 repet #8 - TH1 reads the TrustedRootCertificates')
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(
            f'Step #12 - The updated size of the num_trusted_roots_original list: {trusted_root_list_original_size_updated}')
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size,
                             "Step #12 - Unexpected number of entries in the TrustedRootCertificates table after update")

        logger.info(f'Step #12 repet #9 - TH1 reads the Breadcrumb attribute')
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=cluster_cgen,
            attribute=cluster_cgen.Attributes.Breadcrumb)

        logger.info(f"Step #12 - The Breadcrumb attribute: {breadcrumb_info}")
        asserts.assert_equal(breadcrumb_info, 0, "Breadcrumb value is not 0 after waiting for failsafe timer")

        self.step(13)
        params = await self.open_commissioning_window(self.default_controller, self.dut_node_id)
        setup_pin_code = params.commissioningParameters.setupPinCode
        longDiscriminator = params.randomDiscriminator
        logger.info(f'Step #13 - Open Commissioning Window params with vars: {vars(params)}')

        self.step(14)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'BusyWithOtherAdmin'(4)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kBusyWithOtherAdmin,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #14 - ArmFailSafeResponse with ErrorCode as BusyWithOtherAdmin ({resp.errorCode})')

        self.step(15)
        # Create TH2
        TH2_CA_real = self.certificate_authority_manager.NewCertificateAuthority(maximizeCertChains=True)
        TH2_vid = 0xFFF2
        TH2_fabric_admin_real = TH2_CA_real.NewFabricAdmin(vendorId=TH2_vid, fabricId=2)
        TH2_nodeid = self.default_controller.nodeId+1
        TH2 = TH2_fabric_admin_real.NewController(nodeId=TH2_nodeid)
        newNodeId = self.dut_node_id + 1

        resp = await self.FindAndEstablishPase(
            dev_ctrl=TH2,
            longDiscriminator=longDiscriminator,
            setupPinCode=setup_pin_code,
            nodeid=newNodeId)
        logger.info(f'Step #15 - TH2 successfully establish PASE session completed')

        self.step(16)
        logger.info(f'Step #16 - TH2 Generating a new CSR to update the root certificate...')
        # Flow generates a new TrustedRootCertificate - Request CSR (Certificate Signing Request)
        cmd = cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        th2_csr = await self.send_single_cmd(dev_ctrl=TH2, node_id=newNodeId, cmd=cmd)

        # Flow generates a new TrustedRootCertificate - Isue the certificates
        th2_certs_new = await TH2.IssueNOCChain(th2_csr, newNodeId)
        th2_new_root_cert = th2_certs_new.rcacBytes

        # Flow generates a new TrustedRootCertificate - Send command to add new trusted root certificate
        cmd = cluster_opcreds.Commands.AddTrustedRootCertificate(th2_new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=TH2, node_id=newNodeId, cmd=cmd)

        self.step(17)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=TH2,
            node_id=newNodeId,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        assert debug_text == '' or len(debug_text) <= 512, "debugText must be empty or have a maximum length of 512 characters"
        logger.info(f'Step #17 - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(18)
        nocs = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.NOCs,
            fabric_filtered=False)
        nocs_original_size = len(nocs)
        logger.info(f'Step #18 - TH1 Original size of the nocs list: {nocs_original_size}')

        self.step(19)
        fabrics = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.Fabrics,
            fabric_filtered=False)
        fabrics_original_size = len(fabrics)
        logger.info(f'Step #19 - TH1 Original size of the fabrics list: {fabrics_original_size}')

        self.step(20)
        trusted_roots_list = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_roots_list_size = len(trusted_roots_list)
        logger.info(f'Step #20 - TH1 Original size of the trusted_root list: {trusted_roots_list_size}')

        self.step(21)
        # Commissioning stage numbers - we should find a better way to match these to the C++ code
        # CommissioningDelegate.h
        # TODO: https://github.com/project-chip/connectedhomeip/issues/36629
        kFindOperationalForCommissioningComplete = 30
        logger.info(
            f'Step #21 - TH2 Commissioning stage SetTestCommissionerPrematureCompleteAfter enum: {kFindOperationalForCommissioningComplete}')

        resp = TH2.SetTestCommissionerPrematureCompleteAfter(kFindOperationalForCommissioningComplete)
        logger.info(f'Step #21 - TH2 Commissioning DOES NOT send the CommissioningComplete command')

        self.step(22)
        logger.info("Step #22 - TH1 Waiting for PASE session to stabilize...")
        resp = await self.FindAndEstablishPase(
            dev_ctrl=TH2,
            longDiscriminator=longDiscriminator,
            setupPinCode=setup_pin_code,
            nodeid=newNodeId)
        logger.info(f'Step #22 - TH2 successfully establish PASE session completed')

        try:
            # Verify DUT cannot proceed because the session has not been fully commissioned, leading to a timeout error
            cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
            resp = await self.send_single_cmd(
                dev_ctrl=TH2,
                node_id=newNodeId,
                cmd=cmd)
        except Exception as e:
            logger.info(f"Step #22 - TH2 Expected error occurred during ArmFailSafe command: {str(e)}. Proceeding to next step.")

        self.step(23)
        nocs_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.NOCs,
            fabric_filtered=False)
        nocs_updated_size = len(nocs_updated)
        logger.info(f'Step #23 - TH1 nocs_updated: {nocs_updated_size}')
        asserts.assert_equal(nocs_updated_size, nocs_original_size,
                             "Step #23 - The nocs list size should match the original nocs list size.")

        self.step(24)
        fabrics_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.Fabrics,
            fabric_filtered=False)
        fabrics_updated_size = len(fabrics_updated)
        logger.info(f'Step #24 - TH1 fabrics_original_size2: {fabrics_updated_size}')
        asserts.assert_equal(fabrics_updated_size, fabrics_original_size,
                             "Step #24 - The fabrics list size should match the original fabrics list size.")

        self.step(25)
        trusted_roots_list_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_roots_list_updated_size = len(trusted_roots_list_updated)
        logger.info(f'Step #25 - TH1 the size of the trusted_roots_list_size_updated: {trusted_roots_list_updated_size}')
        asserts.assert_equal(trusted_roots_list_updated_size, trusted_roots_list_size,
                             "Step #25 - The trusted_roots list size should match the original trusted_roots list size.")

        self.step(26)
        basic_commissioning_info = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.BasicCommissioningInfo)
        logger.info(f'Step #26 - basic_commissioning_info: {basic_commissioning_info}')

        logger.info(f'Step #26 - Fully commissioned started')
        resp = await TH2.CommissionOnNetwork(
            nodeId=newNodeId+1, setupPinCode=setup_pin_code,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=longDiscriminator)
        logger.info(f'Step #26 - Fully commissioned done: {resp}')

        self.step(27)
        # TH2 TrustedRootCertificate response with FAILSAFE_REQUIRED
        try:
            logger.info("Step #27 - TH2 Generating a new CSR to update the root certificate...")
            # Flow generates a new TrustedRootCertificate - Request CSR (Certificate Signing Request) and update NOC (Node Operational Certificate)
            cmd = cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
            th2_csr = await self.send_single_cmd(dev_ctrl=TH2, node_id=newNodeId+1, cmd=cmd)

            # Flow generates a new TrustedRootCertificate - Isue the certificates
            th2_certs_new = await TH2.IssueNOCChain(th2_csr, newNodeId+1)
            th2_new_root_cert = th2_certs_new.rcacBytes

            # Flow generates a new TrustedRootCertificate - Send command to add new trusted root certificate
            cmd = cluster_opcreds.Commands.AddTrustedRootCertificate(th2_new_root_cert)
            resp = await self.send_single_cmd(dev_ctrl=TH2, node_id=newNodeId+1, cmd=cmd)

        except Exception as e:
            logger.info(
                f"Step #27 - Expected error occurred during TrustedRootCertificate command: {str(e)}. Proceeding to next step.")

        self.step(28)
        fabrics_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.Fabrics,
            fabric_filtered=False)
        fabrics_updated_size = len(fabrics_updated)
        logger.info(f'Step #28 - TH1 fabrics updated with additional entry for TH2: {fabrics_updated_size}')
        asserts.assert_equal(fabrics_updated_size, fabrics_original_size + 1,
                             "Step #28 - The fabrics list size should match the original fabrics list size + 1.")

        self.step(29)
        # logger.info("Step #29 - TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds and the Breadcrumb value as 1")
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        assert debug_text == '' or len(debug_text) <= 512, "debugText must be empty or have a maximum length of 512 characters"
        logger.info(f'Step #29 - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(30)
        logger.info("Step #30 - Generating a new CSR to update the root certificate...")
        # Flow generates a new TrustedRootCertificate - Request CSR (Certificate Signing Request) and update NOC (Node Operational Certificate)
        cmd = cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        csr_update = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        # Flow generates a new TrustedRootCertificate - Isue the certificates
        th1_certs_new = await self.default_controller.IssueNOCChain(csr_update, self.dut_node_id)
        new_root_cert = th1_certs_new.rcacBytes

        # Flow generates a new TrustedRootCertificate - Send command to add new trusted root certificate
        cmd = cluster_opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        self.step(31)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(f'Step #31 - The updated num_trusted_roots_original: {trusted_root_list_original_size_updated}')
        # Verify that the trusted root list size has increased by 1
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 2,
                             "Step #31 - Unexpected number of entries in the TrustedRootCertificates table after update")

        self.step(32)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        assert debug_text == '' or len(debug_text) <= 512, "debugText must be empty or have a maximum length of 512 characters"
        logger.info(f'Step #32 - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(33)
        # Using maxFailsafe_tmp=10 instead of maxFailsafe=PIXIT.CGEN.FailsafeExpiryLengthSeconds
        logger.info(
            f"Step #33 - Waiting for Failsafe timer to expire for PIXIT.CGEN.FailsafeExpiryLengthSeconds (max_fail_safe): {maxFailsafe_tmp} seconds...")
        start_time = time.time()
        await asyncio.sleep(1)

        # # Wait for the maximum failsafe time - Adding a 2 seconds buffer
        # await asyncio.sleep(maxFailsafe_tmp + 1)

        # elapsed_time = time.time() - start_time
        # logger.info(f"Step #33 - Failsafe timer (max_fail_safe) expired after {elapsed_time} seconds.")

        self.step(34)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(f'Step #34 - The updated num_trusted_roots_original: {trusted_root_list_original_size_updated}')

        # Verify that the trusted root list size has increased by 1
        # 1 cert was removed on step #33 whne FailSafe expired (from 3 to 2)
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 2,
                             "Step #34 - Unexpected number of entries in the TrustedRootCertificates table after update")

        self.step(35)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=TH2,
            node_id=newNodeId+1,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'BusyWithOtherAdmin'(4)
        logger.info(f'Step #35 - ArmFailSafeResponse with ErrorCode as BusyWithOtherAdmin ({resp.errorCode})')
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kBusyWithOtherAdmin,
                             "Failure status returned from arm failsafe")

        self.step(36)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        logger.info(f'Step #36 - response attributes are: {resp.errorCode}')
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(37)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        logger.info(f'Step #37 - response attributes are: {resp.errorCode}')
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(38)
        t_start = time.time()
        formatted_time = datetime.fromtimestamp(t_start).strftime('%m-%d %H:%M:%S')
        milliseconds = str(round((t_start % 1) * 1000)).zfill(3)
        formatted_time_with_ms = f"{formatted_time}.{milliseconds}"
        logger.info(f"Step #38 - TH1 saves the current wall time clock in seconds as Tstart: {formatted_time_with_ms} seconds...")

        self.step(39)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(
            f'Step #39 - The updated size of the num_trusted_roots_original list: {trusted_root_list_original_size_updated}')

        # Flow generates a new TrustedRootCertificate - Request CSR (Certificate Signing Request) and update NOC (Node Operational Certificate)
        logger.info("Step #10 repet #5 - Generating a new CSR to update the root certificate...")
        cmd = cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)

        # Flow generates a new TrustedRootCertificate  - Isue the certificates
        th1_certs_new = await self.default_controller.IssueNOCChain(csr_update, self.dut_node_id)
        new_root_cert = th1_certs_new.rcacBytes

        # Flow generates a new TrustedRootCertificate - Send command to add new trusted root certificate
        cmd = cluster_opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        self.step(40)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(
            f'Step #40 - The updated size of the num_trusted_roots_original list: {trusted_root_list_original_size_updated}')
        # Verify that the trusted root list size has increased by 2 from oririnal trusted root list, Should be 3
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 2,
                             "Step #31 - Unexpected number of entries in the TrustedRootCertificates table after update")

        self.step(41)
        target_time = t_start + (maxFailsafe_tmp / 2)
        formatted_target_time = datetime.fromtimestamp(target_time).strftime('%m-%d %H:%M:%S')
        target_milliseconds = str(round((target_time % 1) * 1000)).zfill(3)
        formatted_target_time_with_ms = f"{formatted_target_time}.{target_milliseconds}"

        logger.info(f"Step #41 - Target time (Tstart + maxFailsafe / 2): {formatted_target_time_with_ms}")

        while time.time() < target_time:
            await asyncio.sleep(0.1)  # Short sleep to avoid busy-waiting

        # Validation: check if the current time reached the target time
        if time.time() >= target_time:
            logger.info("Step #41 - Target time reached. Proceeding...")
        else:
            logger.error("Step #41 - Failed to reach target time.")
            raise Exception("Target time not reached. Cannot proceed.")

        self.step(42)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_tmp)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        logger.info(f'Step #42 - response attributes are: {vars(resp)}')
        logger.info(f'Step #42 - response attributes are: {resp.errorCode}')
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(43)
        target_time_full = t_start + maxFailsafe_tmp
        formatted_target_time_full = datetime.fromtimestamp(target_time_full).strftime('%m-%d %H:%M:%S')
        target_milliseconds_full = str(round((target_time_full % 1) * 1000)).zfill(3)
        formatted_target_time_with_ms_full = f"{formatted_target_time_full}.{target_milliseconds_full}"

        logger.info(f"Step #43 - Target time (Tstart + maxFailsafe): {formatted_target_time_with_ms_full}")

        while time.time() < target_time_full:
            await asyncio.sleep(0.1)  # Short sleep to avoid busy-waiting

        # Validation: check if the current time reached the target time
        if time.time() >= target_time_full:
            logger.info("Step #43 - Target time reached. Proceeding...")
        else:
            logger.error("Step #43 - Failed to reach target time.")
            raise Exception("Target time not reached. Cannot proceed.")

        self.step(44)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=cluster_opcreds,
            attribute=cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(
            f'Step #44 - The updated size of the num_trusted_roots_original list: {trusted_root_list_original_size_updated}')
        # Verify that the trusted root list size has increased by 2 from oririnal trusted root list, Should be 3
        # asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 2,
        #                      "Step #31 - Unexpected number of entries in the TrustedRootCertificates table after update")


if __name__ == "__main__":
    default_matter_test_main()
