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
from matter.clusters.Types import NullValue
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

logger = logging.getLogger(__name__)


class TC_WEBRTCP_2_17(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_17(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.17] Validate ProvideOffer generates Answer command"

    def steps_TC_WEBRTCP_2_17(self) -> list[TestStep]:
        steps = [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via AudioStreamAllocate and VideoStreamAllocate commands to CameraAVStreamManagement",
                     "DUT responds with success and provides stream IDs"),
            TestStep(2, "TH sends the ProvideOffer command with null WebRTCSessionID and valid SDP offer",
                     "DUT responds with ProvideOfferResponse containing allocated WebRTCSessionID"),
            TestStep(3, "TH waits for incoming Answer command from DUT on WebRTC Requestor cluster",
                     "DUT sends Answer command to TH with the allocated WebRTCSessionID and valid SDP answer"),
            TestStep(4, "TH verifies the Answer command contains valid SDP answer content",
                     "Answer command contains properly formatted SDP answer matching the original offer"),
            TestStep(5, "TH sends EndSession command to terminate the WebRTC session",
                     "DUT responds with success status code"),
            TestStep(6, "TH deallocates the Audio and Video streams via AudioStreamDeallocate and VideoStreamDeallocate commands",
                     "DUT responds with success status code for both deallocate commands"),
        ]
        return steps

    def pics_TC_WEBRTCP_2_17(self) -> list[str]:
        pics = [
            "WEBRTCP.S",
            "WEBRTCP.S.C02.Rsp",   # ProvideOffer command
            "WEBRTCP.S.C03.Tx",    # ProvideOfferResponse command
            "WEBRTCR.C.C01.Tx",    # Answer command
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
    async def test_TC_WEBRTCP_2_17(self):
        """
        Executes the test steps for validating ProvideOffer generates Answer command.
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

        # Create WebRTC manager and peer for sending ProvideOffer and receiving Answer
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=endpoint
        )

        self.step(2)
        # Send ProvideOffer command with null WebRTCSessionID and valid SDP offer
        logger.info("Sending ProvideOffer command with null WebRTCSessionID")

        # Create a valid SDP offer
        webrtc_peer.create_offer()
        offer = await webrtc_peer.get_local_offer()

        resp: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await webrtc_peer.send_command(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=offer,
                streamUsage=Clusters.Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=video_stream_id,
                audioStreamID=audio_stream_id,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )

        asserts.assert_equal(type(resp), Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse,
                             "Incorrect response type")
        session_id = resp.webRTCSessionID
        asserts.assert_true(session_id >= 0, f"Invalid session ID: {session_id}")
        logger.info(f"DUT allocated WebRTC session ID: {session_id}")
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(3)
        # Wait for incoming Answer command from DUT on WebRTC Requestor cluster
        logger.info(f"Waiting for Answer command from DUT for session {session_id}")

        # Wait for the Answer command from the DUT
        answer_sessionId, answer_sdp = await webrtc_peer.get_remote_answer()

        # Verify the Answer command contains the same session ID
        asserts.assert_equal(answer_sessionId, session_id,
                             f"Answer session ID {answer_sessionId} does not match expected {session_id}")
        logger.info(f"Received Answer command for session {session_id}")

        self.step(4)
        # Verify the Answer command contains valid SDP answer content
        logger.info("Verifying SDP answer content")

        # Basic validation of SDP answer format
        asserts.assert_true(len(answer_sdp) > 0, "SDP answer is empty")
        asserts.assert_equal(type(answer_sdp), str, "SDP answer has incorrect type")

        # Verify SDP answer contains required session-level fields
        asserts.assert_true("v=0" in answer_sdp, "SDP answer missing version line (v=0)")
        asserts.assert_true("s=" in answer_sdp, "SDP answer missing session name line (s=)")
        asserts.assert_true("t=" in answer_sdp, "SDP answer missing timing line (t=)")

        # Verify SDP answer contains media descriptions
        asserts.assert_true("m=video" in answer_sdp, "SDP answer missing media description line (m=video)")
        asserts.assert_true("m=audio" in answer_sdp, "SDP answer missing media description line (m=audio)")

        logger.info(f"SDP answer validated successfully. Answer length: {len(answer_sdp)} bytes")
        logger.info(f"SDP answer preview: {answer_sdp[:400]}...")

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
