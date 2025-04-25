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
from concurrent.futures import Future

from chip import webrtc
from chip.clusters import CameraAvStreamManagement, WebRTCTransportProvider
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.webrtc import ErrorCallback_t, PeerConnectedCallback_t, PeerDisconnectedCallback_t, SdpAnswerCallback_t, StatsCallback_t
from mobly import asserts
from test_plan_support import commission_if_required


class TC_WEBRTC_1_2(MatterBaseTest):

    def steps_TC_WEBRTC_1_2(self) -> list[TestStep]:
        steps = [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm there is an active WebRTC sessions exist in DUT"),
            TestStep(0, "TH Reads CurrentSessions attribute from WEBRTCP (DUT)"),
            TestStep(1, "TH sends the ProvideOffer command with an SDP Offer with the WebRTCSessionID saved in step 1 to the DUT."),
            TestStep(2, "TH waits up to 30 seconds for Answer command from the DUT."),
            TestStep(3, "TH sends the SUCCESS status code to the DUT."),
            TestStep(4, "TH sends the ICECandidates command with a its ICE candidates to the DUT."),
            TestStep(5, "TH waits up to 30 seconds for ProvideICECandidates command from the DUT."),
            TestStep(6, "TH waits for 10 seconds."),
            TestStep(7, "TH sends the EndSession command with the WebRTCSessionID saved in step 1 to the DUT.")
        ]

        return steps

    def desc_TC_WEBRTC_1_2(self) -> str:
        return '[TC-WEBRTC-1.2] Validate that providing an existing WebRTC session ID with an SDP Offer successfully triggers the re-offer flow.'

    def pics_TC_WEBRTC_1_2(self) -> list[str]:
        return ["WEBRTCR", "WEBRTCP"]

    @async_test_body
    async def test_TC_WEBRTC_1_2(self):
        def on_answer(answer, peer):
            logging.debug("on_answer called")

        def on_error(error, peer):
            logging.debug("on_error called")

        def on_connected(peer):
            logging.debug("on_connected called")

        def on_disconnected(peer):
            logging.debug("on_disconnected called")

        def on_stats(stats, peer):
            logging.debug(stats)

        answer_callback = SdpAnswerCallback_t(on_answer)
        error_callback = ErrorCallback_t(on_error)
        peer_connected_callback = PeerConnectedCallback_t(on_connected)
        peer_disconnected_callback = PeerDisconnectedCallback_t(on_disconnected)
        stats_callback = StatsCallback_t(on_stats)

        self.step("precondition-1")

        webrtc.webrtc_requestor_init()
        # Test Invokation

        client = webrtc.CreateWebrtcClient(1)
        endpoint = self.get_endpoint(default=1)

        #! TODO: upsert session from provide offer response.
        #! But offer response simply reaches too slow here, by which time,
        #! matter stack would have already received Answer command
        #! and returned NOT_FOUND status
        # TODO: Need to handle offer response with CommandSenderCallback directly
        # Upserting expected session details for now

        webrtc.webrtc_requestor_upsert_session(
            sessionId=1,
            audioStreamId=1,
            videoStreamId=1,
            nodeId=self.dut_node_id,
            fabricIndex=self.default_controller.GetFabricIndexInternal(),
            endpoint=endpoint
        )

        self.step("precondition-2")
        await establish_webrtc_session(client, endpoint, self)

        current_sessions = await self.read_single_attribute_check_success(cluster=WebRTCTransportProvider,
                                                                          attribute=WebRTCTransportProvider.Attributes.CurrentSessions,
                                                                          endpoint=endpoint)
        asserts.assert_equal(len(current_sessions), 1, f"No Pre-Existing session found")

        resolution = CameraAvStreamManagement.Structs.VideoResolutionStruct(
            width=640, height=480
        )
        await self.send_single_cmd(cmd=CameraAvStreamManagement.Commands.VideoStreamAllocate(
            streamUsage=0, videoCodec=0, minFrameRate=30, maxFrameRate=30, minBitRate=10000, maxBitRate=10000, minFragmentLen=1,
            maxFragmentLen=10, minResolution=resolution, maxResolution=resolution), endpoint=endpoint)

        self.step(0)
        prev_sessionid = current_sessions[0].id

        webrtc.SetCallbacks(client, answer_callback, error_callback,
                            peer_connected_callback, peer_disconnected_callback, stats_callback)

        self.step(1)
        offer = webrtc.CreateOffer(client)

        provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideOffer(
                webRTCSessionID=prev_sessionid,
                sdp=offer,
                streamUsage=WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=NullValue,
                audioStreamID=NullValue,
                originatingEndpointID=1
            ), endpoint=endpoint
        )

        asserts.assert_equal(provide_offer_response.webRTCSessionID, prev_sessionid, "Invalid response")

        self.step(2)
        answer_sessionId, answer = webrtc.get_remote_answer(timeout=30)

        asserts.assert_equal(prev_sessionid, answer_sessionId)

        self.step(3)
        webrtc.SetAnswer(client, answer)

        self.step(4)
        local_candidates = webrtc.GetCandidates()

        await self.send_single_cmd(cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
            webRTCSessionID=answer_sessionId,
            ICECandidates=local_candidates
        ), endpoint=endpoint)

        # TODO: handle remote candidates
        #  TH wait for 10s
        # remote_candidates = get_remote_ice_candidates()

        # for candidate in remote_candidates:
        #     webrtc.SetCandidate(client, candidate)

        self.skip_step(5)
        self.step(6)
        self.wait_for_user_input("Press enter to complete TC execution")

        self.step(7)
        await self.send_single_cmd(cmd=WebRTCTransportProvider.Commands.EndSession(
            webRTCSessionID=prev_sessionid,
            reason=WebRTCTransportProvider.Enums.WebRTCEndReasonEnum.kUserHangup
        ), endpoint=endpoint)

        webrtc.CloseConnection(client)


