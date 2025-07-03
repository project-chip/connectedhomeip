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

import logging
import os
import tempfile
from time import sleep

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.fault_injection import CHIPFaultId
from chip.interaction_model import InteractionModelError
from chip.testing.apps import AppServerSubprocess
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_SC_3_5(MatterBaseTest):
    def setup_class(self):
        super().setup_class()

        self.th_server = None
        self.storage = None

        self.th_client = self.default_controller
        self.th_server_local_nodeid = 1111
        self.th_server_discriminator = 1234
        self.th_server_passcode = 20202021

        self.th_server_app = self.user_params.get("th_server_app_path", None)
        if not self.th_server_app:
            asserts.fail("This test requires a TH_SERVER app. Specify app path with --string-arg th_server_app_path:<path_to_app>")
        if not os.path.exists(self.th_server_app):
            asserts.fail(f"The path {self.th_server_app} does not exist")

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
        pics = [
            "MCORE.ROLE.COMMISSIONER",
        ]
        return pics

    def steps_TC_SC_3_5(self) -> list[TestStep]:
        steps = [

            TestStep("precondition", "TH_SERVER has been commissioned to TH_CLIENT", is_commissioning=True),

            TestStep("1a", "TH Client sends an OpenCommissioningWindow command to TH_SERVER to allow it to be commissioned by DUT_Initiator and trigger CASE Handshake",
                     "Verify that the TH_SERVER returns SUCCESS"),

            TestStep("1b", "TH Client sends FailAtFault command to FaultInjection cluster on TH_SERVER to include a corrupt TBEData2Encrypted in the Sigma2 it will send during CASE Handshake",
                     "Verify that the TH_SERVER receives the message"),

            TestStep("1c", "TH prompts the user to Commission DUT_Initiator to TH_SERVER",
                     "Verify that the DUT sends a status report to TH_SERVER with a FAILURE general code (value 1), protocol ID of SECURE_CHANNEL (0x0000), and Protocol code of INVALID_PARAMETER (0X0002)."),

            TestStep("2a", "TH Client revokes the Commissioning Window and resends an OpenCommissioningWindow command to TH_SERVER to allow commissioning by DUT_Initiator again and re-trigger the CASE handshake.",
                     "Verify that the TH_SERVER returns SUCCESS"),

            TestStep("2b", "TH Client sends FailAtFault command to FaultInjection cluster on TH_SERVER to include a corrupt responderNOC in the Sigma2 it will send during CASE Handshake",
                     "Verify that the TH_SERVER receives the message"),

            TestStep("2c", "TH prompts the user to Commission DUT_Initiator to TH_SERVER again",
                     "Verify that the DUT sends a status report to TH_SERVER with a FAILURE general code (value 1), protocol ID of SECURE_CHANNEL (0x0000), and Protocol code of INVALID_PARAMETER (0X0002)."),

            TestStep("3a", "TH Client revokes the Commissioning Window and resends an OpenCommissioningWindow command to TH_SERVER to allow commissioning by DUT_Initiator again and re-trigger the CASE handshake.",
                     "Verify that the TH_SERVER returns SUCCESS"),

            TestStep("3b", "TH Client sends FailAtFault command to FaultInjection cluster on TH_SERVER to include a corrupt responderICAC in the Sigma2 it will send during CASE Handshake",
                     "Verify that the TH_SERVER receives the message"),


            TestStep("3c", "TH prompts the user to Commission DUT_Initiator to TH_SERVER again",
                     "Verify that the DUT sends a status report to TH_SERVER with a FAILURE general code (value 1), protocol ID of SECURE_CHANNEL (0x0000), and Protocol code of INVALID_PARAMETER (0X0002)."),


            TestStep("4a", "TH Client revokes the Commissioning Window and resends an OpenCommissioningWindow command to TH_SERVER to allow commissioning by DUT_Initiator again and re-trigger the CASE handshake.",
                     "Verify that the TH_SERVER returns SUCCESS"),

            TestStep("4b", "TH Client sends FailAtFault command to FaultInjection cluster on TH_SERVER to include a corrupt Signature in the Sigma2 it will send during CASE Handshake",
                     "Verify that the TH_SERVER receives the message"),


            TestStep("4c", "TH prompts the user to Commission DUT_Initiator to TH_SERVER again",
                     "Verify that the DUT sends a status report to TH_SERVER with a FAILURE general code (value 1), protocol ID of SECURE_CHANNEL (0x0000), and Protocol code of INVALID_PARAMETER (0X0002)."),



        ]
        return steps

    def start_th_server(self):

        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()

        # Create a temporary storage directory for keeping KVS files.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        logging.info("Temporary storage directory: %s", self.storage.name)

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
            nodeid=self.th_server_local_nodeid, timeout=3*60, iteration=10000, discriminator=self.th_server_discriminator, option=1)
        new_random_passcode = params.setupPinCode
        sleep(1)
        logging.info("OpenCommissioningWindow complete")

        return new_random_passcode

    async def reopen_commissioning_window(self):
        ''' Before reopening Commissioning Window, we need to instruct TH_Server to revoke any active OpenCommissioningWindows '''

        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th_client.SendCommand(nodeid=self.th_server_local_nodeid, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=9000)
        sleep(1)

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
                nodeid=self.th_server_local_nodeid,
                endpoint=0,  # Fault‑Injection cluster lives on EP0
                payload=command,
            )
        except InteractionModelError:
            asserts.fail("Fault Injection Command Failed, is the TH_SERVER app built with the FaultInjection Cluster?")

    @async_test_body
    async def test_TC_SC_3_5(self):

        self.step("precondition")
        await self.th_client.CommissionOnNetwork(nodeId=self.th_server_local_nodeid, setupPinCode=self.th_server_passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.th_server_discriminator)
        logging.info("Commissioning TH_SERVER complete")

        self.step("1a")
        th_server_passcode = await self.open_commissioning_window()

        self.step("1b")
        await self.send_fault_injection_command(CHIPFaultId.CASECorruptTBEData2Encrypted)

        self.step("1c")
        prompt_msg = (
            "\nPlease commission the TH server app from DUT:\n"
            f"  pairing onnetwork 1 {th_server_passcode}\n"
            "Input 'Y' if commissioner DUT fails commissioning AND TH Server Logs display CHIP ERROR = 0x00000054 (INVALID CASE PARAMETER) equivalent to Status Report with protocol code 2 \n"
            "Input 'N' if commissioner DUT commissions successfully \n "
            "Or failure in TH Server Logs IS NOT = 'INVALID CASE PARAMETER'\n"
        )

        if self.is_pics_sdk_ci_only:
            resp = 'Y'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        expected_error_found = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            expected_error_found,
            True,
            f"Expected Error in TH_SERVER logs is {'found' if expected_error_found else 'not found'}"
        )

        '''------------------------------------------- Inject Fault into Sigma2 responderNOC--------------------------------------------- '''

        self.step("2a")
        th_server_passcode = await self.reopen_commissioning_window()

        self.step("2b")
        await self.send_fault_injection_command(CHIPFaultId.CASECorruptSigma2NOC)

        self.step("2c")
        prompt_msg = (
            "\nPlease commission the TH server app from DUT:\n"
            "\nWARNING: Make sure that the Commissioner restarts commissioning from scratch, such as by changing NodeID or by restarting the commissioner\n"
            f"  pairing onnetwork 2 {th_server_passcode}\n"
            "Input 'Y' if commissioner DUT fails commissioning AND TH Server Logs display CHIP ERROR = 0x00000054 (INVALID CASE PARAMETER) equivalent to Status Report with protocol code 2 \n"
            "Input 'N' if commissioner DUT commissions successfully \n "
            "Or failure in TH Server Logs IS NOT = 'INVALID CASE PARAMETER'\n"
        )

        if self.is_pics_sdk_ci_only:
            resp = 'Y'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        expected_error_found = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            expected_error_found,
            True,
            f"Expected Error in TH_SERVER logs is {'found' if expected_error_found else 'not found'}"
        )

        '''------------------------------------------- Inject Fault into Sigma2 responderICAC--------------------------------------------- '''

        self.step("3a")
        th_server_passcode = await self.reopen_commissioning_window()

        self.step("3b")
        await self.send_fault_injection_command(CHIPFaultId.CASECorruptSigma2ICAC)

        self.step("3c")
        prompt_msg = (
            "\nPlease commission the TH server app from DUT:\n"
            "\nWARNING: Make sure that the Commissioner restarts commissioning from scratch, such as by changing NodeID or by restarting the commissioner\n"
            f"  pairing onnetwork 3 {th_server_passcode}\n"
            "Input 'Y' if commissioner DUT fails commissioning AND TH Server Logs display CHIP ERROR = 0x00000054 (INVALID CASE PARAMETER) equivalent to Status Report with protocol code 2 \n"
            "Input 'N' if commissioner DUT commissions successfully \n "
            "Or failure in TH Server Logs IS NOT = 'INVALID CASE PARAMETER'\n"
        )

        if self.is_pics_sdk_ci_only:
            resp = 'Y'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        expected_error_found = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            expected_error_found,
            True,
            f"Expected Error in TH_SERVER logs is {'found' if expected_error_found else 'not found'}"
        )

        '''------------------------------------------- Inject Fault into Sigma2 Signature--------------------------------------------- '''

        self.step("4a")
        th_server_passcode = await self.reopen_commissioning_window()

        self.step("4b")
        await self.send_fault_injection_command(CHIPFaultId.CASECorruptSigma2Signature)

        self.step("4c")
        prompt_msg = (
            "\nPlease commission the TH server app from DUT:\n"
            "\nWARNING: Make sure that the Commissioner restarts commissioning from scratch, such as by changing NodeID or by restarting the commissioner\n"
            f"  pairing onnetwork 4 {th_server_passcode}\n"
            "Input 'Y' if commissioner DUT fails commissioning AND TH Server Logs display CHIP ERROR = 0x00000054 (INVALID CASE PARAMETER) equivalent to Status Report with protocol code 2 \n"
            "Input 'N' if commissioner DUT commissions successfully \n "
            "Or failure in TH Server Logs IS NOT = 'INVALID CASE PARAMETER'\n"
        )

        print(f"self.is_pics_sdk_ci_only = {self.is_pics_sdk_ci_only}")
        if self.is_pics_sdk_ci_only:
            resp = 'Y'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        expected_error_found = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            expected_error_found,
            True,
            f"Expected Error in TH_SERVER logs is {'found' if expected_error_found else 'not found'}"
        )


if __name__ == "__main__":
    default_matter_test_main()
