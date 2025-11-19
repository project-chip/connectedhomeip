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

import logging

from mobly import asserts
from TC_WEBRTC_Utils import WebRTCTestHelper
from test_plan_support import commission_if_required

from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.clusters import CameraAvStreamManagement, Objects, WebRTCTransportProvider
from matter.clusters.Types import NullValue
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from matter.webrtc import BrowserPeerConnection, WebRTCManager


class TC_WEBRTC_1_6(MatterBaseTest, WebRTCTestHelper):
    def steps_TC_WEBRTC_1_6(self) -> list[TestStep]:
        steps = [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm no active WebRTC sessions exist in DUT"),
            TestStep("precondition-3", "Confirm DUT(Camera) supports Speaker feature AVSM.S.F04(SPKR)"),
            TestStep(
                1,
                description="TH reads TwoWayTalkSupport Attribute from AVSM cluster from DUT",
                expectation="Verify DUT responds with status success and contains FullDuplex TwoWayTalkSupportTypeEnum value.",
            ),
            TestStep(
                2,
                description="If DUT supports AVSM.S.F01(VDO) feature, TH sends the VideoStreamAllocate command with valid values of StreamUsage, VideoCodec, MinResolution, MaxResolution, MinBitRate, MaxBitRate, MinFrameRate, MaxFrameRate, KeyFrameInterval, WatermarkEnabled, OSDEnabled",
                expectation="DUT responds with VideoStreamAllocateResponse command with a valid VideoStreamID. TH stores this VideoStreamID for later use.",
            ),
            TestStep(
                3,
                description="TH sends the AudioStreamAllocate command with valid values of StreamUsage, AudioCodec, ChannelCount, SampleRate, BitDepth, BitRate.",
                expectation="DUT responds with AudioStreamAllocateResponse command with a valid AudioStreamID. TH stores this AudioStreamID for later use.",
            ),
            TestStep(
                4,
                description="TH sends the ProvideOffer command with an SDP Offer, null WebRTCSessionID, VideoStreamID and AudioStreamID saved in steps 2, 3 to the DUT.",
                expectation="DUT responds with ProvideOfferResponse containing allocated WebRTCSessionID, VideoStreamID and AudioStreamID. TH saves the WebRTCSessionID to be used later.",
            ),
            TestStep(
                5,
                description="DUT sends Answer command to the TH/WEBRTCR.",
                expectation="Verify that Answer command contains the same WebRTCSessionID saved in step 4 and contain a non-empty SDP string.",
            ),
            TestStep(6, description="TH sends the SUCCESS status code to the DUT."),
            TestStep(
                7,
                description="TH sends the ProvideICECandidates command with a its ICE candidates to the DUT.",
                expectation="DUT responds with SUCCESS status code.",
            ),
            TestStep(
                8,
                description="TH waits for 10 seconds",
                expectation="Verify the WebRTC session has been successfully established. Verify TH is playing live audio received from DUT. Verify DUT is able to play the live audio in its speaker sent by TH. If DUT supports AVSM.S.F01(VDO) feature, Verify TH is playing live video received from DUT.",
            ),
            TestStep(
                9,
                description="TH sends the EndSession command with the WebRTCSessionID saved in step 4 to the DUT.",
                expectation="DUT responds with SUCCESS status code.",
            ),
            TestStep(
                10,
                description="TH sends the VideoStreamDeallocate command if allocated, with the VideoStreamID saved in step 2 to the DUT.",
                expectation="DUT responds with SUCCESS status code.",
            ),
            TestStep(
                11,
                description="TH sends the AudioStreamDeallocate command with the AudioStreamID saved in step 3 to the DUT.",
                expectation="DUT responds with SUCCESS status code.",
            ),
        ]
        return steps

    def desc_TC_WEBRTC_1_6(self) -> str:
        return " [TC-WEBRTC-1.6] Validate Two-Way-Talk Full-Duplex support in camera(DUT) - PROVISIONAL"

    def pics_TC_WEBRTC_1_6(self) -> list[str]:
        return ["WEBRTCR.C", "WEBRTCP.S", "AVSM.S"]

    @property
    def default_timeout(self) -> int:
        return 4 * 60  # 4 minutes

    @run_if_endpoint_matches(
        has_feature(CameraAvStreamManagement, CameraAvStreamManagement.Bitmaps.Feature.kAudio)
        and has_feature(CameraAvStreamManagement, CameraAvStreamManagement.Bitmaps.Feature.kSpeaker)
    )
    async def test_TC_WEBRTC_1_6(self):
        self.step("precondition-1")

        endpoint = self.get_endpoint()
        webrtc_manager = WebRTCManager(event_loop=self.event_loop)
        await webrtc_manager.ws_connect()
        webrtc_peer = None
        try:
            webrtc_peer = await webrtc_manager.create_browser_peer(
                media_direction={"audio": "sendrecv"},
                node_id=self.dut_node_id,
                fabric_index=self.default_controller.GetFabricIndexInternal(),
                endpoint=endpoint,
            )
            await self._run_test_steps(webrtc_manager, webrtc_peer, endpoint)
        finally:
            if webrtc_peer:
                await webrtc_manager.close_all()
            await webrtc_manager.ws_disconnect()

    async def _run_test_steps(self, webrtc_manager: WebRTCManager, webrtc_peer: BrowserPeerConnection, endpoint: int):
        self.step("precondition-2")
        current_sessions = await self.read_single_attribute_check_success(
            cluster=WebRTCTransportProvider, attribute=WebRTCTransportProvider.Attributes.CurrentSessions, endpoint=endpoint
        )
        asserts.assert_equal(len(current_sessions), 0, "Found an existing WebRTC session")

        self.step("precondition-3")

        self.step(1)
        two_way_talk_support = await self.read_single_attribute_check_success(
            cluster=CameraAvStreamManagement, attribute=CameraAvStreamManagement.Attributes.TwoWayTalkSupport, endpoint=endpoint
        )
        asserts.assert_equal(
            two_way_talk_support,
            CameraAvStreamManagement.Enums.TwoWayTalkSupportTypeEnum.kFullDuplex,
            "TwoWayTalkSupport expected FullDuplex value",
        )

        avsm_feature_map = await self.read_single_attribute_check_success(
            cluster=CameraAvStreamManagement, attribute=CameraAvStreamManagement.Attributes.FeatureMap, endpoint=endpoint
        )
        dut_has_vdo_feature = (avsm_feature_map & CameraAvStreamManagement.Bitmaps.Feature.kVideo) > 0
        aVideoStreamID = None
        aAudioStreamID = None
        if dut_has_vdo_feature:
            self.step(2)
            aVideoStreamID = await self.allocate_video_stream(endpoint)
            asserts.assert_is_not_none(aVideoStreamID, "Failed to allocate video stream")
        else:
            self.skip_step(2)

        self.step(3)
        aAudioStreamID = await self.allocate_audio_stream(endpoint)
        asserts.assert_is_not_none(aAudioStreamID, "Failed to allocate audio stream")

        self.step(4)
        offer = await webrtc_peer.create_offer()
        provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await webrtc_peer.send_command(
            cmd=WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=NullValue,
                sdp=offer,
                streamUsage=Objects.Globals.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=aVideoStreamID,
                audioStreamID=aAudioStreamID,
                originatingEndpointID=1,
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )
        session_id = provide_offer_response.webRTCSessionID
        asserts.assert_true(session_id >= 0, "Invalid response")
        if dut_has_vdo_feature:
            asserts.assert_equal(provide_offer_response.videoStreamID, aVideoStreamID, "VideoStreamID mismatch")
        asserts.assert_equal(provide_offer_response.audioStreamID, aAudioStreamID, "AudioStreamID mismatch")
        webrtc_manager.session_id_created(session_id, self.dut_node_id)

        self.step(5)
        answer_sessionId, answer = await webrtc_peer.get_remote_answer()
        asserts.assert_equal(session_id, answer_sessionId, "ProvideAnswer invoked with wrong session id")
        asserts.assert_true(len(answer) > 0, "Invalid answer SDP received")

        self.step(6)
        await webrtc_peer.set_remote_answer(answer)

        self.step(7)
        local_candidates = await webrtc_peer.get_local_ice_candidates()
        local_candidates_struct_list = [
            Objects.Globals.Structs.ICECandidateStruct(
                candidate=cand.candidate,
                SDPMid=cand.sdpMid,
                SDPMLineIndex=NullValue if cand.sdpMLineIndex == -1 else cand.sdpMLineIndex,
            )
            for cand in local_candidates
        ]
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
                webRTCSessionID=answer_sessionId, ICECandidates=local_candidates_struct_list
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        self.step(8)
        if not await webrtc_peer.check_for_session_establishment():
            logging.error("Failed to establish webrtc session")
            raise Exception("Failed to establish webrtc session")

        if not self.is_pics_sdk_ci_only:
            self.user_verify_two_way_talk("Verify if two way talk back is working")

        self.step(9)
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.EndSession(
                webRTCSessionID=session_id, reason=Objects.Globals.Enums.WebRTCEndReasonEnum.kUserHangup
            ),
            endpoint=endpoint,
            payloadCapability=TransportPayloadCapability.LARGE_PAYLOAD,
        )

        if dut_has_vdo_feature:
            self.step(10)
            await self.send_single_cmd(
                endpoint=endpoint, cmd=CameraAvStreamManagement.Commands.VideoStreamDeallocate(videoStreamID=aVideoStreamID)
            )
        else:
            self.skip_step(10)

        self.step(11)
        await self.send_single_cmd(
            endpoint=endpoint, cmd=CameraAvStreamManagement.Commands.AudioStreamDeallocate(audioStreamID=aAudioStreamID)
        )


if __name__ == "__main__":
    default_matter_test_main()