async def establish_webrtc_session(client, endpoint, ctrl):

    session_established_future = Future()

    def on_connected1(peer):
        logging.debug("on_connected")
        session_established_future.set_result(None)

    def on_answer(answer, peer):
        logging.debug("on_answer called")

    def on_error(error, peer):
        logging.debug("on_error called")

    def on_disconnected(peer):
        logging.debug("on_disconnected called")

    def on_stats(stats, peer):
        logging.debug(stats)

    answer_callback = SdpAnswerCallback_t(on_answer)
    error_callback = ErrorCallback_t(on_error)
    peer_connected_callback = PeerConnectedCallback_t(on_connected1)
    peer_disconnected_callback = PeerDisconnectedCallback_t(on_disconnected)
    stats_callback = StatsCallback_t(on_stats)

    webrtc.SetCallbacks(client, answer_callback, error_callback,
                        peer_connected_callback, peer_disconnected_callback, stats_callback)
    webrtc.InitialiseConnection(client)

    offer = webrtc.CreateOffer(client)

    provide_offer_response: WebRTCTransportProvider.Commands.ProvideOfferResponse = await ctrl.send_single_cmd(
        cmd=WebRTCTransportProvider.Commands.ProvideOffer(
            webRTCSessionID=NullValue,
            sdp=offer,
            streamUsage=WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
            videoStreamID=NullValue,
            audioStreamID=NullValue,
            originatingEndpointID=1
        ), endpoint=endpoint
    )

    asserts.assert_true(provide_offer_response.webRTCSessionID >= 0, "Invalid response")

    answer_sessionId, answer = webrtc.get_remote_answer()

    asserts.assert_equal(provide_offer_response.webRTCSessionID, answer_sessionId)

    webrtc.SetAnswer(client, answer)

    local_candidates = webrtc.GetCandidates()

    await ctrl.send_single_cmd(cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
        webRTCSessionID=answer_sessionId,
        ICECandidates=local_candidates
    ), endpoint=endpoint)

    # TODO handle remote candidates
    # remote_candidates = get_remote_ice_candidates()

    # for candidate in remote_candidates:
    #     webrtc.SetCandidate(client, candidate)
    return session_established_future.result()


if __name__ == "__main__":
    default_matter_test_main()
