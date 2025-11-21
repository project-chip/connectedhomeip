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

logger = logging.getLogger(__name__)


class TC_WEBRTCP_2_23(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_23(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.23] Validate EndSession decrements stream reference counts"

    def steps_TC_WEBRTCP_2_23(self) -> list[TestStep]:
        steps = [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(1, "TH allocates both Audio and Video streams via CameraAVStreamManagement",
                     "Valid stream IDs are obtained"),
            TestStep(2, "TH reads the AllocatedAudioStreams and AllocatedVideoStreams attributes to check reference counts",
                     "Reference counts for allocated streams are recorded"),
            TestStep(3, "TH establishes a WebRTC session using the allocated streams",
                     "Valid WebRTCSessionID is obtained and session is active"),
            TestStep(4, "TH reads the stream attributes again to verify reference counts increased",
                     "Reference counts for used streams have increased"),
            TestStep(5, "TH sends the EndSession command with the valid WebRTCSessionID",
                     "DUT responds with success status code"),
            TestStep(6, "TH reads the stream attributes again to verify reference counts decreased",
                     "Reference counts for streams have decremented back to their original values"),
            TestStep(7, "TH deallocates the Audio and Video streams via AudioStreamDeallocate and VideoStreamDeallocate commands",
                     "DUT responds with success status code for both deallocate commands")
        ]
        return steps

    def pics_TC_WEBRTCP_2_23(self) -> list[str]:
        pics = [
            "WEBRTCP.S",
            "WEBRTCP.S.A0000",     # CurrentSessions attribute
            "WEBRTCP.S.C06.Rsp",   # EndSession command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]
        return pics

    async def _get_stream_ref_count(self, stream_id: int, attribute, endpoint: int) -> int:
        """
        Helper method to get the reference count for a specific stream.

        Args:
            stream_id: The ID of the stream to find
            attribute: The attribute to read (AllocatedAudioStreams or AllocatedVideoStreams)
            endpoint: The endpoint to read from

        Returns:
            The reference count for the specified stream
        """
        streams = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.CameraAvStreamManagement,
            attribute=attribute
        )

        # Determine which field to use based on the attribute type
        stream_id_field = 'audioStreamID' if attribute == Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams else 'videoStreamID'

        for stream in streams:
            if getattr(stream, stream_id_field) == stream_id:
                return stream.referenceCount

        asserts.fail(f'Could not find stream {stream_id}')

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_23(self):
        """
        Executes the test steps for validating EndSession decrements stream reference counts.
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
        # Read initial reference counts for the allocated streams
        logger.info("Reading initial reference counts")

        initial_audio_ref_count = await self._get_stream_ref_count(
            audio_stream_id,
            Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams,
            endpoint
        )
        initial_video_ref_count = await self._get_stream_ref_count(
            video_stream_id,
            Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams,
            endpoint
        )

        logger.info(f"Initial reference counts - Audio: {initial_audio_ref_count}, Video: {initial_video_ref_count}")

        self.step(3)
        # Establish a WebRTC session using the allocated streams
        logger.info("Establishing WebRTC session with allocated streams")

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
        logger.info(f"Valid WebRTC session ID: {session_id} is obtained")

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
            logging.error("Failed to establish webrtc session")
            raise Exception("Failed to establish webrtc session")

        logger.info(f"WebRTC session established with session ID: {session_id}")

        self.step(4)
        # Read reference counts again to verify they increased
        logger.info("Reading reference counts after session establishment")

        active_audio_ref_count = await self._get_stream_ref_count(
            audio_stream_id,
            Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams,
            endpoint
        )
        active_video_ref_count = await self._get_stream_ref_count(
            video_stream_id,
            Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams,
            endpoint
        )

        # Verify reference counts increased
        asserts.assert_equal(active_audio_ref_count, initial_audio_ref_count + 1,
                             f"Audio reference count should have increased from {initial_audio_ref_count} to {initial_audio_ref_count + 1}")
        asserts.assert_equal(active_video_ref_count, initial_video_ref_count + 1,
                             f"Video reference count should have increased from {initial_video_ref_count} to {initial_video_ref_count + 1}")
        logger.info(f"Reference counts increased - Audio: {active_audio_ref_count}, Video: {active_video_ref_count}")

        self.step(5)
        # Send EndSession command
        logger.info(f"Sending EndSession command for session {session_id}")
        await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id,
                reason=Clusters.Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )
        logger.info(f"Successfully sent EndSession command for session {session_id}")

        self.step(6)
        # Read reference counts again to verify they decreased back to original values
        logger.info("Reading reference counts after EndSession")

        final_audio_ref_count = await self._get_stream_ref_count(
            audio_stream_id,
            Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams,
            endpoint
        )
        final_video_ref_count = await self._get_stream_ref_count(
            video_stream_id,
            Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams,
            endpoint
        )

        # Verify reference counts decreased back to original values
        asserts.assert_equal(final_audio_ref_count, initial_audio_ref_count,
                             f"Audio reference count should have decreased back to {initial_audio_ref_count}")
        asserts.assert_equal(final_video_ref_count, initial_video_ref_count,
                             f"Video reference count should have decreased back to {initial_video_ref_count}")
        logger.info(f"Reference counts decreased - Audio: {final_audio_ref_count}, Video: {final_video_ref_count}")

        self.step(7)
        # Deallocate the Audio and Video streams
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
