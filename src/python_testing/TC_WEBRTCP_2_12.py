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
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import logging

from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

logger = logging.getLogger(__name__)


class TC_WEBRTCP_2_12(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_12(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.12] Validate SolicitOffer resource exhaustion"

    def steps_TC_WEBRTCP_2_12(self) -> list[TestStep]:
        steps = [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success and provides stream IDs"),
            TestStep(2, "TH sends multiple SolicitOffer commands to exhaust the DUT's capacity for WebRTC sessions (DUT-specific limit)",
                     "DUT successfully creates sessions until capacity is reached"),
            TestStep(3, "TH sends an additional SolicitOffer command when DUT capacity is exhausted",
                     "DUT responds with SolicitOfferResponse containing allocated WebRTCSessionID"),
            TestStep(4, "TH waits for DUT to send End command with reason OutOfResources",
                     "DUT sends End command with reason OutOfResources (value 0x08)")
        ]
        return steps

    def pics_TC_WEBRTCP_2_12(self) -> list[str]:
        pics = [
            "WEBRTCP.S",
            "WEBRTCP.S.C00.Rsp",   # SolicitOffer command
            "WEBRTCP.S.C01.Tx",    # SolicitOfferResponse command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]
        return pics

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_12(self):
        """
        Executes the test steps for validating SolicitOffer resource exhaustion.
        """

        self.step("precondition")
        # Commission DUT - already done
        endpoint = self.get_endpoint()

        self.step(1)
        # Allocate Audio and Video streams
        audio_stream_id = await self.allocate_one_audio_stream()
        video_stream_id = await self.allocate_one_video_stream()

        # Validate that the streams were allocated successfully
        await self.validate_allocated_audio_stream(audio_stream_id)
        await self.validate_allocated_video_stream(video_stream_id)

        # Create WebRTC manager and peer for receiving End commands
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        self.step(2)
        # Send multiple SolicitOffer commands to exhaust the DUT's capacity
        logger.info("Starting to send SolicitOffer commands to exhaust DUT capacity")

        # Get the maximum concurrent WebRTC sessions from user or use default for CI
        prompt_msg = (
            "\nPlease enter the maximum number of concurrent WebRTC sessions supported by the DUT:\n"
            "This value is DUT-specific and should be obtained from the DUT documentation or specifications.\n"
            "Enter the number (e.g., 5): "
        )

        if self.is_pics_sdk_ci_only:
            # Use default value for CI testing
            max_attempts = 5
        else:
            user_input = self.wait_for_user_input(prompt_msg)
            try:
                max_attempts = int(user_input.strip())
                asserts.assert_true(max_attempts > 0, "Maximum concurrent sessions must be greater than 0")
                logger.info(f"Using user-specified max_attempts={max_attempts}")
            except ValueError:
                asserts.fail(f"Invalid input '{user_input}'. Please enter a valid number.")

        # Try to allocate multiple sessions to reach the DUT's capacity limit

        for attempt in range(max_attempts):
            logger.info(f"Attempt {attempt + 1}: Sending SolicitOffer command")
            resp: Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse = await webrtc_peer.send_command(
                cmd=Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
                    streamUsage=Clusters.Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                    videoStreamID=video_stream_id,
                    audioStreamID=audio_stream_id,
                    originatingEndpointID=1,
                ),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                                 "Incorrect response type")
            session_id = resp.webRTCSessionID
            asserts.assert_true(session_id >= 0, f"Invalid session ID: {session_id}")
            logger.info(f"Created session {session_id} in attempt {attempt + 1}")
            webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(3)
        # Send an additional SolicitOffer command when DUT capacity is exhausted
        logger.info("Sending additional SolicitOffer command to trigger resource exhaustion")

        # The resource exhaustion happens internally in the DUT, but it still creates a session
        # and then sends an End command with OutOfResources reason.
        resp: Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse = await webrtc_peer.send_command(
            cmd=Clusters.WebRTCTransportProvider.Commands.SolicitOffer(
                streamUsage=Clusters.Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=video_stream_id,
                audioStreamID=audio_stream_id,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )

        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.SolicitOfferResponse,
                             "Incorrect response type")
        session_id = resp.webRTCSessionID
        asserts.assert_true(session_id >= 0, f"Invalid session ID: {session_id}")
        logger.info(f"DUT allocated session {session_id} despite resource exhaustion")
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(4)
        # Wait for DUT to send End command with reason OutOfResources
        logger.info("Waiting for DUT to send End command with OutOfResources reason")

        # Wait for the End command from the DUT
        end_sessionId, reason = await webrtc_peer.get_remote_end()

        # Verify the End command has OutOfResources reason (value 0x08)
        kOutOfResourcesReason = 0x08  # WebRTCEndReasonEnum.kOutOfResources
        asserts.assert_equal(reason, kOutOfResourcesReason,
                             f"Expected OutOfResources reason ({kOutOfResourcesReason}), got {reason}")

        logger.info(f"Successfully received End command for session {end_sessionId} with OutOfResources reason {reason}")

        await webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
