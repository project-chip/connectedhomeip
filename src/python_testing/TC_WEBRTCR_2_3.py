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
#       --string-arg th_server_app_path:out/linux-x64-camera/chip-camera-app
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import os
import tempfile

import websockets
from chip.testing.apps import AppServerSubprocess
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

SERVER_URI = "ws://localhost:9002"


class TC_WebRTCRequestor_2_3(MatterBaseTest):
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
            port=5680,
            extra_args=["--camera-deferred-offer"]
        )

        self.th_server.start(
            expected_output="Server initialization complete",
            timeout=30
        )

    def teardown_class(self):
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    def desc_TC_WebRTCRequestor_2_3(self) -> str:
        """Returns a description of this test"""
        return "[TC-{picsCode}-2.3] Validate sending an SDP Offer command to {DUT_Server} with an existing session id"

    def steps_TC_WebRTCRequestor_2_3(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        steps = [
            TestStep(1, "Commission the {TH_Server} from DUT"),
            TestStep(2, "Connect the {TH_Server} from DUT"),
            TestStep(3, "Send SolicitOffer command to the {TH_Server}"),
        ]
        return steps

    # This test has some manual steps and one sleep for up to 30 seconds. Test typically
    # runs under 1 mins, so 3 minutes is more than enough.
    @property
    def default_timeout(self) -> int:
        return 3 * 60

    async def send_command(self, command):
        async with websockets.connect(SERVER_URI) as websocket:
            logging.info(f"Connected to {SERVER_URI}")

            # Send command
            logging.info(f"Sending command: {command}")
            await websocket.send(command)

            # Receive response
            await websocket.recv()
            logging.info("Received command response")

    @async_test_body
    async def test_TC_WebRTCRequestor_2_3(self):
        """
        Executes the test steps for the WebRTC Provider cluster scenario.
        """
        await asyncio.sleep(3)

        self.step(1)
        # Prompt user with instructions
        prompt_msg = (
            "\nPlease commission the server app from DUT:\n"
            "  pairing onnetwork 1 20202021\n"
            "Input 'Y' if DUT successfully commissions without any warnings\n"
            "Input 'N' if commissioner warns about commissioning the non-genuine device, "
            "Or Commissioning fails with device appropriate attestation error\n"
        )

        if self.is_pics_sdk_ci_only:
            await self.send_command("pairing onnetwork 1 20202021")
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

        self.step(2)
        await asyncio.sleep(1)
        # Prompt user with instructions
        prompt_msg = (
            "\nPlease connect the server app from DUT:\n"
            "  webrtc connect 1 1\n"
        )

        if self.is_pics_sdk_ci_only:
            await self.send_command("webrtc connect 1 1")
        else:
            self.wait_for_user_input(prompt_msg)

        self.step(3)
        await asyncio.sleep(1)
        # Prompt user with instructions
        prompt_msg = (
            "\nSend 'SolicitOffer' command to the server app from DUT:\n"
            "  webrtc solicit-offer 3\n"
            "Input 'Y' if WebRTC session is successfully established\n"
            "Input 'N' if WebRTC session is not established\n"
        )

        if self.is_pics_sdk_ci_only:
            self.th_server.set_output_match("PeerConnection State: Connected")
            self.th_server.event.clear()

            try:
                await self.send_command("webrtc solicit-offer 3")
                # Wait up to 90s until the provider logs that the data‑channel opened
                if not self.th_server.event.wait(90):
                    raise TimeoutError("PeerConnection is not connected within 90s")
                resp = 'Y'
            except TimeoutError:
                resp = 'N'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        session_success = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            session_success,
            True,
            f"WebRTC session {'established' if session_success else 'failed'}"
        )


if __name__ == "__main__":
    default_matter_test_main()
