#
#    Copyright (c) 2026 Project CHIP Authors
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
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

log = logging.getLogger(__name__)


class TC_WEBRTCP_2_33(MatterBaseTest, WEBRTCPTestBase):
    def desc_TC_WEBRTCP_2_33(self) -> str:
        """Returns a description of this test"""
        return "[TC-WEBRTCP-2.33] Validate WebRTC video and audio stream reuse and media delivery across consecutive sessions (referenceCount 0 -> 1 -> 0 -> 1)"

    def steps_TC_WEBRTCP_2_33(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned", is_commissioning=True),
            TestStep(
                1,
                "TH allocates both Audio and Video streams via CameraAVStreamManagement",
                "Valid stream IDs are obtained with initial referenceCount == 0",
            ),
            TestStep(
                2,
                "TH establishes first WebRTC session (Session 1) using the allocated streams",
                "Valid WebRTCSessionID is obtained, referenceCounts increment to 1, and WebRTC session reaches Connected state",
            ),
            TestStep(
                3,
                "TH verifies active RTP video and audio media reception during Session 1",
                "Non-zero RTP video frames and audio packets are received by the native WebRTC peer connection",
            ),
            TestStep(
                4,
                "TH sends EndSession for Session 1 and reads AllocatedVideoStreams / AllocatedAudioStreams",
                "DUT responds with SUCCESS; referenceCounts decrement back to 0 while streams remain allocated",
            ),
            TestStep(
                5,
                "TH establishes second WebRTC session (Session 2) reusing the SAME allocated VideoStreamID and AudioStreamID without deallocating",
                "DUT responds with ProvideOfferResponse, referenceCounts increment from 0 to 1, and Session 2 reaches Connected state",
            ),
            TestStep(
                6,
                "TH verifies active RTP video and audio media reception during Session 2 (stream reuse)",
                "Non-zero RTP video frames and audio packets are received by the native WebRTC peer connection during the reused stream session",
            ),
            TestStep(
                7,
                "TH terminates Session 2 via EndSession and deallocates the Audio and Video streams",
                "DUT responds with SUCCESS and streams are removed from AllocatedAudioStreams / AllocatedVideoStreams",
            ),
        ]

    def pics_TC_WEBRTCP_2_33(self) -> list[str]:
        return [
            "WEBRTCP.S",
            "WEBRTCP.S.A0000",     # CurrentSessions attribute
            "WEBRTCP.S.C02.Rsp",   # ProvideOffer command
            "WEBRTCP.S.C06.Rsp",   # EndSession command
            "AVSM.S",
            "AVSM.S.F00",          # Audio Data Output feature
            "AVSM.S.F01",          # Video Data Output feature
        ]

    async def _get_stream_ref_count(self, stream_id: int, attribute, endpoint: int) -> int:
        streams = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.CameraAvStreamManagement,
            attribute=attribute,
        )
        stream_id_field = (
            "audioStreamID"
            if attribute == Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams
            else "videoStreamID"
        )
        for stream in streams:
            if getattr(stream, stream_id_field) == stream_id:
                return stream.referenceCount
        asserts.fail(f"Could not find stream {stream_id}")
        return None

    async def _establish_session(
        self,
        webrtc_manager: WebRTCManager,
        endpoint: int,
        video_stream_id: int,
        audio_stream_id: int,
    ) -> tuple[int, LibdatachannelPeerConnection]:
        peer: LibdatachannelPeerConnection = webrtc_manager.create_peer(
            node_id=self.dut_node_id,
            fabric_index=self.default_controller.GetFabricIndexInternal(),
            endpoint=endpoint,
        )
        peer.create_offer()
        offer = await peer.get_local_offer()

        provide_offer_response: Clusters.WebRTCTransportProvider.Commands.ProvideOfferResponse = await peer.send_command(
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
        asserts.assert_true(session_id >= 0, f"Invalid WebRTC session ID: {session_id}")
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        answer_session_id, answer = await peer.get_remote_answer()
        asserts.assert_equal(session_id, answer_session_id, "Answer session ID mismatch")
        peer.set_remote_answer(answer)

        local_candidates = await peer.get_local_ice_candidates()
        local_candidates_struct_list = [
            Clusters.Objects.Globals.Structs.ICECandidateStruct(candidate=cand.candidate) for cand in local_candidates
        ]
        await self.send_single_cmd(
            cmd=Clusters.WebRTCTransportProvider.Commands.ProvideICECandidates(
                webRTCSessionID=answer_session_id,
                ICECandidates=local_candidates_struct_list,
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )

        ice_session_id, remote_candidates = await peer.get_remote_ice_candidates()
        asserts.assert_equal(session_id, ice_session_id, "ProvideIceCandidates invoked with wrong session id")
        asserts.assert_true(len(remote_candidates) > 0, "Invalid remote ICE candidates received")
        peer.set_remote_ice_candidates(remote_candidates)

        if not await peer.check_for_session_establishment():
            asserts.fail(f"Failed to establish WebRTC session {session_id}")

        return session_id, peer

    async def _verify_media_delivery(
        self,
        peer: LibdatachannelPeerConnection,
        session_label: str,
        timeout_sec: float = 5.0,
        sample_duration_sec: float = 1.5,
    ):
        log.info("[%s] Checking native per-peer RTP media counters...", session_label)
        video_frames, video_bytes, audio_packets, audio_bytes = await peer.wait_for_media_delivery(
            expect_video=True,
            expect_audio=True,
            timeout_s=timeout_sec,
            sample_duration_s=sample_duration_sec,
        )
        log.info(
            "[%s] Media stats over %.1fs: Video=%d frames (%d bytes), Audio=%d packets (%d bytes)",
            session_label,
            sample_duration_sec,
            video_frames,
            video_bytes,
            audio_packets,
            audio_bytes,
        )
        asserts.assert_greater(
            video_frames,
            0,
            f"[{session_label}] Expected > 0 RTP video frames, got {video_frames} (0 video packets received over {timeout_sec}s)",
        )
        asserts.assert_greater(
            audio_packets,
            0,
            f"[{session_label}] Expected > 0 RTP audio packets, got {audio_packets} (0 audio packets received over {timeout_sec}s)",
        )

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WEBRTCP_2_33(self):
        self.step("precondition")
        endpoint = self.get_endpoint()
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)

        try:
            self.step(1)
            audio_stream_id = await self.allocate_one_audio_stream()
            video_stream_id = await self.allocate_one_video_stream()
            await self.validate_allocated_audio_stream(audio_stream_id)
            await self.validate_allocated_video_stream(video_stream_id)

            initial_audio_ref = await self._get_stream_ref_count(
                audio_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams, endpoint
            )
            initial_video_ref = await self._get_stream_ref_count(
                video_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams, endpoint
            )
            asserts.assert_equal(initial_audio_ref, 0, "Initial audio referenceCount must be 0")
            asserts.assert_equal(initial_video_ref, 0, "Initial video referenceCount must be 0")

            self.step(2)
            log.info("Starting Session 1 with freshly allocated streams (video=%s, audio=%s)", video_stream_id, audio_stream_id)
            session1_id, peer1 = await self._establish_session(webrtc_manager, endpoint, video_stream_id, audio_stream_id)

            s1_audio_ref = await self._get_stream_ref_count(
                audio_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams, endpoint
            )
            s1_video_ref = await self._get_stream_ref_count(
                video_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams, endpoint
            )
            asserts.assert_equal(s1_audio_ref, 1, "Session 1 audio referenceCount must be 1")
            asserts.assert_equal(s1_video_ref, 1, "Session 1 video referenceCount must be 1")

            self.step(3)
            await self._verify_media_delivery(peer1, "Session 1 (Fresh Stream)")

            self.step(4)
            log.info("Ending Session 1 (session_id=%s) WITHOUT deallocating streams", session1_id)
            await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                    webRTCSessionID=session1_id,
                    reason=Clusters.Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup,
                ),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            await webrtc_manager.remove_peer(session1_id)

            between_audio_ref = await self._get_stream_ref_count(
                audio_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams, endpoint
            )
            between_video_ref = await self._get_stream_ref_count(
                video_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams, endpoint
            )
            asserts.assert_equal(between_audio_ref, 0, "Audio referenceCount must decrement back to 0 after EndSession")
            asserts.assert_equal(between_video_ref, 0, "Video referenceCount must decrement back to 0 after EndSession")

            self.step(5)
            log.info(
                "Starting Session 2 REUSING existing allocated streams (video=%s, audio=%s) with referenceCount == 0",
                video_stream_id,
                audio_stream_id,
            )
            session2_id, peer2 = await self._establish_session(webrtc_manager, endpoint, video_stream_id, audio_stream_id)

            s2_audio_ref = await self._get_stream_ref_count(
                audio_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams, endpoint
            )
            s2_video_ref = await self._get_stream_ref_count(
                video_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams, endpoint
            )
            asserts.assert_equal(s2_audio_ref, 1, "Session 2 audio referenceCount must increment back to 1")
            asserts.assert_equal(s2_video_ref, 1, "Session 2 video referenceCount must increment back to 1")

            self.step(6)
            await self._verify_media_delivery(peer2, "Session 2 (Reused Stream)")

            self.step(7)
            log.info("Ending Session 2 and deallocating streams")
            await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                    webRTCSessionID=session2_id,
                    reason=Clusters.Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup,
                ),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            await webrtc_manager.remove_peer(session2_id)

            await self.send_single_cmd(
                cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamDeallocate(audioStreamID=audio_stream_id),
                endpoint=endpoint,
            )
            await self.send_single_cmd(
                cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamDeallocate(videoStreamID=video_stream_id),
                endpoint=endpoint,
            )

        finally:
            await webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
