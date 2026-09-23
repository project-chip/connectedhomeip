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
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     factory-reset: true
#     quiet: true
#     script-args: >
#       --string-arg th_server_app_path:${ALL_CLUSTERS_APP}
#       --storage-path admin_storage.json
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import os
import tempfile

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.exceptions import ChipStackError
from matter.fault_injection import CHIPFaultId
from matter.interaction_model import InteractionModelError
from matter.testing.apps import AppServerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_SC_3_5(MatterBaseTest):

    disable_wildcard_subscription = True

    def setup_class(self):
        super().setup_class()

        self.th_server = None
        self.storage = None

        # We assume that DUT_Commissioner has ICAC in its NOC Chain unless determined otherwise in step 1c.
        # When running in CI, we also want this to be True to avoid skipping steps.
        self.dut_has_icac = True

        self.th_client = self.default_controller
        self.th_server_local_nodeid = 1111
        self.th_server_discriminator = 1234
        self.th_server_passcode = 20202021

        self.th_server_app = self.user_params.get("th_server_app_path", None)
        if not self.th_server_app:
            asserts.fail("This test requires a TH_SERVER app. Specify app path with --string-arg th_server_app_path:<path_to_app>")
        if not os.path.exists(self.th_server_app):
            asserts.fail(f"The path {self.th_server_app} does not exist")

        self.commissioning_timeout = int(self.user_params.get("commissioning_timeout", 90))

        # Start TH Server
        self.start_th_server()

    def teardown_class(self):

        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    def desc_TC_SC_3_5(self) -> str:
        return "[TC-SC-3.5] CASE Error Handling [DUT_Initiator] "

    def pics_TC_SC_3_5(self) -> list[str]:
        return [
            "MCORE.ROLE.COMMISSIONER",
        ]

    @property
    def default_timeout(self) -> int:
        # Test requires manual control of a commissioner, so we set a long enough timeout
        # so that testers can input commands/control the commissioner without worrying about a timeout.
        # Note that individual user prompt steps still time out after 60s, but this is now more flexible
        # because testers merely need to start the commissioning within that timeframe
        return 25*60

    def steps_TC_SC_3_5(self) -> list[TestStep]:
        return [

            TestStep("precondition", "TH_SERVER has been commissioned to TH_CLIENT"),

            TestStep("1a", "TH Client sends an OpenCommissioningWindow command to TH_SERVER to allow it to be commissioned by DUT_Commissioner to determine if the DUT_Commissioner has an ICAC in its NOC Chain",
                     "Verify that the TH_SERVER returns SUCCESS"),

            TestStep("1b", "TH prompts the user to Commission DUT_Commissioner to TH_SERVER",
                     "Verify that the DUT commissioned TH_SERVER successfully"),

            TestStep("1c", "TH Client Reads the NOCs attribute on TH_SERVER and checks if DUT_Commissioner has ICAC in its NOC Chain",
                     "Verify that NOCs attribute returns two NOCStructs and determine if DUT_Commissioner has ICAC"),

            TestStep("1d", "TH Client removes the DUT_Commissioner's fabric from TH_SERVER",
                     "Verify that the DUT_Commissioner's fabric is removed from TH_SERVER"),

            TestStep("2a", "TH Client sends an OpenCommissioningWindow command to TH_SERVER to allow it to be commissioned by DUT_Commissioner and trigger CASE Handshake",
                     "Verify that the TH_SERVER returns SUCCESS"),

            TestStep("2b", "TH Client sends FailAtFault command to FaultInjection cluster on TH_SERVER to include a corrupt TBEData2Encrypted in the Sigma2 it will send during CASE Handshake",
                     "Verify that the TH_SERVER receives the message"),

            TestStep("2c", "TH prompts the user to Commission DUT_Commissioner to TH_SERVER",
                     "Verify that the DUT sends a status report to TH_SERVER with a FAILURE general code (value 1), protocol ID of SECURE_CHANNEL (0x0000), and Protocol code of INVALID_PARAMETER (0X0002). Verify that the commissioning failed by checking that the commissioning window is still open on TH_SERVER."),

            TestStep("3a", "TH Client revokes the Commissioning Window and resends an OpenCommissioningWindow command to TH_SERVER to allow commissioning by DUT_Commissioner again and re-trigger the CASE handshake.",
                     "Verify that the TH_SERVER returns SUCCESS"),

            TestStep("3b", "TH Client sends FailAtFault command to FaultInjection cluster on TH_SERVER to include a corrupt responderNOC in the Sigma2 it will send during CASE Handshake",
                     "Verify that the TH_SERVER receives the message"),

            TestStep("3c", "TH prompts the user to Commission DUT_Commissioner to TH_SERVER again",
                     "Verify that the DUT sends a status report to TH_SERVER with a FAILURE general code (value 1), protocol ID of SECURE_CHANNEL (0x0000), and Protocol code of INVALID_PARAMETER (0X0002). Verify that the commissioning failed by checking that the commissioning window is still open on TH_SERVER."),

            TestStep("4a", "TH Client revokes the Commissioning Window and resends an OpenCommissioningWindow command to TH_SERVER to allow commissioning by DUT_Commissioner again and re-trigger the CASE handshake."
                     " This Test Step is skipped if DUT_Commissioner does not have ICAC in its NOC Chain",
                     "Verify that the TH_SERVER returns SUCCESS"),

            TestStep("4b", "TH Client sends FailAtFault command to FaultInjection cluster on TH_SERVER to include a corrupt responderICAC in the Sigma2 it will send during CASE Handshake."
                     " This Test Step is skipped if DUT_Commissioner does not have ICAC in its NOC Chain",
                     "Verify that the TH_SERVER receives the message"),


            TestStep("4c", "TH prompts the user to Commission DUT_Commissioner to TH_SERVER again."
                     " This Test Step is skipped if DUT_Commissioner does not have ICAC in its NOC Chain",
                     "Verify that the DUT sends a status report to TH_SERVER with a FAILURE general code (value 1), protocol ID of SECURE_CHANNEL (0x0000), and Protocol code of INVALID_PARAMETER (0X0002). Verify that the commissioning failed by checking that the commissioning window is still open on TH_SERVER."),


            TestStep("5a", "TH Client revokes the Commissioning Window and resends an OpenCommissioningWindow command to TH_SERVER to allow commissioning by DUT_Commissioner again and re-trigger the CASE handshake.",
                     "Verify that the TH_SERVER returns SUCCESS"),

            TestStep("5b", "TH Client sends FailAtFault command to FaultInjection cluster on TH_SERVER to include a corrupt Signature in the Sigma2 it will send during CASE Handshake",
                     "Verify that the TH_SERVER receives the message"),


            TestStep("5c", "TH prompts the user to Commission DUT_Commissioner to TH_SERVER again",
                     "Verify that the DUT sends a status report to TH_SERVER with a FAILURE general code (value 1), protocol ID of SECURE_CHANNEL (0x0000), and Protocol code of INVALID_PARAMETER (0X0002). Verify that the commissioning failed by checking that the commissioning window is still open on TH_SERVER."),



        ]

    def start_th_server(self):

        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()

        # Create a temporary storage directory for keeping KVS files.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        log.info("Temporary storage directory: %s", self.storage.name)

        self.th_server = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage.name,
            discriminator=self.th_server_discriminator,
            passcode=self.th_server_passcode
        )

        self.th_server.start(
            expected_output="Server initialization complete",
            timeout=60)

    async def open_commissioning_window(self):

        # Instructing TH Server to accept a new Commissioner, which is the DUT
        params = await self.th_client.OpenCommissioningWindow(
            nodeId=self.th_server_local_nodeid, timeout=3*60, iteration=10000, discriminator=self.th_server_discriminator, option=1)
        await asyncio.sleep(1)
        log.info("OpenCommissioningWindow complete")

        return params.setupManualCode, params.setupPinCode

    async def revoke_and_open_commissioning_window(self):
        ''' Before reopening Commissioning Window, we need to instruct TH_SERVER to revoke any active OpenCommissioningWindows.'''

        try:
            revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
            await self.th_client.SendCommand(nodeId=self.th_server_local_nodeid, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=9000)
        except InteractionModelError as e:
            # If the window is already closed, we just go ahead with opening the commissioning window
            if e.status == Clusters.AdministratorCommissioning.Enums.StatusCode.kWindowNotOpen:
                pass
            else:
                log.exception('Error running RevokeCommissioning command: %s', e)
                asserts.fail(f"RevokeCommissioning failed with error: {e.status}")
        except Exception as e:
            log.exception('Error running RevokeCommissioning command: %s', e)
            asserts.fail(f"RevokeCommissioning failed with error: {str(e)}")

        await asyncio.sleep(1)

        return await self.open_commissioning_window()

    async def send_fault_injection_command(self, faultID: CHIPFaultId):
        '''Inject Fault in TH_SERVER, by sending fault injection Cluster Command from TH_CLIENT'''

        command = Clusters.FaultInjection.Commands.FailAtFault(
            type=Clusters.FaultInjection.Enums.FaultType.kChipFault,
            id=faultID,
            numCallsToFail=1,
            takeMutex=False,
        )

        try:
            await self.th_client.SendCommand(
                nodeId=self.th_server_local_nodeid,
                endpoint=0,  # Fault‑Injection cluster lives on EP0
                payload=command,
            )
        except InteractionModelError:
            asserts.fail("Fault Injection Command Failed, is the TH_SERVER app built with the FaultInjection Cluster?")

    async def assert_dut_commissioner_failed_to_complete_commissioning(self):
        '''Helper to assert that DUT_Commissioner failed to commission TH_SERVER; We do this by checking that the commissioning window is still open.
           NOTE: This method should only be called when testing CASE Error failure AND after TH_Client has opened commissioning window for DUT_Commissioner.
        '''

        AC_cluster = Clusters.AdministratorCommissioning
        window_status = await self.read_single_attribute_check_success(
            dev_ctrl=self.th_client,
            node_id=self.th_server_local_nodeid,
            fabric_filtered=False,
            endpoint=0,
            cluster=AC_cluster,
            attribute=AC_cluster.Attributes.WindowStatus
        )

        asserts.assert_not_equal(window_status, AC_cluster.Enums.CommissioningWindowStatusEnum.kWindowNotOpen,
                                 "Commissioning window is expected to be open, but was found to be closed. This indicates that DUT_Commissioner completed commissioning successfully, which is not expected in this test step")

    def create_second_controller(self) -> ChipDeviceCtrl.ChipDeviceController:
        new_CA = self.certificate_authority_manager.NewCertificateAuthority()

        new_fabric_admin = new_CA.NewFabricAdmin(vendorId=0xFFF1,
                                                 fabricId=self.matter_test_config.fabric_id)

        return new_fabric_admin.NewController(nodeId=112233)

    @async_test_body
    async def test_TC_SC_3_5(self):

        self.step("precondition")
        await self.th_client.CommissionOnNetwork(nodeId=self.th_server_local_nodeid, setupPinCode=self.th_server_passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.th_server_discriminator)
        log.info("Commissioning TH_SERVER complete")

        # ------------------------------------------- Determine if DUT Commissioner has ICAC in its NOC Chain---------------------------------------------

        self.step("1a")
        th_server_manual_code, th_server_passcode = await self.open_commissioning_window()

        self.step("1b")
        prompt_msg = (
            "\nPlease commission the TH_SERVER app from DUT using the Manual Pairing Code below:\n"
            f"  Manual Pairing Code: {th_server_manual_code}  (chip-tool: pairing onnetwork 1 {th_server_passcode})\n"
            "Input anything once commissioning has started \n"
        )

        self.th_server.set_output_match("Commissioning completed successfully")
        self.th_server.event.clear()

        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(prompt_msg)
        else:
            th2 = self.create_second_controller()
            await th2.CommissionOnNetwork(nodeId=1, setupPinCode=th_server_passcode)

        if not self.th_server.event.wait(self.commissioning_timeout):
            asserts.fail("DUT_Commissioner failed to commission TH_SERVER successfully within 90s.")

        self.step("1c")
        nocStructs = await self.read_single_attribute_check_success(dev_ctrl=self.th_client,
                                                                    node_id=self.th_server_local_nodeid,
                                                                    cluster=Clusters.OperationalCredentials,
                                                                    attribute=Clusters.OperationalCredentials.Attributes.NOCs,
                                                                    fabric_filtered=False)

        asserts.assert_equal(len(nocStructs), 2,
                             f"Expected 2 NOCStructs (1 for TH Client, 1 for DUT Commissioner), got {len(nocStructs)}")

        dut_commissioner_noc_struct = next((noc for noc in nocStructs if noc.fabricIndex != self.th_client.fabricId), None)

        asserts.assert_is_not_none(
            dut_commissioner_noc_struct,
            "Could not find a NOCStruct for DUT Commissioner. Please ensure that DUT Commissioner commissioned TH Server successfully."
        )

        dut_commissioner_icac = dut_commissioner_noc_struct.icac
        dut_commissioner_fabric_index = dut_commissioner_noc_struct.fabricIndex

        # Determine if DUT_Commissioner has ICAC in its NOC Chain
        if dut_commissioner_icac == NullValue:
            log.info("DUT_Commissioner does not have ICAC in its NOC Chain")
            self.dut_has_icac = False
        else:
            log.info("DUT_Commissioner has ICAC in its NOC Chain")
            self.dut_has_icac = True

        # Remove DUT_Commissioner's fabric from TH_SERVER to prepare for commissioning DUT_Commissioner in next step
        self.step("1d")

        cmd = Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=dut_commissioner_fabric_index)
        resp = await self.send_single_cmd(dev_ctrl=self.th_client, node_id=self.th_server_local_nodeid, cmd=cmd)
        asserts.assert_equal(
            resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk)

        # ------------------------------------------- Inject Fault into Sigma2 TBEData2Encrypted---------------------------------------------
        self.step("2a")
        th_server_manual_code, th_server_passcode = await self.open_commissioning_window()

        self.step("2b")
        await self.send_fault_injection_command(CHIPFaultId.CASECorruptTBEData2Encrypted)

        self.step("2c")
        prompt_msg = (
            "\nPlease commission the TH_SERVER app from DUT using the Manual Pairing Code below:\n"
            f"  Manual Pairing Code: {th_server_manual_code}  (chip-tool: pairing onnetwork 1 {th_server_passcode})\n"
            "Input anything once commissioning has started \n"
        )

        self.th_server.set_output_match("Invalid CASE parameter")
        self.th_server.event.clear()

        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(prompt_msg)
        else:
            th2 = self.create_second_controller()
            with asserts.assert_raises(ChipStackError):
                await th2.CommissionOnNetwork(nodeId=1, setupPinCode=th_server_passcode)

        if not self.th_server.event.wait(self.commissioning_timeout):
            asserts.fail("Commissioning failure was not detected within 90s.")

        await self.assert_dut_commissioner_failed_to_complete_commissioning()

        # ------------------------------------------- Inject Fault into Sigma2 responderNOC---------------------------------------------

        self.step("3a")
        th_server_manual_code, th_server_passcode = await self.revoke_and_open_commissioning_window()

        self.step("3b")
        await self.send_fault_injection_command(CHIPFaultId.CASECorruptSigma2NOC)

        self.step("3c")
        prompt_msg = (
            "\nPlease commission the TH_SERVER app from DUT using the Manual Pairing Code below:\n"
            "\nWARNING: Make sure that the Commissioner restarts commissioning from scratch, such as by changing NodeID or by restarting the commissioner\n"
            f"  Manual Pairing Code: {th_server_manual_code}  (chip-tool: pairing onnetwork 2 {th_server_passcode})\n"
            "Input anything once commissioning has started \n"
        )

        self.th_server.set_output_match("Invalid CASE parameter")
        self.th_server.event.clear()

        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(prompt_msg)
        else:
            th2 = self.create_second_controller()
            with asserts.assert_raises(ChipStackError):
                await th2.CommissionOnNetwork(nodeId=2, setupPinCode=th_server_passcode)

        if not self.th_server.event.wait(self.commissioning_timeout):
            asserts.fail("Commissioning failure was not detected within 90s.")

        await self.assert_dut_commissioner_failed_to_complete_commissioning()

        # ------------------------------------------- Inject Fault into Sigma2 responderICAC---------------------------------------------

        if not self.dut_has_icac:
            log.info("DUT_Commissioner does not have ICAC in its NOC Chain, skipping ICAC Fault Injection testcase")
            self.skip_step("4a")
            self.skip_step("4b")
            self.skip_step("4c")
        else:
            self.step("4a")
            th_server_manual_code, th_server_passcode = await self.revoke_and_open_commissioning_window()

            self.step("4b")
            await self.send_fault_injection_command(CHIPFaultId.CASECorruptSigma2ICAC)

            self.step("4c")
            prompt_msg = (
                "\nPlease commission the TH_SERVER app from DUT using the Manual Pairing Code below:\n"
                "\nWARNING: Make sure that the Commissioner restarts commissioning from scratch, such as by changing NodeID or by restarting the commissioner\n"
                f"  Manual Pairing Code: {th_server_manual_code}  (chip-tool: pairing onnetwork 3 {th_server_passcode})\n"
                "Input anything once commissioning has started \n"
            )

            self.th_server.set_output_match("Invalid CASE parameter")
            self.th_server.event.clear()

            if not self.is_pics_sdk_ci_only:
                self.wait_for_user_input(prompt_msg)
            else:
                th2 = self.create_second_controller()
                with asserts.assert_raises(ChipStackError):
                    await th2.CommissionOnNetwork(nodeId=3, setupPinCode=th_server_passcode)

            if not self.th_server.event.wait(self.commissioning_timeout):
                asserts.fail("Commissioning failure was not detected within 90s.")

            await self.assert_dut_commissioner_failed_to_complete_commissioning()

        # ------------------------------------------- Inject Fault into Sigma2 Signature---------------------------------------------

        self.step("5a")
        th_server_manual_code, th_server_passcode = await self.revoke_and_open_commissioning_window()

        self.step("5b")
        await self.send_fault_injection_command(CHIPFaultId.CASECorruptSigma2Signature)

        self.step("5c")
        prompt_msg = (
            "\nPlease commission the TH_SERVER app from DUT using the Manual Pairing Code below:\n"
            "\nWARNING: Make sure that the Commissioner restarts commissioning from scratch, such as by changing NodeID or by restarting the commissioner\n"
            f"  Manual Pairing Code: {th_server_manual_code}  (chip-tool: pairing onnetwork 4 {th_server_passcode})\n"
            "Input anything once commissioning has started \n"
        )

        self.th_server.set_output_match("Invalid CASE parameter")
        self.th_server.event.clear()

        if not self.is_pics_sdk_ci_only:
            self.wait_for_user_input(prompt_msg)
        else:
            th2 = self.create_second_controller()
            with asserts.assert_raises(ChipStackError):
                await th2.CommissionOnNetwork(nodeId=4, setupPinCode=th_server_passcode)

        if not self.th_server.event.wait(self.commissioning_timeout):
            asserts.fail("Commissioning failure was not detected within 90s.")

        await self.assert_dut_commissioner_failed_to_complete_commissioning()


if __name__ == "__main__":
    default_matter_test_main()
