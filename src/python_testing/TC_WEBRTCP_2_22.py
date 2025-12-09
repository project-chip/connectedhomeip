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

log = logging.getLogger(__name__)


class TC_WEBRTCP_2_22(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_22(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.22] Validate EndSession removes session from CurrentSessions"

    def steps_TC_WEBRTCP_2_22(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via CameraAVStreamManagement",
                     "Valid stream IDs are obtained"),
            TestStep(2, "TH establishes a valid WebRTC session with DUT",
                     "Valid WebRTCSessionID is obtained and session is active"),
            TestStep(3, "TH reads CurrentSessions attribute from WebRTCTransportProvider on DUT",
                     "Verify the number of WebRTCSession in the list is 1 and contains the active session"),
            TestStep(4, "TH sends the EndSession command with the valid WebRTCSessionID",
                     "DUT responds with success status code"),
            TestStep(5, "TH reads CurrentSessions attribute from WebRTCTransportProvider on DUT",
                     "Verify the number of WebRTCSession in the list is 0 (session removed)"),
            TestStep(6, "TH deallocates the Audio and Video streams via AudioStreamDeallocate and VideoStreamDeallocate commands",
                     "DUT responds with success status code for both deallocate commands")
        ]

    def pics_TC_WEBRTCP_2_22(self) -> list[str]:
        return [
            "WEBRTCP.S",
            "WEBRTCP.S.A0000",     # CurrentSessions attribute
            "WEBRTCP.S.C06.Rsp",   # EndSession command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_22(self):
        """
        Executes the test steps for validating EndSession removes session from CurrentSessions.
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
        # Establish a valid WebRTC session with DUT
        log.info("Establishing WebRTC session with DUT")

        # Create and send ProvideOffer
        webrtc_peer.create_offer()
        offer = await webrtc_peer.get_local_offer()

        provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await webrtc_peer.send_command(
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
        session_id = provide_offer_response.webRTCSessionID
        asserts.assert_true(session_id >= 0, f"Invalid session ID: {session_id}")
        log.info(f"Valid WebRTC session ID: {session_id} is obtained")

        # Register session with WebRTC manager
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        # Wait for Answer command
        answer_sessionId, answer = await webrtc_peer.get_remote_answer()
        asserts.assert_equal(session_id, answer_sessionId, "Answer session ID mismatch")
        webrtc_peer.set_remote_answer(answer)

        # Send ProvideIceCandidates command
        local_candidates = await webrtc_peer.get_local_ice_candidates()
        local_candidates_struct_list = [
            Clusters.Objects.Globals.Structs.ICECandidateStruct(candidate=cand.candidate) for cand in local_candidates
        ]
        await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideICECandidates(
                webRTCSessionID=answer_sessionId, ICECandidates=local_candidates_struct_list
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )

        # Wait for IceCandidates command
        ice_session_id, remote_candidates = await webrtc_peer.get_remote_ice_candidates()
        asserts.assert_equal(session_id, ice_session_id, "ProvideIceCandidates invoked with wrong session id")
        asserts.assert_true(len(remote_candidates) > 0, "Invalid remote ice candidates received")
        webrtc_peer.set_remote_ice_candidates(remote_candidates)

        # Valid WebRTC session is established
        if not await webrtc_peer.check_for_session_establishment():
            log.error("Failed to establish webrtc session")
            raise Exception("Failed to establish webrtc session")

        log.info(f"WebRTC session established with session ID: {session_id}")

        self.step(3)
        # Read CurrentSessions attribute and verify the session is present
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.WebRTCTransportProvider,
            attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 1, "Expected exactly 1 active session")
        asserts.assert_equal(current_sessions[0].id, session_id, "Session ID in CurrentSessions does not match")
        log.info(f"Verified session {session_id} is present in CurrentSessions")

        self.step(4)
        # Send EndSession command
        log.info(f"Sending EndSession command for session {session_id}")
        await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id,
                reason=Clusters.Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )
        log.info(f"Successfully sent EndSession command for session {session_id}")

        self.step(5)
        # Read CurrentSessions attribute and verify the session is removed
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.WebRTCTransportProvider,
            attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions
        )
        asserts.assert_equal(len(current_sessions), 0, "Expected session to be removed from CurrentSessions")
        log.info(f"Verified session {session_id} has been removed from CurrentSessions")

        self.step(6)
        # Deallocate the Audio and Video streams to return DUT to known state
        log.info("Deallocating Audio and Video streams")

        # Deallocate audio stream
        await self.send_single_cmd(
            cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamDeallocate(
                audioStreamID=audio_stream_id
            ),
            endpoint=endpoint,
        )
        log.info(f"Successfully deallocated audio stream {audio_stream_id}")

        # Deallocate video stream
        await self.send_single_cmd(
            cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamDeallocate(
                videoStreamID=video_stream_id
            ),
            endpoint=endpoint,
        )
        log.info(f"Successfully deallocated video stream {video_stream_id}")

        # Clean up
        await webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
