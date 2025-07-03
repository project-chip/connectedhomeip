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

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.testing.apps import AppServerSubprocess
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_WebRTCRequestor_2_2(MatterBaseTest):
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
            passcode=self.th_server_passcode
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

    def desc_TC_WebRTCRequestor_2_2(self) -> str:
        """Returns a description of this test"""
        return "[TC-{picsCode}-2.2] Validate sending an SDP Answer command to {DUT_Server} with an invalid session id"

    def steps_TC_WebRTCRequestor_2_2(self) -> list[TestStep]:
        """
        Define the step-by-step sequence for the test.
        """
        steps = [
            TestStep(1, "Commission the {TH_Server} from TH"),
            TestStep(2, "Open the Commissioning Window of the {TH_Server}"),
            TestStep(3, "Commission the {TH_Server} from DUT"),
            TestStep(4, "Connect the {TH_Server} from DUT"),
            TestStep(5, "Activate the Fault injection to modify the session ID of the WebRTC Answer command from {TH_Server}"),
            TestStep(6, "Send ProvideOffer command to the {TH_Server} from DUT"),
        ]
        return steps

    # This test has some manual steps and one sleep for up to 30 seconds. Test typically
    # runs under 1 mins, so 3 minutes is more than enough.
    @property
    def default_timeout(self) -> int:
        return 3 * 60

    @async_test_body
    async def test_TC_WebRTCRequestor_2_2(self):
        """
        Executes the test steps for the WebRTC Provider cluster scenario.
        """

        discriminator = 1234
        passcode = 20202021
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
            "\nPlease commission the server app from DUT:\n"
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
        # Prompt user with instructions
        prompt_msg = (
            "\nPlease connect the server app from DUT:\n"
            "  webrtc connect 1 1\n"
        )

        if self.is_pics_sdk_ci_only:
            # TODO: send command to DUT via websocket
            pass
        else:
            self.wait_for_user_input(prompt_msg)

        self.step(5)
        logging.info("Injecting kFault_ModifyWebRTCAnswerSessionId on TH_SERVER")

        # --- Fault‑Injection cluster (mfg‑specific 0xFFF1_FC06) ---
        # Use FailAtFault to activate the chip‑layer fault exactly once
        #
        #  • faultType = kChipFault (0x03)  – always used for CHIP faults
        #  • id        = FaultInjection.Id.kFault_ModifyWebRTCAnswerSessionId
        #  • numCallsToSkip = 0  – trigger on the very next call
        #  • numCallsToFail = 1  – inject once, then auto‑clear
        #  • takeMutex      = False  – single‑threaded app, no lock needed
        #
        command = Clusters.FaultInjection.Commands.FailAtFault(
            type=Clusters.FaultInjection.Enums.FaultType.kChipFault,
            id=15,  # kFault_ModifyWebRTCAnswerSessionId
            numCallsToFail=1,
            takeMutex=False,
        )
        await self.default_controller.SendCommand(
            nodeid=self.th_server_local_nodeid,
            endpoint=0,  # Fault‑Injection cluster lives on EP0
            payload=command,
        )
        sleep(1)

        self.step(6)
        # Prompt user with instructions
        prompt_msg = (
            "\nSend 'ProvideOffer' command to the server app from DUT:\n"
            "  webrtc provide-offer 3\n"
            "Input 'Y' if WebRTC session is failed with error 'NOT_FOUND'\n"
            "Input 'N' if WebRTC session is successfully established\n"
        )

        if self.is_pics_sdk_ci_only:
            # TODO: send command to DUT via websocket
            resp = 'Y'
        else:
            resp = self.wait_for_user_input(prompt_msg)

        result = resp.lower() == 'y'

        # Verify results
        asserts.assert_equal(
            result,
            True,
            f"WebRTC session {'failed as expected' if result else 'unexpectedly succeeded'}"
        )


if __name__ == "__main__":
    default_matter_test_main()
