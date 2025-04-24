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
from chip.clusters import WebRTCTransportProvider
from chip import webrtc
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.webrtc import (ErrorCallback_t,  PeerConnectedCallback_t,
                         PeerDisconnectedCallback_t, SdpAnswerCallback_t,  StatsCallback_t)
from mobly import asserts

from test_plan_support import commission_if_required


class TC_WEBRTC_1_4(MatterBaseTest):

    def steps_TC_WEBRTC_1_4(self) -> list[TestStep]:
        steps = [
            TestStep("precondition-1", commission_if_required(), is_commissioning=True),
            TestStep("precondition-2", "Confirm no active WebRTC sessions exist in DUT"),
            TestStep(1, "TH sends the SolicitOffer command without ICEServers and ICETransportPolicy to the DUT."),
            TestStep(2, "DUT sends Offer command to TH/WEBRTCR."),
            TestStep(3, "TH sends the SUCCESS status code to the DUT."),
            TestStep(4, "TH sends the ProvideAnswer command with the WebRTCSessionID saved in step 1 and a SDP Offer to the DUT."),
            TestStep(5, "TH sends the ICECandidates command with a its ICE candidates to the DUT."),
            TestStep(6, "TH waits up to 30 seconds for ProvideICECandidates command from the DUT."),
            TestStep(7, "TH waits for 10 seconds."),
            TestStep(8, "TH sends the EndSession command with the WebRTCSessionID saved in step 1 to the DUT.")
        ]

        return steps

    def desc_TC_WEBRTC_1_4(self) -> str:
        return '[TC-WEBRTC-1.3] Validate Deferred Offer Flow for Battery-Powered Camera in Standby Mode.'

    def pics_TC_WEBRTC_1_4(self) -> list[str]:
        return ["WEBRTCR", "WEBRTCP"]

    @async_test_body
    async def test_TC_WEBRTC_1_4(self):
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

        client = webrtc.CreateWebrtcClient(1)

        webrtc.SetCallbacks(client, answer_callback, error_callback,
                            peer_connected_callback, peer_disconnected_callback, stats_callback)

        webrtc.webrtc_requestor_init()

        # TODO Initialise PeerConnection without setting local offer

        endpoint = self.get_endpoint(default=1)

        self.step("precondition-1")

        self.step("precondition-2")
        current_sessions = await self.read_single_attribute_check_success(cluster=WebRTCTransportProvider,
                                                                          attribute=WebRTCTransportProvider.Attributes.CurrentSessions,
                                                                          endpoint=endpoint)
        asserts.assert_equal(len(current_sessions), 0, "Found an existing WebRTC session")

        self.step(1)
        solicit_offer_response: WebRTCTransportProvider.Commands.SolicitOfferResponse = await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.SolicitOffer(
                streamUsage=WebRTCTransportProvider.Enums.StreamUsageEnum.kLiveView,
                videoStreamID=NullValue,
                audioStreamID=NullValue,
            ), endpoint=endpoint
        )
        session_id = solicit_offer_response.webRTCSessionID
        asserts.assert_true(session_id >= 0, "Invalid response")
        asserts.assert_false(solicit_offer_response.deferredOffer, "Expected deferredOffer = False")

        self.step(2)
        # TH should immediately get Answer from DUT
        offer_sessionId, remote_offer_sdp, remote_nodeid = webrtc.get_remote_offer(timeout=5)
        asserts.assert_equal(offer_sessionId, session_id, "Invalid session id")
        asserts.assert_true(len(remote_offer_sdp) > 0, "Invalid offer sdp received")

        self.step(3)
        webrtc.SetOffer(client, remote_offer_sdp)

        self.step(4)
        local_answer = webrtc.CreateAnswer(client)
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideAnswer(
                webRTCSessionID=session_id,
                sdp=local_answer
            ), endpoint=endpoint
        )

        self.step(5)
        local_candidates = webrtc.GetCandidates()
        await self.send_single_cmd(
            cmd=WebRTCTransportProvider.Commands.ProvideICECandidates(
                webRTCSessionID=session_id,
                ICECandidates=local_candidates
            ), endpoint=endpoint
        )

        self.step(6)
        ice_session_id, remote_candidates = webrtc.get_remote_ice_candidates(timeout=30)
        asserts.assert_equal(ice_session_id, session_id, "Invalid session id")
        asserts.assert_true(len(remote_candidates) > 0, "Invalid ice candidates received")
        for candidate in remote_candidates:
            webrtc.SetCandidate(client, candidate)

        self.step(7)
        self.wait_for_user_input("Verify WebRTC session is established")

        self.step(8)
        await self.send_single_cmd(cmd=WebRTCTransportProvider.Commands.EndSession(
            webRTCSessionID=session_id,
            reason=WebRTCTransportProvider.Enums.WebRTCEndReasonEnum.kUserHangup
        ), endpoint=endpoint)

        webrtc.CloseConnection(client)


if __name__ == "__main__":
    default_matter_test_main()
