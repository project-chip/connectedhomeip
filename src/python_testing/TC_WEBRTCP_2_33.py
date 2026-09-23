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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --camera-test-videosrc --camera-test-audiosrc
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
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

log = logging.getLogger(__name__)


class TC_WEBRTCP_2_33(MatterBaseTest, WEBRTCPTestBase):

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
                # originatingEndpointID is the Requestor (TH) endpoint hosting the WebRTCTransportRequestor
                # server cluster (hardcoded as kWebRTCRequesterDynamicEndpointId = 1 in
                # WebRTCTransportRequestorManager.h) where the DUT sends Answer/ICECandidates callbacks,
                # whereas `endpoint` below is the destination endpoint on the DUT.
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
        )
        session_id = provide_offer_response.webRTCSessionID
        asserts.assert_true(session_id >= 0, f"Invalid WebRTC session ID: {session_id}")
        self._active_session_id = session_id
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
    ):
        log.info("[%s] Checking native per-peer RTP media counters...", session_label)
        stats = await peer.wait_for_media_delivery(
            expect_video=True,
            expect_audio=True,
            timeout_s=timeout_sec,
        )
        log.info(
            "[%s] Media stats: Video=%d frames (%d bytes), Audio=%d packets (%d bytes)",
            session_label,
            stats.video_frames,
            stats.video_bytes,
            stats.audio_packets,
            stats.audio_bytes,
        )
        asserts.assert_greater(
            stats.video_frames,
            0,
            f"[{session_label}] Expected > 0 RTP video frames, got {stats.video_frames} (0 video frames received over {timeout_sec}s)",
        )
        asserts.assert_greater(
            stats.audio_packets,
            0,
            f"[{session_label}] Expected > 0 RTP audio packets, got {stats.audio_packets} (0 audio packets received over {timeout_sec}s)",
        )

    @property
    def default_endpoint(self) -> int:
        return 1

    @pics(
        "WEBRTCP.S",
        "WEBRTCP.S.A0000",     # CurrentSessions attribute
        "WEBRTCP.S.C02.Rsp",   # ProvideOffer command
        "WEBRTCP.S.C06.Rsp",   # EndSession command
        "AVSM.S",
        "AVSM.S.F00",          # Audio Data Output feature
        "AVSM.S.F01",          # Video Data Output feature
    )
    @async_test_body
    async def test_TC_WEBRTCP_2_33(self) -> None:
        """[TC-WEBRTCP-2.33] Validate WebRTC video and audio stream reuse and media delivery across consecutive sessions (referenceCount 0 -> 1 -> 0 -> 1)"""
        self.step("precondition", "DUT commissioned", is_commissioning=True)
        endpoint = self.get_endpoint()
        current_sessions = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.WebRTCTransportProvider,
            attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions,
        )
        asserts.assert_equal(len(current_sessions), 0, "CurrentSessions must be empty in precondition")

        allocated_audio_streams = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.CameraAvStreamManagement,
            attribute=Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams,
        )
        asserts.assert_equal(len(allocated_audio_streams), 0, "AllocatedAudioStreams must be empty in precondition")

        allocated_video_streams = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.CameraAvStreamManagement,
            attribute=Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams,
        )
        asserts.assert_equal(len(allocated_video_streams), 0, "AllocatedVideoStreams must be empty in precondition")

        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        audio_stream_id = None
        video_stream_id = None
        self._active_session_id = None

        try:
            self.step(
                1,
                "TH allocates both Audio and Video streams via CameraAVStreamManagement",
                expectation="Valid stream IDs are obtained with initial referenceCount == 0",
            )
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

            self.step(
                2,
                "TH establishes first WebRTC session (Session 1) using the allocated streams",
                expectation="Valid WebRTCSessionID is obtained, referenceCounts increment to 1, and WebRTC session reaches Connected state",
            )
            log.info("Starting Session 1 with freshly allocated streams (video=%s, audio=%s)", video_stream_id, audio_stream_id)
            session1_id, peer1 = await self._establish_session(webrtc_manager, endpoint, video_stream_id, audio_stream_id)

            current_sessions = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=Clusters.WebRTCTransportProvider,
                attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions,
            )
            asserts.assert_equal(len(current_sessions), 1, "Expected CurrentSessions to contain Session 1")
            asserts.assert_equal(current_sessions[0].id, session1_id, "Session ID in CurrentSessions does not match Session 1")

            s1_audio_ref = await self._get_stream_ref_count(
                audio_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams, endpoint
            )
            s1_video_ref = await self._get_stream_ref_count(
                video_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams, endpoint
            )
            asserts.assert_equal(s1_audio_ref, 1, "Session 1 audio referenceCount must be 1")
            asserts.assert_equal(s1_video_ref, 1, "Session 1 video referenceCount must be 1")

            self.step(
                3,
                "TH verifies active RTP video and audio media reception during Session 1",
                expectation="Non-zero RTP video frames and audio packets are received by the native WebRTC peer connection",
            )
            await self._verify_media_delivery(peer1, "Session 1 (Fresh Stream)")

            self.step(
                4,
                "TH sends EndSession for Session 1 and reads AllocatedVideoStreams / AllocatedAudioStreams",
                expectation="DUT responds with SUCCESS; referenceCounts decrement back to 0 while streams remain allocated",
            )
            log.info("Ending Session 1 (session_id=%s) WITHOUT deallocating streams", session1_id)
            await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                    webRTCSessionID=session1_id,
                    reason=Clusters.Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup,
                ),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            self._active_session_id = None
            await webrtc_manager.remove_peer(session1_id)

            current_sessions = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=Clusters.WebRTCTransportProvider,
                attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions,
            )
            asserts.assert_equal(len(current_sessions), 0, "CurrentSessions must be empty after ending Session 1")

            between_audio_ref = await self._get_stream_ref_count(
                audio_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams, endpoint
            )
            between_video_ref = await self._get_stream_ref_count(
                video_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams, endpoint
            )
            asserts.assert_equal(between_audio_ref, 0, "Audio referenceCount must decrement back to 0 after EndSession")
            asserts.assert_equal(between_video_ref, 0, "Video referenceCount must decrement back to 0 after EndSession")

            self.step(
                5,
                "TH establishes second WebRTC session (Session 2) reusing the SAME allocated VideoStreamID and AudioStreamID without deallocating",
                expectation="DUT responds with ProvideOfferResponse, referenceCounts increment from 0 to 1, and Session 2 reaches Connected state",
            )
            log.info(
                "Starting Session 2 REUSING existing allocated streams (video=%s, audio=%s) with referenceCount == 0",
                video_stream_id,
                audio_stream_id,
            )
            session2_id, peer2 = await self._establish_session(webrtc_manager, endpoint, video_stream_id, audio_stream_id)

            current_sessions = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=Clusters.WebRTCTransportProvider,
                attribute=Clusters.WebRTCTransportProvider.Attributes.CurrentSessions,
            )
            asserts.assert_equal(len(current_sessions), 1, "Expected CurrentSessions to contain Session 2")
            asserts.assert_equal(current_sessions[0].id, session2_id, "Session ID in CurrentSessions does not match Session 2")

            s2_audio_ref = await self._get_stream_ref_count(
                audio_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams, endpoint
            )
            s2_video_ref = await self._get_stream_ref_count(
                video_stream_id, Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams, endpoint
            )
            asserts.assert_equal(s2_audio_ref, 1, "Session 2 audio referenceCount must increment back to 1")
            asserts.assert_equal(s2_video_ref, 1, "Session 2 video referenceCount must increment back to 1")

            self.step(
                6,
                "TH verifies active RTP video and audio media reception during Session 2 (stream reuse)",
                expectation="Non-zero RTP video frames and audio packets are received by the native WebRTC peer connection during the reused stream session",
            )
            await self._verify_media_delivery(peer2, "Session 2 (Reused Stream)")

            self.step(
                7,
                "TH terminates Session 2 via EndSession and deallocates the Audio and Video streams",
                expectation="DUT responds with SUCCESS and streams are removed from AllocatedAudioStreams / AllocatedVideoStreams",
            )
            log.info("Ending Session 2 and deallocating streams")
            await self.send_single_cmd(
                cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                    webRTCSessionID=session2_id,
                    reason=Clusters.Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup,
                ),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            self._active_session_id = None
            await webrtc_manager.remove_peer(session2_id)

            await self.send_single_cmd(
                cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamDeallocate(audioStreamID=audio_stream_id),
                endpoint=endpoint,
            )
            deallocated_audio_id = audio_stream_id
            audio_stream_id = None

            await self.send_single_cmd(
                cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamDeallocate(videoStreamID=video_stream_id),
                endpoint=endpoint,
            )
            deallocated_video_id = video_stream_id
            video_stream_id = None

            allocated_audio_streams = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=Clusters.CameraAvStreamManagement,
                attribute=Clusters.CameraAvStreamManagement.Attributes.AllocatedAudioStreams,
            )
            asserts.assert_not_in(
                deallocated_audio_id,
                [stream.audioStreamID for stream in allocated_audio_streams],
                f"Audio stream {deallocated_audio_id} should have been deallocated",
            )

            allocated_video_streams = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=Clusters.CameraAvStreamManagement,
                attribute=Clusters.CameraAvStreamManagement.Attributes.AllocatedVideoStreams,
            )
            asserts.assert_not_in(
                deallocated_video_id,
                [stream.videoStreamID for stream in allocated_video_streams],
                f"Video stream {deallocated_video_id} should have been deallocated",
            )

        finally:
            if self._active_session_id is not None:
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.WebRTCTransportProvider.Commands.EndSession(
                            webRTCSessionID=self._active_session_id,
                            reason=Clusters.Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup,
                        ),
                        endpoint=endpoint,
                        payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
                    )
                except Exception as e:
                    log.warning("Failed to end active WebRTC session %s during cleanup: %s", self._active_session_id, e)
            if audio_stream_id is not None:
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.CameraAvStreamManagement.Commands.AudioStreamDeallocate(audioStreamID=audio_stream_id),
                        endpoint=endpoint,
                    )
                except Exception as e:
                    log.warning("Failed to deallocate audio stream %s during cleanup: %s", audio_stream_id, e)
            if video_stream_id is not None:
                try:
                    await self.send_single_cmd(
                        cmd=Clusters.CameraAvStreamManagement.Commands.VideoStreamDeallocate(videoStreamID=video_stream_id),
                        endpoint=endpoint,
                    )
                except Exception as e:
                    log.warning("Failed to deallocate video stream %s during cleanup: %s", video_stream_id, e)
            await webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
