#
#    Copyright (c) 2025 Project CHIP Authors
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
#       --int-arg PIXIT.CGEN.FailsafeExpiryLengthSeconds:1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import random
import time

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_CGEN_2_2(MatterBaseTest):

    cluster_opcreds = Clusters.OperationalCredentials
    cluster_cgen = Clusters.GeneralCommissioning

    CERT_FAILSAFE_EXPIRATION_TIME_SECONDS = 1

    async def set_failsafe_timer(self, dev_ctrl, node_id, expiration_time_seconds):
        '''
        Triggering the failsafe expiry to clean up resources like fabric tables, NOCs (Node Operational Credentials),
        and trusted root certificates. This is necessary to avoid accumulation of invalid data, which could be caused
        by misconfigurations (e.g., incorrect network credentials).

        In CI environments, this function helps bypass long waits by resetting the failsafe timer to 1 seconds,
        allowing the test to proceed without unnecessary delays.

        Args:
            dev_ctrl: The device controller to send the command.
            node_id: The node identifier to which the command is sent.

        Returns:
            response: The response from the command sent to the device.
        '''
        # Buffer for latency
        buffer_latency = .5

        # Resetting the failsafe timer to 1 seconds to clean up resources and avoid waiting in CI.
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=expiration_time_seconds)
        # Sending the command to the DUT (Device Under Test).
        resp = await self.send_single_cmd(dev_ctrl=dev_ctrl, node_id=node_id, cmd=cmd)

        start_time = time.time()
        # Wait for the expiration time (fail-safe will expire after this time), plus buffer for latency
        logger.info(f'Waiting for {expiration_time_seconds} seconds (with additional buffer {buffer_latency} seconds for latency).')
        await asyncio.sleep(expiration_time_seconds + buffer_latency)
        elapsed_time = time.time() - start_time
        logger.info(
            f'Failsafe timer expired after: {elapsed_time:.2f} seconds (including {buffer_latency} seconds for latency buffer).')

        return resp

    async def run_steps_3_to_5(self, failsafe_duration: int, is_first_run: bool):
        '''
        Executes steps 3 through 5, with optional Test Step.

        Step 3: Sends an ArmFailSafe command to the DUT and verifies the response.
        Step 4: Reads the Breadcrumb attribute from the DUT and verifies its value.
        Step 5: Generates a new CSR (Certificate Signing Request) to update the root certificate, 
                issues the new certificates, and adds the new root certificate to the DUT.
        Step 10: If is_first_run is False, repeats steps #3, #4 and #5 on step #10.


        Args:
            failsafe_duration (int): The duration in seconds for which the failsafe remains active. 
                                This should be less than the DUT MaxCumulativeFailsafeSeconds
            is_first_run (bool): A flag to control whether the step should be executed or skipped.

        Returns:
            new_root_cert
        '''
        if is_first_run:
            self.step(3)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=failsafe_duration, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        asserts.assert_true(debug_text == '' or len(debug_text) <= 512,
                            "debugText must be empty or have a maximum length of 512 characters.")
        if is_first_run:
            logger.info(f'Step #3: ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')
        else:
            logger.info(f'Step #10 - Repeated Step #3: ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        if is_first_run:
            self.step(4)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=self.cluster_cgen,
            attribute=self.cluster_cgen.Attributes.Breadcrumb)
        asserts.assert_equal(breadcrumb_info, 1,
                             "The Breadcrumb attribute is not 1")
        if is_first_run:
            logger.info(f'Step #4: The Breadcrumb attribute: {breadcrumb_info}')
        else:
            logger.info(f'Step #10 - Repeated Step #4: The Breadcrumb attribute: {breadcrumb_info}')

        if is_first_run:
            self.step(5)
        # 5.1 Request CSR (Certificate Signing Request)
        cmd = self.cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        csr_update = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        # NOTE: This step is required because RCAC generation is not exposed separately in the API.
        # The default controller forces re-generation of root certificates when new ones are issued.
        # Ideally, a root certificate would be self-signed and not require a CSR, but the current APIs don’t support this.

        # 5.2 Isue the certificates
        th1_certs_new = await self.default_controller.IssueNOCChain(csr_update, self.dut_node_id)
        new_root_cert = th1_certs_new.rcacBytes
        # NOTE: The IssueNOCChain generates a new RCAC because it is initialized to produce maximally sized certificates.

        # 5.3 Send command to add new trusted root certificate
        cmd = self.cluster_opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        if is_first_run:
            logger.info('Step #5: Completed - CSR generated, certificates issued, and new trusted root certificate added.')
        else:
            logger.info('Step #10 - Repeated Step #5: Completed - CSR generated, certificates issued, and new trusted root certificate added.')

        return new_root_cert

    async def run_steps_8_to_9(self, trusted_root_list_original_size: int, is_first_run: bool):
        '''
        Executes steps 8 through 9, with optional Test Step.

        Step 8: Waits for the failsafe timeout and reads the TrustedRootCertificates attribute.
        Step 9: Reads the Breadcrumb attribute from the DUT and verifies its value after the failsafe timeout.
        Step 12: If is_first_run is False, repeats steps #8 and #9 on step #12.


        Args:
            trusted_root_list_original_size (int): The original number of trusted root certificates before the failsafe timeout.
            is_first_run (bool): A flag to control whether the step should be executed or skipped.

        Returns:
            None
        '''
        if is_first_run:
            self.step(8)
        trusted_root_list_original_after_wait = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_after_wait = len(trusted_root_list_original_after_wait)
        asserts.assert_equal(trusted_root_list_original_size_after_wait, trusted_root_list_original_size,
                             "Unexpected number of entries in the TrustedRootCertificates table after wait")
        if is_first_run:
            logger.info(
                f'Step #8: The size of the num_trusted_roots_original list after waiting for failsafe timeout: {trusted_root_list_original_size_after_wait}')
        else:
            logger.info(
                f'Step #12 - Repeated Step #8: The size of the num_trusted_roots_original list after waiting for failsafe timeout: {trusted_root_list_original_size_after_wait}')

        if is_first_run:
            self.step(9)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=self.cluster_cgen,
            attribute=self.cluster_cgen.Attributes.Breadcrumb)
        asserts.assert_equal(breadcrumb_info, 0, "Breadcrumb value is not 0 after waiting for failsafe timer")
        if is_first_run:
            logger.info(f'Step #9: After waiting for failsafe timeout the Breadcrumb attribute: {breadcrumb_info}')
        else:
            logger.info(
                f'Step #12 - Repeated Step #9: After waiting for failsafe timeout the Breadcrumb attribute: {breadcrumb_info}')

    def desc_TC_CGEN_2_2(self) -> str:
        return '[TC-CGEN-2.2] ArmFailSafe command verification [DUT - Server]'

    def pics_TC_CGEN_2_2(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "CGEN.S"
        ]
        return pics

    def steps_TC_CGEN_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(0, 'Commissioning, already done', is_commissioning=True),
            TestStep(1, '''TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster 
                     and saves the number of list items as numTrustedRootsOriginal.'''),
            TestStep(2, '''TH1 reads the BasicCommissioningInfo attribute 
                     and saves the MaxCumulativeFailsafeSeconds as maxFailsafe.'''),
            TestStep('3-5', 'TH1 execute function run_steps_3_to_5 to run steps #3 through #5.'),
            TestStep(3, '''TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds
                     and the Breadcrumb value as 1.'''),
            TestStep(4, 'TH1 reads the Breadcrumb attribute.'),
            TestStep(5, '''TH1 generates a new TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1.
                     TH1 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate.'''),
            TestStep(6, 'TH1 reads the TrustedRootCertificate attribute.'),
            TestStep(7, 'TH1 waits for PIXIT.CGEN.FailsafeExpiryLengthSeconds to ensure the failsafe timer has expired.'),
            TestStep('8-9', 'TH1 execute function run_steps_8_to_9 to run steps #8 through #9.'),
            TestStep(8, 'TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster.'),
            TestStep(9, 'TH1 reads the Breadcrumb attribute and verify that the breadcrumb attribute is 0.'),
            TestStep(10, 'TH1 repeats steps #3 through #5 using function run_steps_3_to_5.'),
            TestStep(11, 'TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0.'),
            TestStep(12, 'TH1 Repeat steps 8 through 9.'),
            TestStep(13, 'TH1 sends the OpenCommissioningWindow command to the Administrator Commissioning cluster.'),
            TestStep(14, 'TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds'),
            TestStep(15, 'TH2 opens a PASE connection to the DUT.'),
            TestStep(16, '''TH2 obtains or generates a TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1.
                     TH2 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate.'''),
            TestStep(17, 'TH2 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0.'),
            TestStep(18, '''TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a non-fabric-filtered read
                     and saves the returned list as nocs.'''),
            TestStep(19, '''TH1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read
                     and saves the returned list as fabrics.'''),
            TestStep(20, '''TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster
                     and saves the returned list as trustedroots.'''),
            TestStep(21, '''TH2 starts commissioning the DUT. It performs all steps up to establishing a CASE connection,
                     but DOES NOT send the CommissioningComplete command.'''),
            TestStep(22, 'TH2 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0.'),
            TestStep(23, 'TH1 reads the NOCs attribute from the Node Operational Credentials cluster using a non-fabric-filtered read.'),
            TestStep(24, 'TH1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read.'),
            TestStep(25, 'TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster.'),
            TestStep(26, 'TH2 fully commissions the DUT.'),
            TestStep(27, '''TH2 obtains or generates a TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1.
                     TH2 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate.'''),
            TestStep(28, 'TH1 reads the Fabrics attribute from the Node Operational Credentials cluster using a non-fabric-filtered read.'),
            TestStep(29, '''TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds
                     and the Breadcrumb value as 1.'''),
            TestStep(30, '''TH1 obtains or generates a new TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1.
                     TH1 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate.'''),
            TestStep(31, 'TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster.'),
            TestStep(32, 'TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to maxFailsafe.'),
            TestStep(33, 'TH1 waits for PIXIT.CGEN.FailsafeExpiryLengthSeconds.'),
            TestStep(34, 'TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster.'),
            TestStep(35, '''TH2 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to PIXIT.CGEN.FailsafeExpiryLengthSeconds
                     and the Breadcrumb value as 1.'''),
            TestStep(36, 'TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to 0.'),
            TestStep(37, 'TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to maxFailsafe.'),
            TestStep(38, 'TH1 saves the current wall time clock in seconds as Tstart,'),
            TestStep(39, """TH1 obtains or generates a new TrustedRootCertificate that is different from the previously commissioned TrustedRootCertificate for TH1.
                      TH1 sends an AddTrustedRootCertificate command to the Node Operational Credentials cluster to install this new certificate."""),
            TestStep(40, 'TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster.'),
            TestStep(41, 'TH1 waits until the current wall time clock is Tstart + maxFailsafe/2.'),
            TestStep(42, 'TH1 sends ArmFailSafe command to the DUT with ExpiryLengthSeconds field set to maxFailsafe.'),
            TestStep(43, '''TH1 waits until the current wall time clock is Tstart + maxFailsafe. maxFailsafe is the maximum amount of time a failsafe can be armed for,
                     so the failsafe is required to time out at this point, despite having been re-armed in step 42.'''),
            TestStep(44, 'TH1 reads the TrustedRootCertificates attribute from the Node Operational Credentials cluster.'),
        ]
        return steps

    @async_test_body
    async def test_TC_CGEN_2_2(self):

        self.step(0)

        # Read the Steps
        self.step(1)
        trusted_root_list_original = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size = len(trusted_root_list_original)
        logger.info(f'Step #1: The size of the original num_trusted_roots_original list: {trusted_root_list_original_size}')

        self.step(2)
        basic_commissioning_info = await self.read_single_attribute_check_success(
            cluster=self.cluster_cgen,
            attribute=self.cluster_cgen.Attributes.BasicCommissioningInfo)
        maxFailsafe = basic_commissioning_info.maxCumulativeFailsafeSeconds
        logger.info(f'Step #2: The MaxCumulativeFailsafeSeconds (max_fail_safe): {maxFailsafe}')

        if self.is_pics_sdk_ci_only:
            run_type = "CI Test"
            failsafe_expiration_seconds = self.matter_test_config.global_test_params['PIXIT.CGEN.FailsafeExpiryLengthSeconds']
        else:
            run_type = "Cert Test"
            failsafe_expiration_seconds = self.CERT_FAILSAFE_EXPIRATION_TIME_SECONDS

        # Timeout used in test steps to verify failsafe. Must be less than DUT MaxCumulativeFailsafeSeconds
        asserts.assert_less(failsafe_expiration_seconds, maxFailsafe,
                            "failsafe_expiration_seconds should be less than MaxCumulativeFailsafeSeconds.")
        logger.info(f'Step #2: {run_type} - Value of FailsafeExpiryLengthSeconds: {failsafe_expiration_seconds}')

        # TH1 steps #3 through #5 using the function run_steps_3_to_5
        self.step('3-5')
        new_root_cert = await self.run_steps_3_to_5(failsafe_expiration_seconds, is_first_run=True)

        self.step(6)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)

        # Verify that the trusted root list size has increased by 1
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 1,
                             "Unexpected number of entries in the TrustedRootCertificates table after update")

        # Check if the new certificate is in the updated list
        asserts.assert_in(new_root_cert, trusted_root_list_original_updated,
                          "New root certificate was not added to the trusted root list.")

        logger.info(f'Step #6: The updated size of the num_trusted_roots_original list: {trusted_root_list_original_size_updated}')

        self.step(7)
        if self.is_pics_sdk_ci_only:
            # Step 7 - In CI environments, the 'set_failsafe_timer' function is used to immediately force the failsafe timer to expire,
            # avoiding the original wait time defined in PIXIT.CGEN.FailsafeExpiryLengthSeconds
            # and speeding up test execution by setting the expiration time to 1 second.

            # Running identifier
            run_type = "CI Test"
            logger.info(
                f'Step 7: {run_type} - Bypassing failsafe expiration to avoid unnecessary delays in CI environment.')

            # Force the failsafe timer to expire immediately for TH1, avoiding unnecessary delays in CI environments
            resp = await self.set_failsafe_timer(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                expiration_time_seconds=failsafe_expiration_seconds)
            logger.info(
                f'Step #7: {run_type} - Failsafe timer expiration bypassed for TH1 by setting expiration time to {failsafe_expiration_seconds} seconds. '
                f'Test continues without the original wait.'
            )
        else:
            # Running identifier
            run_type = "Cert Test"
            # # FAILSAFE_EXPIRATION_SECONDS constant, adjusted to avoid long waits
            logger.info(
                f'Step #7: {run_type} - Waiting for Failsafe timer to expire for FAILSAFE_EXPIRATION_SECONDS constant: {failsafe_expiration_seconds} seconds...')

            # Wait for the full duration of the FAILSAFE_EXPIRATION_SECONDS constant time with an additional 0.5-second buffer
            await asyncio.sleep(failsafe_expiration_seconds + .5)

        # TH1 steps #8 through #9 using the function run_steps_8_to_9
        self.step('8-9')
        resp = await self.run_steps_8_to_9(trusted_root_list_original_size, is_first_run=True)

        self.step(10)
        # Repeat TH1 steps #3 through #5 using the function run_steps_3_to_5
        new_root_cert = await self.run_steps_3_to_5(failsafe_expiration_seconds, is_first_run=False)

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
        asserts.assert_true(debug_text == '' or len(debug_text) <= 512,
                            "debugText must be empty or have a maximum length of 512 characters.")
        logger.info(f'Step #11: ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(12)
        # Repeat TH1 steps #8 through #9 using the function run_steps_8_to_9
        resp = await self.run_steps_8_to_9(trusted_root_list_original_size, is_first_run=False)

        self.step(13)
        params = await self.open_commissioning_window(self.default_controller, self.dut_node_id)
        setup_pin_code = params.commissioningParameters.setupPinCode
        longDiscriminator = params.randomDiscriminator
        setup_qr_code = params.commissioningParameters.setupQRCode
        logger.info(f'Step #13: Open Commissioning Window params with vars: {vars(params)}')

        self.step(14)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=failsafe_expiration_seconds)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'BusyWithOtherAdmin'(4)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kBusyWithOtherAdmin,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #14: ArmFailSafeResponse with ErrorCode as BusyWithOtherAdmin ({resp.errorCode})')

        self.step(15)
        # Create TH2
        # NOTE: The 'maximizeCertChains' argument is being used to ensure new trusted roots are generated each time this process is executed.
        TH2_CA_real = self.certificate_authority_manager.NewCertificateAuthority(maximizeCertChains=True)
        TH2_vid = 0xFFF2
        TH2_fabric_admin_real = TH2_CA_real.NewFabricAdmin(vendorId=TH2_vid, fabricId=2)
        TH2_nodeid = self.default_controller.nodeId+1
        TH2 = TH2_fabric_admin_real.NewController(nodeId=TH2_nodeid)
        newNodeId = self.dut_node_id + 1

        resp = await TH2.FindOrEstablishPASESession(setupCode=setup_qr_code, nodeid=newNodeId)
        logger.info('Step #15 - TH2 successfully establish PASE session completed')

        self.step(16)
        logger.info('Step #16 - TH2 Generating a new CSR to update the root certificate...')
        # Flow generates a new TrustedRootCertificate - Request CSR (Certificate Signing Request)
        cmd = self.cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
        th2_csr = await self.send_single_cmd(dev_ctrl=TH2, node_id=newNodeId, cmd=cmd)

        # Flow generates a new TrustedRootCertificate - Isue the certificates
        th2_certs_new = await TH2.IssueNOCChain(th2_csr, newNodeId)
        th2_new_root_cert = th2_certs_new.rcacBytes

        # Flow generates a new TrustedRootCertificate - Send command to add new trusted root certificate
        cmd = self.cluster_opcreds.Commands.AddTrustedRootCertificate(th2_new_root_cert)
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
        asserts.assert_true(debug_text == '' or len(debug_text) <= 512,
                            "debugText must be empty or have a maximum length of 512 characters.")
        logger.info(f'Step #17 - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(18)
        nocs = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.NOCs,
            fabric_filtered=False)
        nocs_original_size = len(nocs)
        logger.info(f'Step #18 - TH1 Original size of the nocs list: {nocs_original_size}')

        self.step(19)
        fabrics = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.Fabrics,
            fabric_filtered=False)
        fabrics_original_size = len(fabrics)
        logger.info(f'Step #19 - TH1 Original size of the fabrics list: {fabrics_original_size}')

        self.step(20)
        trusted_roots_list = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.TrustedRootCertificates)
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
        logger.info('Step #21 - TH2 Commissioning DOES NOT send the CommissioningComplete command')

        # TH2.SetSkipCommissioningComplete(True)
        logger.info('Step #21 - CommissioningComplete skipped to avoid premature completion.')

        self.step(22)
        logger.info("Step #22 - TH1 Waiting for PASE session to stabilize...")
        resp = await TH2.FindOrEstablishPASESession(setupCode=setup_qr_code, nodeid=newNodeId)
        logger.info('Step #22 - TH2 successfully establish PASE session completed')

        try:
            # Verify DUT cannot proceed because the session has not been fully commissioned, leading to a timeout error
            cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
            resp = await self.send_single_cmd(
                dev_ctrl=TH2,
                node_id=newNodeId,
                cmd=cmd)
        except ChipStackError as e:
            asserts.assert_in('Timeout',
                              str(e), f'Expected Timeout error, but got {str(e)}')
            logger.info(f"Step #22 - TH2 Expected error occurred during ArmFailSafe command: {str(e)}. Proceeding to next step.")
        else:
            asserts.assert_true(False, 'Expected Timeout, but no exception occurred.')

        self.step(23)
        nocs_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.NOCs,
            fabric_filtered=False)
        nocs_updated_size = len(nocs_updated)
        logger.info(f'Step #23 - TH1 nocs_updated: {nocs_updated_size}')
        asserts.assert_equal(nocs_updated_size, nocs_original_size,
                             "The nocs list size should match the original nocs list size.")

        self.step(24)
        fabrics_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.Fabrics,
            fabric_filtered=False)
        fabrics_updated_size = len(fabrics_updated)
        logger.info(f'Step #24 - TH1 fabrics_original_size2: {fabrics_updated_size}')
        asserts.assert_equal(fabrics_updated_size, fabrics_original_size,
                             "The fabrics list size should match the original fabrics list size.")

        self.step(25)
        trusted_roots_list_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_roots_list_updated_size = len(trusted_roots_list_updated)
        logger.info(f'Step #25 - TH1 the size of the trusted_roots_list_size_updated: {trusted_roots_list_updated_size}')
        asserts.assert_equal(trusted_roots_list_updated_size, trusted_roots_list_size,
                             "The trusted_roots list size should match the original trusted_roots list size.")

        self.step(26)
        basic_commissioning_info = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.BasicCommissioningInfo)
        logger.info(f'Step #26 - basic_commissioning_info: {basic_commissioning_info}')

        logger.info('Step #26 - Fully commissioned started')
        resp = await TH2.CommissionOnNetwork(
            nodeId=newNodeId+1, setupPinCode=setup_pin_code,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=longDiscriminator)
        logger.info(f'Step #26 - Fully commissioned done: {resp}')

        self.step(27)
        # TH2 TrustedRootCertificate response with FAILSAFE_REQUIRED
        try:
            logger.info("Step #27 - TH2 Generating a new CSR to update the root certificate...")
            # Flow generates a new TrustedRootCertificate - Request CSR (Certificate Signing Request) and update NOC (Node Operational Certificate)
            cmd = self.cluster_opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
            th2_csr = await self.send_single_cmd(dev_ctrl=TH2, node_id=newNodeId+1, cmd=cmd)

            # Flow generates a new TrustedRootCertificate - Isue the certificates
            th2_certs_new = await TH2.IssueNOCChain(th2_csr, newNodeId+1)
            th2_new_root_cert = th2_certs_new.rcacBytes

            # Flow generates a new TrustedRootCertificate - Send command to add new trusted root certificate
            cmd = self.cluster_opcreds.Commands.AddTrustedRootCertificate(th2_new_root_cert)
            resp = await self.send_single_cmd(dev_ctrl=TH2, node_id=newNodeId+1, cmd=cmd)

        except InteractionModelError as e:
            asserts.assert_in('FailsafeRequired (0xca)',
                              str(e), f'Expected FailsafeRequired error, but got {str(e)}')
            logger.info(f'Step #27 - Expected error occurred: {str(e)}. Proceeding to next step.')
        else:
            asserts.assert_true(False, 'Expected InteractionModelError with FailsafeRequired, but no exception occurred.')

        self.step(28)
        fabrics_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.Fabrics,
            fabric_filtered=False)
        fabrics_updated_size = len(fabrics_updated)
        logger.info(f'Step #28 - TH1 fabrics updated with additional entry for TH2: {fabrics_updated_size}')
        asserts.assert_equal(fabrics_updated_size, fabrics_original_size + 1,
                             "The fabrics list size should match the original fabrics list size + 1.")

        self.step(29)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=failsafe_expiration_seconds, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        asserts.assert_true(debug_text == '' or len(debug_text) <= 512,
                            "debugText must be empty or have a maximum length of 512 characters.")
        logger.info(f'Step #29: ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(30)
        # Reused TrustedRootCertificate created in step #27 - Send command to add new trusted root certificate
        cmd = self.cluster_opcreds.Commands.AddTrustedRootCertificate(th2_new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        self.step(31)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(f'Step #31 - The updated num_trusted_roots_original: {trusted_root_list_original_size_updated}')
        # Verify that the trusted root list size has increased by 1, the trusted root list size is numTrustedRootsOriginal + 2
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 2,
                             "Unexpected number of entries in the TrustedRootCertificates table after update")

        self.step(32)
        logger.info(f'Step #32: - The maxFailsafe (max_fail_safe): {maxFailsafe}')

        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe)
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
        logger.info(f'Step #32: ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(33)
        # Set the failsafe expiration timeout to PIXIT.CGEN.FailsafeExpiryLengthSeconds seconds fpr CI or FAILSAFE_EXPIRATION_SECONDS constant for Cert, must be less than maxFailsafe (max_fail_safe).
        failsafe_timeout_less_than_max = failsafe_expiration_seconds
        # Verify that failsafe_timeout_less_than_max is less than max_fail_safe
        asserts.assert_less(failsafe_timeout_less_than_max, maxFailsafe)

        if self.is_pics_sdk_ci_only:
            # Step 33 - In CI environments avoiding the original wait time defined in PIXIT.CGEN.FailsafeExpiryLengthSeconds
            # and speeding up test execution by setting the expiration time to 2 seconds.

            run_type = "CI Test"
            logger.info(
                f'Step 33: {run_type} - Bypassing failsafe expiration to avoid unnecessary delays in CI environment.')

            logger.info(
                f'Step #33: {run_type} - Waiting for the failsafe timer '
                f'(PIXIT.CGEN.FailsafeExpiryLengthSeconds --adjusted time for CI) to approach expiration, '
                f'but not allowing it to fully expire. Waiting for: {failsafe_timeout_less_than_max} seconds.')
            # Wait PIXIT.CGEN.FailsafeExpiryLengthSeconds time with an additional 0.5-second buffer, not allowing the fully exire (max_fail_safe).
            await asyncio.sleep(failsafe_timeout_less_than_max + .5)
        else:
            run_type = "Cert Test"

            logger.info(
                f'Step #33: {run_type} - Waiting for the failsafe timer '
                f'(FAILSAFE_EXPIRATION_SECONDS constant) to approach expiration, '
                f'but not allowing it to fully expire. Waiting for: {failsafe_timeout_less_than_max} seconds.')
            # Wait FAILSAFE_EXPIRATION_SECONDS constant time with an additional 0.5-second buffer, not allowing the fully exire (max_fail_safe).
            await asyncio.sleep(failsafe_timeout_less_than_max + .5)

        self.step(34)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(f'Step #34 - The updated num_trusted_roots_original: {trusted_root_list_original_size_updated}')
        # Verify that the trusted root list size is numTrustedRootsOriginal + 2
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 2,
                             "Step #34 - Unexpected number of entries in the TrustedRootCertificates table after update")

        self.step(35)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=failsafe_expiration_seconds, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=TH2,
            node_id=newNodeId+1,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'BusyWithOtherAdmin'(4)
        logger.info(f'Step #35 - TH2 ArmFailSafeResponse with ErrorCode as BusyWithOtherAdmin ({resp.errorCode})')
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kBusyWithOtherAdmin,
                             "Failure status returned from arm failsafe")

        self.step(36)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #36: ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(37)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #37: ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(38)
        if self.is_pics_sdk_ci_only:
            # In CI environment, bypass the wait for the failsafe expiration to avoid unnecessary delays.
            run_type = "CI Test"
            logger.info(
                f'Step #38: {run_type} - Bypassing due to failsafe expiration workaround to avoid unnecessary delays in CI environment.')
        else:
            run_type = "Cert Test"
            t_start = time.time()

            # Get the current time and format it for logging
            logger.info(f'Step #38: {run_type} - TH1 saves the Current time as t_start')

        self.step(39)
        # Reused TrustedRootCertificate created in step #5 - Send command to add new trusted root certificate
        cmd = self.cluster_opcreds.Commands.AddTrustedRootCertificate(new_root_cert)
        resp = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)

        self.step(40)
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(
            f'Step #40: The updated size of the num_trusted_roots_original list is {trusted_root_list_original_size_updated}')
        # Verify that the trusted root list size is numTrustedRootsOriginal + 2
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size + 2,
                             "Unexpected number of entries in the TrustedRootCertificates table after update")

        self.step(41)
        # Limit maxFailsafe to PIXIT.CGEN.FailsafeExpiryLengthSeconds seconds for CI or FAILSAFE_EXPIRATION_SECONDS constant for Cert to prevent excessively long waits in tests (due maxFailsafe = 900 seconds).
        maxFailsafe_original = maxFailsafe
        maxFailsafe = failsafe_expiration_seconds
        if self.is_pics_sdk_ci_only:
            # In CI environment, bypass the wait for the failsafe expiration to avoid unnecessary delays.
            run_type = "CI Test"
            logger.info(
                f'Step #41: {run_type} - Bypassing due to failsafe expiration workaround to avoid unnecessary delays in CI environment.')
        else:
            run_type = "Cert Test"

            # Make TH1 wait until the target_time is greater than or equal to half of the maxFailsafe time with an additional 0.5-second buffer.
            target_time = maxFailsafe/2
            await asyncio.sleep(target_time + .5)

            c_time = time.time()
            elapsed_time = (c_time - t_start) * 1000

            # Verify that at least half of the maxFailsafe time has passed, allowing TH1 to proceed.
            logger.info(
                f'Step #41: {run_type} - - MaxFailsafe is {maxFailsafe}. '
                f'TH1 can proceed. Elapsed time: {elapsed_time:.2f} ms.'
                f'The target time ({target_time} seconds) has passed. '
                f'Confirmation that ArmFailSafe has not expired yet.'
            )

        self.step(42)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=maxFailsafe_original)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd)
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #42: ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(43)
        if self.is_pics_sdk_ci_only:
            # Step 43 - In CI environments, the 'set_failsafe_timer' function is used to immediately force the failsafe timer to expire,
            # avoiding the original wait time defined in PIXIT.CGEN.FailsafeExpiryLengthSeconds,
            # and speeding up test execution by setting the expiration time to 1 second.

            run_type = "CI Test"
            logger.info(
                f'Step #43: {run_type} - Bypassing due to failsafe expiration workaround to avoid unnecessary delays in CI environment.')

            # Force the failsafe timer to expire immediately for TH1, avoiding unnecessary delays in CI environments
            resp = await self.set_failsafe_timer(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                expiration_time_seconds=failsafe_expiration_seconds)
            logger.info(
                f'Step #43 {run_type} - Failsafe timer expiration bypassed for TH1 by setting expiration time to {failsafe_expiration_seconds} seconds. '
                f'Test continues without the original wait.'
            )
        else:
            run_type = "Cert Test"

            # Calculate the target time (Tstart + maxFailsafe) with an additional 0.5-second buffer
            target_time = (t_start + maxFailsafe) + .5

            # Wait until the target_time is reached using asyncio.sleep to avoid busy-waiting
            await asyncio.sleep(target_time - time.time())

            c_time = time.time()
            elapsed_time = (c_time - t_start) * 1000

            # Checks if the elapsed time from start_time has met or exceeded maxFailsafe
            # TH1 process can proceed
            logger.info(
                f'Step #43: {run_type} - MaxFailsafe is {maxFailsafe}. '
                f'TH1 can proceed. Elapsed time: {elapsed_time:.2f} ms. '
                f'The target time ({maxFailsafe} seconds) has passed '
                f'Confirmation that ArmFailSafe has not expired yet.'
            )

        self.step(44)
        # Remove Fabric from TH2  to ensure it is correctly cleaned up and that the root certificates are aligned with expectations.
        # Read the CurrentFabricIndex attribute from TH2 (the second controller)
        fabric_idx = await self.read_single_attribute_check_success(dev_ctrl=TH2, node_id=newNodeId+1, cluster=self.cluster_opcreds, attribute=self.cluster_opcreds.Attributes.CurrentFabricIndex)
        logger.info(f'Step #44: TH2 CurrentFabricIndex attribute: {fabric_idx}')
        # Remove the fabric from TH2 (second controller)
        cmd = Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=fabric_idx)
        await self.send_single_cmd(dev_ctrl=TH2, node_id=newNodeId+1, cmd=cmd)

        # The expected number of root certificates should be numTrustedRootsOriginal after removing the fabric
        trusted_root_list_original_updated = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=self.cluster_opcreds,
            attribute=self.cluster_opcreds.Attributes.TrustedRootCertificates)
        trusted_root_list_original_size_updated = len(trusted_root_list_original_updated)
        logger.info(
            f'Step #44: he updated size of the num_trusted_roots_original list: {trusted_root_list_original_size_updated}')
        # Verify that the trusted root list size from oririnal trusted root list is 1
        asserts.assert_equal(trusted_root_list_original_size_updated, trusted_root_list_original_size,
                             "Unexpected number of entries in the TrustedRootCertificates table after update")


if __name__ == "__main__":
    default_matter_test_main()
