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

"""Checks that WebRTCPeerConnection.send_command() forwards every SolicitOffer / ProvideOffer field to the camera.

send_command() goes through the C++ WebRTCTransportProviderClient, which re-encodes the command instead of sending
the caller's TLV as-is. These tests send fields that the plain send_single_cmd() path already covers in TC_WEBRTCP_2_*.
"""

import logging

from mobly import asserts
from TC_WEBRTCPTestBase import WEBRTCPTestBase

from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.clusters import Objects, WebRTCTransportProvider
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main
from matter.webrtc import LibdatachannelPeerConnection, WebRTCManager

log = logging.getLogger(__name__)


class TestWebRTCProviderClientStreamFields(MatterTestCommissionedDevice, WEBRTCPTestBase):
    @property
    def default_timeout(self) -> int:
        return 4 * 60

    @property
    def default_endpoint(self) -> int:
        return 1

    def create_peer(self, webrtc_manager: WebRTCManager) -> LibdatachannelPeerConnection:
        return webrtc_manager.create_peer(
            node_id=self.dut_node_id, fabric_index=self.default_controller.GetFabricIndexInternal(), endpoint=self.get_endpoint()
        )

    async def allocate_streams(self) -> tuple[int, int]:
        audio_stream_id = await self.allocate_one_audio_stream()
        video_stream_id = await self.allocate_one_video_stream()
        await self.validate_allocated_audio_stream(audio_stream_id)
        await self.validate_allocated_video_stream(video_stream_id)
        return video_stream_id, audio_stream_id

    async def assert_session_streams(self, session_id: int, video_stream_id: int, audio_stream_id: int):
        current_sessions = await self.read_single_attribute_check_success(
            cluster=WebRTCTransportProvider,
            attribute=WebRTCTransportProvider.Attributes.CurrentSessions,
            endpoint=self.get_endpoint(),
        )
        session = next((s for s in current_sessions if s.id == session_id), None)
        asserts.assert_is_not_none(session, f"Session {session_id} not found in CurrentSessions")
        asserts.assert_equal(session.videoStreamID, video_stream_id, "Session uses the wrong video stream")
        asserts.assert_equal(session.audioStreamID, audio_stream_id, "Session uses the wrong audio stream")

    async def send_ice_candidates_and_wait_for_session(self, webrtc_peer: LibdatachannelPeerConnection, session_id: int):
        local_candidates = await webrtc_peer.get_local_ice_candidates()
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
                webRTCSessionID=session_id,
                ICECandidates=[Objects.Globals.Structs.ICECandidateStruct(candidate=cand.candidate) for cand in local_candidates],
            ),
            endpoint=self.get_endpoint(),
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        asserts.assert_true(await webrtc_peer.check_for_session_establishment(), "Failed to establish WebRTC session")

    async def end_session(self, session_id: int):
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id, reason=Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=self.get_endpoint(),
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

    @async_test_body
    async def test_sframe_config_is_forwarded(self):
        video_stream_id, _ = await self.allocate_streams()

        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer = self.create_peer(webrtc_manager)

        # The provider rejects a Kid shorter than 2 bytes with CONSTRAINT_ERROR. If SFrameConfig were dropped, the
        # command would succeed and start a session.
        invalid_sframe_config = WebRTCTransportProvider.Structs.SFrameStruct(cipherSuite=0x0002, baseKey=b"\x00" * 32, kid=b"\x01")
        try:
            await webrtc_peer.send_command(
                cmd=WebRTCTransportProvider.Commands.SolicitOffer(
                    streamUsage=Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                    originatingEndpointID=1,
                    videoStreamID=video_stream_id,
                    SFrameConfig=invalid_sframe_config,
                ),
                endpoint=self.get_endpoint(),
                payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
            )
            asserts.fail("SolicitOffer with an invalid SFrameConfig succeeded; SFrameConfig was not sent")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Expected CONSTRAINT_ERROR for a 1-byte SFrame Kid")

        await webrtc_manager.close_all()

    @async_test_body
    async def test_solicit_offer_with_stream_lists(self):
        video_stream_id, audio_stream_id = await self.allocate_streams()

        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer = self.create_peer(webrtc_manager)

        solicit_offer_response: WebRTCTransportProvider.Commands.SolicitOfferResponse = await webrtc_peer.send_command(
            cmd=WebRTCTransportProvider.Commands.SolicitOffer(
                streamUsage=Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                originatingEndpointID=1,
                videoStreams=[video_stream_id],
                audioStreams=[audio_stream_id],
            ),
            endpoint=self.get_endpoint(),
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        session_id = solicit_offer_response.webRTCSessionID
        asserts.assert_false(solicit_offer_response.deferredOffer, "Expected deferredOffer = False")
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        offer_session_id, remote_offer_sdp = await webrtc_peer.get_remote_offer(timeout_s=5)
        asserts.assert_equal(offer_session_id, session_id, "Offer received with the wrong session ID")
        webrtc_peer.set_remote_offer(remote_offer_sdp)
        await self.assert_session_streams(session_id, video_stream_id, audio_stream_id)

        local_answer = await webrtc_peer.get_local_answer()
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideAnswer(webRTCSessionID=session_id, sdp=local_answer),
            endpoint=self.get_endpoint(),
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        await self.send_ice_candidates_and_wait_for_session(webrtc_peer, session_id)
        await self.end_session(session_id)
        await webrtc_manager.close_all()

    @async_test_body
    async def test_provide_offer_with_stream_lists(self):
        video_stream_id, audio_stream_id = await self.allocate_streams()

        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        webrtc_peer = self.create_peer(webrtc_manager)

        webrtc_peer.create_offer()
        local_offer = await webrtc_peer.get_local_offer()
        provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await webrtc_peer.send_command(
            cmd=WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=local_offer,
                streamUsage=Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                originatingEndpointID=1,
                videoStreams=[video_stream_id],
                audioStreams=[audio_stream_id],
            ),
            endpoint=self.get_endpoint(),
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        session_id = provide_offer_response.webRTCSessionID
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        answer_session_id, answer = await webrtc_peer.get_remote_answer()
        asserts.assert_equal(answer_session_id, session_id, "Answer received with the wrong session ID")
        webrtc_peer.set_remote_answer(answer)
        await self.assert_session_streams(session_id, video_stream_id, audio_stream_id)

        await self.send_ice_candidates_and_wait_for_session(webrtc_peer, session_id)
        await self.end_session(session_id)
        await webrtc_manager.close_all()


if __name__ == "__main__":
    default_matter_test_main()
