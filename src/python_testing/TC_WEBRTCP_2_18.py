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


class TC_WEBRTCP_2_18(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_18(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.18] Validate ProvideAnswer command processing"

    def steps_TC_WEBRTCP_2_18(self) -> list[TestStep]:
        steps = [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success and provides stream IDs"),
            TestStep(2, "TH sends the SolicitOffer command with valid stream IDs",
                     "DUT responds with SolicitOfferResponse containing allocated WebRTCSessionID"),
            TestStep(3, "TH waits for incoming Offer command from DUT on WebRTC Requestor cluster",
                     "DUT sends Offer command with the allocated WebRTCSessionID and valid SDP offer"),
            TestStep(4, "TH sends the ProvideAnswer command with valid WebRTCSessionID and valid SDP answer",
                     "DUT responds with success status code"),
            TestStep(5, "TH sends EndSession command to terminate the WebRTC session",
                     "DUT responds with success status code"),
            TestStep(6, "TH deallocates the Audio and Video streams via AudioStreamDeallocate and VideoStreamDeallocate commands",
                     "DUT responds with success status code for both deallocate commands"),
        ]
        return steps

    def pics_TC_WEBRTCP_2_18(self) -> list[str]:
        pics = [
            "WEBRTCP.S",
            "WEBRTCP.S.C00.Rsp",   # SolicitOffer command
            "WEBRTCP.S.C01.Tx",    # SolicitOfferResponse command
            "WEBRTCR.S.C04.Rsp",   # ProvideAnswer command
            "WEBRTCP.S.C06.Rsp",   # EndSession command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]
        return pics

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_18(self):
        """
        Executes the test steps for validating ProvideAnswer command processing.
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

        # Create WebRTC manager and peer for sending SolicitOffer and receiving Offer
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        self.step(2)
        # Send SolicitOffer command with valid stream IDs
        logger.info("Sending SolicitOffer command")

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
        logger.info(f"DUT allocated WebRTC session ID: {session_id}")

        # Register the session ID with the WebRTC manager so it can handle incoming commands
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(3)
        # Wait for incoming Offer command from DUT on WebRTC Requestor cluster
        logger.info(f"Waiting for Offer command from DUT for session {session_id}")

        # Wait for the Offer command from the DUT
        offer_sessionId, offer_sdp = await webrtc_peer.get_remote_offer(timeout_s=30)

        # Verify the Offer command contains the same session ID
        asserts.assert_equal(offer_sessionId, session_id,
                             f"Offer session ID {offer_sessionId} does not match expected {session_id}")
        logger.info(f"Received Offer command for session {session_id}")

        # Verify the Offer contains valid SDP content
        asserts.assert_true(len(offer_sdp) > 0, "SDP offer is empty")
        logger.info(f"Received SDP offer with length: {len(offer_sdp)} bytes")
        webrtc_peer.set_remote_offer(offer_sdp)

        self.step(4)
        # Send ProvideAnswer command with valid WebRTCSessionID and valid SDP answer
        logger.info(f"Sending ProvideAnswer command for session {session_id}")

        # Create a valid SDP answer in response to the received offer
        local_answer = await webrtc_peer.get_local_description_with_ice_candidates()
        await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideAnswer(webRTCSessionID=session_id, sdp=local_answer),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )

        logger.info(f"Successfully sent ProvideAnswer command for session {session_id}")

        self.step(5)
        # Send EndSession command to terminate the WebRTC session
        logger.info(f"Sending EndSession command for session {session_id}")

        await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id,
                reason=Clusters.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
        )

        logger.info(f"Successfully ended WebRTC session {session_id}")

        self.step(6)
        # Deallocate the Audio and Video streams to return DUT to known state
        logger.info("Deallocating Audio and Video streams")

        # Deallocate audio stream
        await self.send_single_cmd(
            cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamDeallocate(
                audioStreamID=audio_stream_id
            ),
            endpoint=endpoint,
        )
        logger.info(f"Successfully deallocated audio stream {audio_stream_id}")

        # Deallocate video stream
        await self.send_single_cmd(
            cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamDeallocate(
                videoStreamID=video_stream_id
            ),
            endpoint=endpoint,
        )
        logger.info(f"Successfully deallocated video stream {video_stream_id}")

        # Clean up
        await webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
