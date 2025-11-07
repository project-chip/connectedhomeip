#
#  Copyright (c) 2025 Project CHIP Authors
#  All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_CONTROLLER_APP}
#     app-args: interactive server
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --string-arg th_server_app_path:${CAMERA_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import os
import random
import tempfile
from time import sleep

from mobly import asserts
from TC_WEBRTCRTestBase import WEBRTCRTestBase

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.apps import AppServerSubprocess
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main


class TC_WebRTCR_2_7(WEBRTCRTestBase):
    def setup_class(self):
        super().setup_class()

        self.th_server = None
        self.storage = None

        th_server_app = self.user_params.get("th_server_app_path", None)
        if not th_server_app:
            asserts.fail("This test requires a TH_SERVER app. Specify app path with --string-arg th_server_app_path:<path_to_app>")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")

        # Create a temporary storage directory for keeping KVS files.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        logging.info("Temporary storage directory: %s", self.storage.name)

        self.th_server_discriminator = 1234
        self.th_server_passcode = 20202021

        # Start TH_SERVER (Provider)
        self.th_server = AppServerSubprocess(
            th_server_app,
            storage_dir=self.storage.name,
            discriminator=self.th_server_discriminator,
            passcode=self.th_server_passcode,
            port=5684,  # Use unique port number to avoid port conflict
        )

        self.th_server.start(
            expected_output="Server initialization complete",
            timeout=30)

        sleep(1)

    def teardown_class(self):
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    def desc_TC_WebRTCR_2_7(self) -> str:
        """Returns a description of this test"""
        return "[TC-{picsCode}-2.7] Validate ICECandidates command with empty candidate list"

    def steps_TC_WebRTCR_2_7(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        steps = [
            TestStep(1, "Commission the {TH_Server} from TH"),
            TestStep(2, "Open the Commissioning Window of the {TH_Server}"),
            TestStep(3, "Commission the {TH_Server} from DUT"),
            TestStep(4, "Activate fault injection on TH_SERVER to empty candidate list of the ICECandidates command"),
            TestStep(5, "Trigger TH_SERVER to send ICECandidates command with empty candidate list to DUT"),
        ]
        return steps

    def pics_TC_WebRTCR_2_7(self) -> list[str]:
        """
        Return the list of PICS applicable to this test case.
        """
        pics = [
            "WEBRTCR.S",           # WebRTC Transport Requestor Server
            "WEBRTCR.S.C02.Rsp",   # ICECandidates command
        ]
        return pics

    # This test has some manual steps and one sleep for up to 30 seconds. Test typically
    # runs under 1 mins, so 3 minutes is more than enough.
    @property
    def default_timeout(self) -> int:
        return 3 * 60

    @async_test_body
    async def test_TC_WebRTCR_2_7(self):
        """
        Executes the test steps for the WebRTC ICECandidates with empty candidate list scenario.
        """

        discriminator = self.th_server_discriminator
        passcode = self.th_server_passcode
        self.th_server_local_nodeid = 1111
        self.discriminator = random.randint(0, 4095)

        self.step(1)
        await self.default_controller.CommissionOnNetwork(nodeId=self.th_server_local_nodeid, setupPinCode=passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)
        logging.info("Commissioning TH_SERVER complete")

        self.step(2)
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=self.th_server_local_nodeid, timeout=3*60, iteration=10000, discriminator=self.discriminator, option=1)
        passcode = params.setupPinCode
        sleep(1)

        self.step(3)
        # Prompt user with instructions
        prompt_msg = (
            f"\nPlease commission the server app from DUT: manual code='{
                params.setupManualCode}' QR code='{params.setupQRCode}' :\n"
            f"  pairing onnetwork 1 {passcode}\n"
            "Input 'Y' if DUT successfully commissions without any warnings\n"
            "Input 'N' if commissioner warns about commissioning the non-genuine device, "
            "Or Commissioning fails with device appropriate attestation error\n"
        )

        if self.is_pics_sdk_ci_only:
            await self.send_command(f"pairing onnetwork 1 {passcode}")
            resp = 'Y'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        commissioning_success = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            commissioning_success,
            True,
            f"Commissioning {'succeeded' if commissioning_success else 'failed'}"
        )

        self.step(4)
        logging.info("Injecting kFault_EmptyWebRTCICECandidatesList on TH_SERVER")

        # --- Fault‑Injection cluster (mfg‑specific 0xFFF1_FC06) ---
        # Use FailAtFault to activate the chip‑layer fault exactly once
        #
        #  • faultType = kChipFault (0x03)  – always used for CHIP faults
        #  • id        = FaultInjection.Id.kFault_EmptyWebRTCICECandidatesList
        #  • numCallsToSkip = 0  – trigger on the very next call
        #  • numCallsToFail = 1  – inject once, then auto‑clear
        #  • takeMutex      = False  – single‑threaded app, no lock needed
        #
        command = Clusters.FaultInjection.Commands.FailAtFault(
            type=Clusters.FaultInjection.Enums.FaultType.kChipFault,
            id=33,  # kFault_EmptyWebRTCICECandidatesList
            numCallsToFail=1,
            takeMutex=False,
        )
        await self.default_controller.SendCommand(
            nodeid=self.th_server_local_nodeid,
            endpoint=0,  # Fault‑Injection cluster lives on EP0
            payload=command,
        )
        sleep(1)

        self.step(5)
        # Prompt user with instructions
        prompt_msg = (
            "\nSend 'ProvideOffer' command to the server app from DUT:\n"
            "  webrtc establish-session 1\n"
            "This should be done by establishing a WebRTC session and then triggering ICE candidate exchange.\n"
            "Input 'Y' if DUT responds with appropriate error status code (specification requires minimum 1 candidate)\n"
            "Input 'N' if DUT accepts the ICECandidates command with empty candidate list or responds with success\n"
        )

        if self.is_pics_sdk_ci_only:
            self.th_server.set_output_match("CONSTRAINT_ERROR")
            self.th_server.event.clear()

            try:
                await self.send_command("webrtc establish-session 1")
                # Wait up to 90s until the provider logs that the data‑channel opened
                if not self.th_server.event.wait(90):
                    raise TimeoutError("PeerConnection is not connected within 90s")
                resp = 'Y'
            except TimeoutError:
                resp = 'N'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        result = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            result,
            True,
            f"DUT {'correctly responded with error for empty candidate list' if result else 'did not respond with error as expected for empty candidate list'}"
        )


if __name__ == "__main__":
    default_matter_test_main()
