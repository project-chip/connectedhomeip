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

from matter import ChipDeviceCtrl
from matter.testing.apps import AppServerSubprocess
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_WebRTCR_2_5(MatterBaseTest):
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
            port=5682,  # Use unique port number to avoid port conflict
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

    def desc_TC_WebRTCR_2_5(self) -> str:
        """Returns a description of this test"""
        return "[TC-{picsCode}-2.5] Validate CurrentSessions attribute read"

    def steps_TC_WebRTCR_2_5(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        steps = [
            TestStep(1, "Commission the {TH_Server} from TH"),
            TestStep(2, "Open the Commissioning Window of the {TH_Server}"),
            TestStep(3, "Commission the {TH_Server} from DUT"),
            TestStep(4, "Read CurrentSessions attribute from DUT"),
            TestStep(5, "Establish a WebRTC session between {TH_Server} and DUT"),
            TestStep(6, "Read CurrentSessions attribute from DUT"),
            TestStep(7, "End the WebRTC session"),
            TestStep(8, "Read CurrentSessions attribute from DUT"),
        ]
        return steps

    def pics_TC_WebRTCR_2_5(self) -> list[str]:
        """
        Return the list of PICS applicable to this test case.
        """
        pics = [
            "WEBRTCR.S",           # WebRTC Transport Requestor Server
            "WEBRTCR.S.A0000",     # CurrentSessions attribute
            "WEBRTCR.S.C03.Rsp",   # End command
        ]
        return pics

    # This test has multiple manual steps for attribute reads and session management.
    # Test typically runs under 2 mins, so 5 minutes is sufficient.
    @property
    def default_timeout(self) -> int:
        return 5 * 60

    @async_test_body
    async def test_TC_WebRTCR_2_5(self):
        """
        Executes the test steps for the WebRTC CurrentSessions attribute validation.
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
            # TODO: send command to DUT via websocket
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
        # Prompt user to read CurrentSessions attribute before establishing session
        prompt_msg = (
            "\nRead CurrentSessions attribute from DUT:\n"
            "  webrtctransportprovider read current-sessions 1 1\n"
            "Input 'Y' if attribute read succeeds and returns an empty list\n"
            "Input 'N' if attribute read fails or returns non-empty list\n"
        )

        if self.is_pics_sdk_ci_only:
            # TODO: read attribute via websocket and verify empty list
            resp = 'Y'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        read_success = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            read_success,
            True,
            f"CurrentSessions attribute read {'succeeded with empty list' if read_success else 'failed or returned non-empty list'}"
        )

        self.step(5)
        # Prompt user to establish WebRTC session
        prompt_msg = (
            "\nEstablish WebRTC session between TH_SERVER and DUT:\n"
            "  webrtc establish-session 1\n"
            "Input 'Y' if WebRTC session is successfully established\n"
            "Input 'N' if WebRTC session establishment failed\n"
        )

        if self.is_pics_sdk_ci_only:
            # TODO: establish session via websocket
            resp = 'Y'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        session_success = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            session_success,
            True,
            f"WebRTC session {'established successfully' if session_success else 'failed to establish'}"
        )

        self.step(6)
        # Prompt user to read CurrentSessions attribute after establishing session
        prompt_msg = (
            "\nRead CurrentSessions attribute from DUT:\n"
            "  webrtctransportprovider read current-sessions 1 1\n"
            "If the read succeeds and returns a list with one WebRTCSessionStruct, enter the session ID\n"
            "If the read fails or returns incorrect data, enter 'N'\n"
            "Example: if the output shows '0x0 = 0 (unsigned),' then the session ID is 0, enter: 0\n"
            "Note: The session ID is the value of field 0x0 in the WebRTCSessionStruct\n"
        )

        if self.is_pics_sdk_ci_only:
            # TODO: read attribute via websocket and verify session info
            self.session_id = "1"  # Mock session ID for CI
            session_read_success = True
        else:
            resp = self.wait_for_user_input(prompt_msg)
            if resp.lower() == 'n':
                session_read_success = False
                self.session_id = None
            else:
                try:
                    # Validate that the input is a valid session ID (numeric)
                    int(resp.strip())
                    self.session_id = resp.strip()
                    session_read_success = True
                except ValueError:
                    session_read_success = False
                    self.session_id = None

        # Verify results
        asserts.assert_equal(
            session_read_success,
            True,
            ("CurrentSessions attribute read "
             f"{'succeeded with session info' if session_read_success else 'failed or returned incorrect data'}")
        )

        # Ensure we have a valid session ID before proceeding
        if not self.session_id:
            asserts.fail("No valid session ID obtained from CurrentSessions attribute read")

        self.step(7)
        # Prompt user to end WebRTC session
        prompt_msg = (
            "\nEnd the WebRTC session:\n"
            f"  webrtctransportprovider end-session {self.session_id} 2 1 1\n"
            "Input 'Y' if session is successfully terminated\n"
            "Input 'N' if session termination failed\n"
        )

        if self.is_pics_sdk_ci_only:
            # TODO: end session via websocket
            resp = 'Y'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        session_end_success = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            session_end_success,
            True,
            f"WebRTC session {'terminated successfully' if session_end_success else 'failed to terminate'}"
        )

        self.step(8)
        # Prompt user to read CurrentSessions attribute after ending session
        prompt_msg = (
            "\nRead CurrentSessions attribute from DUT:\n"
            "  webrtctransportprovider read current-sessions 1 1\n"
            "Input 'Y' if attribute read succeeds and returns an empty list\n"
            "Input 'N' if attribute read fails or returns non-empty list\n"
        )

        if self.is_pics_sdk_ci_only:
            # TODO: read attribute via websocket and verify empty list
            resp = 'Y'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        final_read_success = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            final_read_success,
            True,
            ("CurrentSessions attribute read"
             f"{'succeeded with empty list' if final_read_success else 'failed or returned non-empty list'}")
        )


if __name__ == "__main__":
    default_matter_test_main()
